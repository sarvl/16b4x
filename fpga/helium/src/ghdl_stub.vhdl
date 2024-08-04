LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.p_arithmetic.ALL;
USE work.p_word.ALL;
USE work.p_mem.ALL;

ENTITY ghdl_stub IS 
END ENTITY ghdl_stub;


ARCHITECTURE arch OF ghdl_stub IS 
	CONSTANT sim_skip_init  : boolean := true;
	CONSTANT sim_fast_flash : boolean := true;
	CONSTANT sim_fast_ram   : boolean := true;

	SIGNAL clk : std_ulogic := '0';

	SIGNAL flash_si, flash_so, flash_clk, flash_csn : std_ulogic := '0';

	TYPE t_f_state IS (
		f_state_cmd    , f_state_addr   , f_state_dummy  , f_state_read   , f_state_ignore ,
		f_state_write  
	);
	SIGNAL f_state   : t_f_state := f_state_cmd;

	SIGNAL f_counter : integer RANGE 127 DOWNTO 0 := 7;
	SIGNAL f_addr    : std_ulogic_vector(23 DOWNTO 0) := x"000000";


	TYPE t_m_state IS (
		m_start        , m_read_addr    , m_read_wait_0  , m_read_wait_1  , m_read_read_0 , 
		m_read_read_1  , m_write_addr   , m_write_write_0, m_write_write_1, m_write_wait_0,
		m_write_wait_1 , m_inv_norw      
		);
	SIGNAL m_state : t_m_state := m_start;


	SIGNAL mem_re, mem_we, mem_rdy, mem_clk : std_ulogic;
	SIGNAL mem_data : std_logic_vector(7 DOWNTO 0);
