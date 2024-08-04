--P35Q32U

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.p_word.ALL;

--pulse ren low for 1 cycle to read
--pusle wen low for 1 cycle to write
--wait for ready
ENTITY flash_controller IS 
	GENERIC(
		sim_fast : boolean := false
	);
	PORT(
		--actual  IF 
		addr       : IN    std_ulogic_vector(22 DOWNTO 0);
		data_in    : IN    t_word;
		data_out   :   OUT t_word := x"0000";
		re         : IN    std_ulogic;
		we         : IN    std_ulogic;
		rdy        :   OUT std_ulogic := '1';
		clk        : IN    std_ulogic;

		--flash controller
		--61
		flash_si   : OUT std_ulogic := 'Z';
		--62
		flash_so   : IN  std_ulogic;
		--59
		flash_clk  : OUT std_ulogic := '0';
		--60
		flash_csn  : OUT std_ulogic := '1'
	);
END ENTITY flash_controller;


ARCHITECTURE arch OF flash_controller IS
	--send only when action is 1
	--clock divider
	SIGNAL action : std_ulogic := '0';

	SIGNAL count    : unsigned(5 DOWNTO 0) := (OTHERS => '0');

	SIGNAL internal_data  : std_ulogic_vector(47 DOWNTO 0);

	TYPE t_state IS (
		state_idle       , state_read_0     , state_read_1     , state_read_2     , state_read_3     ,
		state_read_4     , state_read_5     , state_read_6     , state_write_en_0 , state_write_en_1 ,
		state_write_en_2 , state_write_en_3 , state_write_0    , state_write_1    , state_write_2    ,
		state_write_3    
	);
	SIGNAL state : t_state := state_idle;

BEGIN
	-- synthesis translate_off
	gen_sim_fast:
	IF sim_fast GENERATE
		rdy <= '1';

		flash_data : ENTITY work.flash_data(arch) PORT MAP(
			clk      => clk,
			addr     => addr,
			data_in  => data_in,
			data_out => data_out,
			we       => we 
		);
	ELSE GENERATE
	-- synthesis translate_on
		rdy <= NOT we AND NOT re WHEN state = state_idle
		  ELSE '1'               WHEN state = state_read_6
		  ELSE '1'               WHEN state = state_write_3
		  ELSE '0';

		data_out <= internal_data(15 DOWNTO 0);

		PROCESS(ALL) IS
		BEGIN
			IF rising_edge(clk) THEN 
				CASE state IS
					WHEN state_idle =>
						state <= state_write_en_0 WHEN we
							ELSE state_read_0     WHEN re
							ELSE UNAFFECTED;
						flash_csn <= '1';

						flash_si  <= 'Z';
						flash_clk <= '0';
					--read
					WHEN state_read_0 =>
						flash_clk <= '0';
						flash_csn <= '0';
						internal_data   <= x"000B" & addr & '0' & x"00";

						count     <= to_unsigned(39, 6);
						state     <= state_read_1 WHEN action ELSE UNAFFECTED;
					WHEN state_read_1 =>
						flash_clk <= '0';
						flash_si  <= internal_data(to_integer(count));

						count     <= count - 1    WHEN action ELSE UNAFFECTED;
						state     <= state_read_2 WHEN action ELSE UNAFFECTED;
					WHEN state_read_2 =>
						flash_clk <= '1';
						state     <= state_read_3 WHEN action AND count ?= "111111"
								ELSE state_read_1 WHEN action
								ELSE UNAFFECTED;
					WHEN state_read_3 =>
						count     <=  to_unsigned(15, 6);
						flash_clk <= '0';
						state     <= state_read_4 WHEN action ELSE UNAFFECTED;
					WHEN state_read_4 =>
						flash_clk <= '0';
						--I know  about blocking/nonblocking
						internal_data(to_integer(count)) <= flash_so;
						count     <= count - 1    WHEN action ELSE UNAFFECTED;
						state     <= state_read_5 WHEN action ELSE UNAFFECTED;
					WHEN state_read_5 =>
						flash_clk <= '1';
						state     <= state_read_6 WHEN action AND count ?= "111111"
								ELSE state_read_4 WHEN action
								ELSE UNAFFECTED;
					WHEN state_read_6 =>
						/*out elsewhere*/
						state     <= state_idle;
					--write
					--write en
					WHEN state_write_en_0 =>
						flash_clk <= '0';
						flash_csn <= '0';
						internal_data   <= x"000000000006";
		
						count     <= to_unsigned(7, 6);
						state     <= state_write_en_1 WHEN action ELSE UNAFFECTED;
					WHEN state_write_en_1 =>
						flash_clk <= '0';
						flash_si  <= internal_data(to_integer(count));
		
						count     <= count - 1        WHEN action ELSE UNAFFECTED;
						state     <= state_write_en_2 WHEN action ELSE UNAFFECTED;
					WHEN state_write_en_2 =>
						flash_clk <= '1';
						state     <= state_write_en_3 WHEN action AND count ?= "111111"
								ELSE state_write_en_2 WHEN action
								ELSE UNAFFECTED;
					WHEN state_write_en_3 =>
						flash_csn <= '1';
						state     <= state_write_0    WHEN action AND count ?= "111111"
								ELSE state_write_en_2 WHEN action
								ELSE UNAFFECTED;
					--actual stuff
					WHEN state_write_0 =>
						flash_clk <= '0';
						flash_csn <= '0';
						internal_data   <= x"02" & addr & '0' & data_in;
		
						count     <= to_unsigned(47, 6);
						state     <= state_write_1 WHEN action ELSE UNAFFECTED;
					WHEN state_write_1 =>
						flash_clk <= '0';
						flash_si  <= internal_data(to_integer(count));
		
						count     <= count - 1     WHEN action ELSE UNAFFECTED;
						state     <= state_write_2 WHEN action ELSE UNAFFECTED;
					WHEN state_write_2 =>
						flash_clk <= '1';
						state     <= state_write_3 WHEN action AND count ?= "111111"
								ELSE state_write_1 WHEN action
								ELSE UNAFFECTED;
					WHEN state_write_3 =>
						flash_csn <= '1';
						state     <= state_idle;
				END CASE;
			END IF;
		END PROCESS;


		regs: PROCESS(ALL) IS
		BEGIN
			IF rising_edge(clk) THEN
		--		action <= '1';
				action <= NOT action;
			END IF;
		END PROCESS regs;
	-- synthesis translate_off
	END GENERATE gen_sim_fast;
	-- synthesis translate_on
END ARCHITECTURE arch;
