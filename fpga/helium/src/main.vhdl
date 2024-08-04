LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.p_word.ALL;

ENTITY main IS
	GENERIC(
		sim            : boolean := false;
		sim_skip_init  : boolean := false;
		sim_fast_flash : boolean := false;
		sim_fast_ram   : boolean := false
	);
	PORT(
		--52
		clk_p       : IN std_ulogic;
		--4
		nreset      : IN std_ulogic;
		--3
		int         : IN std_ulogic;

		--70:77
		screen_data : OUT std_ulogic_vector( 7 DOWNTO  0) := "00000000";
		--68
		screen_rs   : OUT std_ulogic := '0';
		--69
		screen_en   : OUT std_ulogic := '0';

		--16:13,11:10
		leds : OUT std_ulogic_vector(5 DOWNTO 0) := "010101";

		--flash controller
		--61
		flash_si   : OUT std_ulogic := 'Z';
		--62
		flash_so   : IN  std_ulogic;
		--59
		flash_clk  : OUT std_ulogic := '0';
		--60
		flash_csn  : OUT std_ulogic := '1';

		--external mem interface
		--38
		p_mem_re   :   OUT std_ulogic := '0';
		--37
		p_mem_we   :   OUT std_ulogic := '0';
		--36,39,25:30
		p_mem_data : INOUT std_ulogic_vector(7 DOWNTO 0) := x"00";
		--33
		p_mem_rdy  : IN    std_ulogic;
		--34
		p_mem_clk  :   OUT std_ulogic := '0'
	);

END ENTITY main;


ARCHITECTURE arch OF main IS 
	SIGNAL timer_1ms_passed : std_ulogic := '0';

	SIGNAL mem_re, mem_we, mem_rdy       : std_ulogic := '0';
	SIGNAL mem_addr                      :  t_word := x"0000";
	SIGNAL mem_data                      : t_rword := x"0000";

	SIGNAL flash_re, flash_we, flash_rdy : std_ulogic := '0';
	SIGNAL flash_addr                    :  t_word := x"0000";
	SIGNAL flash_data_in, flash_data_out : t_rword := x"0000";

	SIGNAL port_re, port_we, port_rdy    : std_ulogic := '0';
	SIGNAL port_addr                     :  t_word := x"0000";
	SIGNAL port_data                     : t_rword := x"0000";

	SIGNAL interrupt  : std_ulogic;
	SIGNAL int_id     : unsigned(1 DOWNTO 0);

	--vector for future compatibility
	SIGNAL port_rdy_src, port_rdy_src_sav : unsigned(0 DOWNTO 0) := "0";

	CONSTANT port_rdy_src_const : unsigned(port_rdy_src'range) := to_unsigned(0, port_rdy_src'length);
	CONSTANT port_rdy_src_flash : unsigned(port_rdy_src'range) := to_unsigned(1, port_rdy_src'length);

	SIGNAL clk : std_ulogic := '0';
BEGIN
	--slows down clock to 3MHz
	pll: ENTITY work.Gowin_rPLL(Behavioral)	PORT MAP (
        clkout => clk,
        clkin  => clk_p
    );

	timer:  ENTITY work.timer(arch) PORT MAP(
		clk              => clk,
		nreset           => '1',
		timer_1us_passed => OPEN,
		timer_1ms_passed => timer_1ms_passed) 
		/*synthesis syn_noprune = 1*/;

	mc: ENTITY work.memory_controller(arch) 
	GENERIC MAP(
		sim      => sim,
		sim_fast => sim AND sim_fast_ram  
		)
	PORT MAP(
		clk      => clk,
		re       => mem_re,
		we       => mem_we,
		rdy      => mem_rdy,
		addr     => mem_addr,
		data     => mem_data,
		--external IF
		mem_re   => p_mem_re,
		mem_we   => p_mem_we,
		mem_data => p_mem_data,
		mem_rdy  => p_mem_rdy,
		mem_clk  => p_mem_clk
		);

	fc: ENTITY work.flash_controller(arch) 
	GENERIC MAP(
		sim_fast => sim AND sim_fast_flash
		)
	PORT MAP(
		--address is shifted by 1 internally
		addr      => 7x"00" & flash_addr, 
		data_in   => flash_data_in,
		data_out  => flash_data_out,
		re        => flash_re,
		we        => flash_we,
		rdy       => flash_rdy,
		clk       => clk,

		--external flash IF
		flash_si  => flash_si,
		flash_so  => flash_so,
		flash_clk => flash_clk,
		flash_csn => flash_csn);
	
	cpu:    ENTITY work.cpu(helium) 
	GENERIC MAP(
		sim_skip_init => sim AND sim_skip_init
		)
	PORT MAP(
		clk          => clk,
		nreset       => '1',
		ex_interrupt => '0',
		ex_int_id    => (OTHERS => '0'),
		mem_addr   => mem_addr  , mem_data   => mem_data  , mem_rdy   => mem_rdy  , mem_re   => mem_re  , mem_we   => mem_we  ,
		port_addr  => port_addr , port_data  => port_data , port_rdy  => port_rdy , port_re  => port_re , port_we  => port_we );

	interrupt <= timer_1ms_passed OR NOT int;
	int_id    <= "01" WHEN timer_1ms_passed
	        ELSE "10" WHEN NOT int
	        ELSE "00";

	flash_we <= port_we WHEN port_addr = x"0005"
	       ELSE '0';
	flash_re <= port_re WHEN port_addr = x"0005"
	       ELSE '0';

	port_data  <= x"ZZZZ"        WHEN port_we
	         ELSE flash_data_out WHEN port_rdy_src ?= port_rdy_src_flash
	         ELSE x"BEEF";

	WITH port_rdy_src SELECT port_rdy <=
		'1'       WHEN port_rdy_src_const,
		flash_rdy WHEN port_rdy_src_flash,
		'0'       WHEN OTHERS            ;

--	leds <= port_data(5 DOWNTO 0);


	ports: PROCESS (ALL) IS 
	BEGIN
		   IF port_re THEN 
			CASE port_addr IS 
				WHEN x"0005" => port_rdy_src <= port_rdy_src_flash;
				WHEN OTHERS  => port_rdy_src <= port_rdy_src_const;
			END CASE;
		ELSIF port_we THEN 
			CASE port_addr IS 
				WHEN x"0000" => port_rdy_src <= port_rdy_src_const;
				WHEN x"0001" => port_rdy_src <= port_rdy_src_const;
				WHEN x"0002" => port_rdy_src <= port_rdy_src_const;
				WHEN x"0003" => port_rdy_src <= port_rdy_src_const;
				WHEN x"0004" => port_rdy_src <= port_rdy_src_const;
				WHEN x"0005" => port_rdy_src <= port_rdy_src_flash;
				WHEN OTHERS  => port_rdy_src <= port_rdy_src_const;
			END CASE;
		ELSE
			port_rdy_src <= port_rdy_src_sav;
		END IF;


		IF rising_edge(clk) THEN 
			IF port_we THEN 
				CASE port_addr IS 
					WHEN x"0000" => leds            <= NOT port_data(5 DOWNTO 0);
					WHEN x"0001" => screen_data     <= port_data(7 DOWNTO 0);
					WHEN x"0002" => screen_rs       <= port_data(0);
					WHEN x"0003" => screen_en       <= port_data(0);
					WHEN x"0004" => flash_addr      <= port_data;
					WHEN x"0005" => flash_data_in   <= port_data; /*see near flash*/
					WHEN OTHERS  => NULL;
				END CASE;
			END IF;
		END IF;


		IF rising_edge(clk) THEN
			port_rdy_src_sav <= port_rdy_src;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