BEGIN
	dut: ENTITY work.main(arch)
	GENERIC MAP(
		sim            => true,
		sim_skip_init  => sim_skip_init,
		sim_fast_ram   => sim_fast_ram,
		sim_fast_flash => sim_fast_flash
	)
	PORT MAP(
		clk_p       => clk,
		nreset      => '1',
		int         => '0',

		--screen
		screen_data => OPEN,
		screen_rs   => OPEN,
		screen_en   => OPEN,

		--leds
		leds        => OPEN,

		--flash
		flash_si    => flash_si ,
		flash_so    => flash_so ,
		flash_clk   => flash_clk,
		flash_csn   => flash_csn,

		--ram
		p_mem_re    => mem_re,
		p_mem_we    => mem_we,
		p_mem_data  => mem_data,
		p_mem_rdy   => mem_rdy, 
		p_mem_clk   => mem_clk 
	);

	/* 
	 * ignore latch for write, asssume it always succeds 
	 * operates on flash_clk 
	 * SIMULATION ONLY
	 *
	 * if fast sim is enabled, flash controller interfaces directly with flash much faster
	 * this is used to test whether flash controller works as intended
	 */
	gen_sim_fast_flash:
	IF NOT sim_fast_flash GENERATE
		--bypassed for convienience anyway
		fu: ENTITY work.flash_data(arch) PORT MAP(
			clk => '0', addr => 23x"000000", data_in=>x"0000", data_out=>OPEN, we => '0');

		flash_stub:
		PROCESS IS 
			VARIABLE cmd     : std_ulogic_vector( 7 DOWNTO 0) := (OTHERS => '0');
			ALIAS flash_data IS <<SIGNAL fu.flash : t_mem_rbyte>>;
		BEGIN
			WAIT ON flash_clk;

			IF flash_csn = '1' THEN
				f_state <= f_state_cmd;
				f_counter <= 7;
			ELSE
				CASE f_state IS
					WHEN f_state_cmd =>
						flash_so <= '0';
						cmd(f_counter) := flash_si;

						f_counter  <= 23             WHEN rising_edge(flash_clk) AND f_counter = 0
							   ELSE f_counter - 1    WHEN rising_edge(flash_clk)
							   ELSE f_counter;

						f_state  <= f_state_addr   WHEN rising_edge(flash_clk) AND f_counter = 0
							   ELSE f_state;

					WHEN f_state_addr =>
						flash_so <= '0';
						f_addr(f_counter) <= flash_si;

						f_counter  <= 7              WHEN rising_edge(flash_clk) AND f_counter = 0 AND cmd = x"0B"
							   ELSE 7                WHEN rising_edge(flash_clk) AND f_counter = 0 AND cmd = x"06"
							   ELSE 15               WHEN rising_edge(flash_clk) AND f_counter = 0 AND cmd = x"02"
							   ELSE f_counter - 1    WHEN rising_edge(flash_clk)
							   ELSE f_counter;

						f_state  <= f_state_dummy  WHEN rising_edge(flash_clk) AND f_counter = 0 AND cmd = x"0B"
							   ELSE f_state_ignore WHEN rising_edge(flash_clk) AND f_counter = 0 AND cmd = x"06"
							   ELSE f_state_write  WHEN rising_edge(flash_clk) AND f_counter = 0 AND cmd = x"02"
							   ELSE f_state;
					
					WHEN f_state_dummy =>
						flash_so <= '0';

						f_counter  <= 7              WHEN rising_edge(flash_clk) AND f_counter = 0
							   ELSE f_counter - 1    WHEN rising_edge(flash_clk)
							   ELSE f_counter;

						f_state  <= f_state_read   WHEN rising_edge(flash_clk) AND f_counter = 0
							   ELSE f_state;

					--transition to falling edge
					WHEN f_state_read =>
						flash_so <= flash_data(to_uint(f_addr))(f_counter);

						f_addr <= f_addr + 1       WHEN falling_edge(flash_clk) AND f_counter = 0
							   ELSE f_addr;
						f_counter  <= 7              WHEN falling_edge(flash_clk) AND f_counter = 0
							   ELSE f_counter - 1    WHEN falling_edge(flash_clk)
							   ELSE f_counter;

					WHEN f_state_ignore =>
						NULL;

					WHEN f_state_write =>
						flash_data(to_uint(f_addr))(f_counter) <= flash_si;

						f_addr <= f_addr + 1       WHEN rising_edge(flash_clk) AND f_counter = 0
							   ELSE f_addr;
						f_counter  <= 7              WHEN rising_edge(flash_clk) AND f_counter = 0
							   ELSE f_counter - 1    WHEN rising_edge(flash_clk)
							   ELSE f_counter;
				END CASE;
			END IF;
		END PROCESS flash_stub;
	END GENERATE gen_sim_fast_flash;

	
	/* 
	 * SIMULATION ONLY
	 *
	 * if fast sim is enabled, mem controller interfaces directly with mem much faster
	 * this is used to test whether mem controller works as intended
	 */
	gen_sim_fast_ram:
	IF NOT sim_fast_ram GENERATE
		--bypassed for convienience anyway
		mu : ENTITY work.ram_data(arch) PORT MAP(
			clk => '0', addr => x"0000", data => OPEN, re => '0', we => '0' 
		);
	
		mem_rdy  <= NOT mem_re AND NOT mem_we WHEN m_state = m_start
		       ELSE '1'                       WHEN m_state = m_read_read_0 OR m_state = m_read_read_1
		                                        OR m_state = m_write_wait_1
		       ELSE '0';

		ram_stub:
		PROCESS IS
			VARIABLE addr    : std_ulogic_vector(15 DOWNTO 0) := (OTHERS => '0');

			ALIAS ram_data IS <<SIGNAL mu.ram : t_mem_rword>>;
		BEGIN
			WAIT ON mem_clk;

			CASE m_state IS 
				WHEN m_start =>
					addr(15 DOWNTO 8) := mem_data;
					mem_data <= x"ZZ";

					m_state <= m_read_addr  WHEN rising_edge(mem_clk) AND mem_re = '1'
					      ELSE m_write_addr WHEN rising_edge(mem_clk) AND mem_we = '1'
					      ELSE m_inv_norw   WHEN rising_edge(mem_clk)
					      ELSE UNAFFECTED;
				WHEN m_read_addr =>
					addr( 7 DOWNTO 0) := mem_data;
					mem_data <= x"ZZ";

					m_state <= m_read_wait_0 WHEN falling_edge(mem_clk)
					      ELSE UNAFFECTED;
				WHEN m_read_wait_0 =>
					mem_data <= x"ZZ";
					m_state <= m_read_wait_1 WHEN rising_edge(mem_clk)
					      ELSE UNAFFECTED;
				WHEN m_read_wait_1 =>
					m_state <= m_read_read_0 WHEN falling_edge(mem_clk)
					      ELSE UNAFFECTED;

					mem_data <= ram_data(to_uint(addr))(15 DOWNTO 8);
				WHEN m_read_read_0 =>
					m_state <= m_read_read_1 WHEN rising_edge(mem_clk)
					      ELSE UNAFFECTED;

					mem_data <= ram_data(to_uint(addr))( 7 DOWNTO 0);

				WHEN m_read_read_1 =>
					mem_data <= x"ZZ";
					m_state <= m_start WHEN falling_edge(mem_clk)
					      ELSE UNAFFECTED;

				WHEN m_write_addr =>
					mem_data <= x"ZZ";
					addr( 7 DOWNTO 0) := mem_data;

					m_state <= m_write_write_0 WHEN falling_edge(mem_clk)
					      ELSE UNAFFECTED;
				WHEN m_write_write_0 =>
					mem_data <= x"ZZ";
					ram_data(to_uint(addr))(15 DOWNTO 8) <= mem_data;
					
					m_state <= m_write_write_1 WHEN rising_edge(mem_clk)
					      ELSE UNAFFECTED;
				WHEN m_write_write_1 =>
					ram_data(to_uint(addr))( 7 DOWNTO 0) <= mem_data;
					m_state <= m_write_wait_0 WHEN falling_edge(mem_clk)
					      ELSE UNAFFECTED;

					mem_data <= ram_data(to_uint(addr))(15 DOWNTO 8);
				WHEN m_write_wait_0 =>
					mem_data <= x"ZZ";
					m_state <= m_write_wait_1 WHEN rising_edge(mem_clk)
					      ELSE UNAFFECTED;
				WHEN m_write_wait_1 =>
					mem_data <= x"ZZ";
					m_state <= m_start WHEN falling_edge(mem_clk)
					      ELSE UNAFFECTED;

				WHEN m_inv_norw =>
					ASSERT false
						REPORT "no read or write on rising edge"
						SEVERITY failure;
			END CASE;
		END PROCESS;
	END GENERATE gen_sim_fast_ram;


	clock:
	PROCESS IS BEGIN
		clk <= '0'; WAIT FOR 500 PS;
		clk <= '1'; WAIT FOR 500 PS;
	END PROCESS;

END ARCHITECTURE arch;
