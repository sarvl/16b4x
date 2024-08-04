LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY memory_controller IS
	GENERIC (
		sim      : boolean := false;
		sim_fast : boolean := false
	);
	PORT(
		--actual IF
		clk  : IN    std_ulogic;
		re   : IN    std_ulogic;
		we   : IN    std_ulogic;
		rdy  :   OUT std_ulogic;
		addr : IN    std_ulogic_vector(15 DOWNTO 0);
		data : INOUT std_ulogic_vector(15 DOWNTO 0);

		--external IF
		mem_re   :   OUT std_ulogic; 
		mem_we   :   OUT std_ulogic; 
		mem_data : INOUT std_ulogic_vector(7 DOWNTO 0);
		mem_rdy  : IN    std_ulogic; 
		mem_clk  :   OUT std_ulogic
	);
END ENTITY;

ARCHITECTURE arch OF memory_controller IS
	SIGNAL addr_saved, data_saved : std_ulogic_vector(15 DOWNTO 0) := x"0000";

	TYPE t_state IS (
		state_idle         , state_read_addr_0  , state_read_addr_1  , state_read_addr_2  , state_read_addr_3  ,
		state_read_wait_0  , state_read_wait_1  , state_read_read_0  , state_read_read_1  , state_write_addr_0 ,
		state_write_addr_1 , state_write_addr_2 , state_write_addr_3 , state_write_write_0, state_write_write_1,
		state_write_write_2, state_write_write_3, state_write_wait_0 , state_write_wait_1 , state_write_fin    
	);
	SIGNAL state, state_in       : t_state := state_idle;


	SIGNAL change_counter : unsigned(6 DOWNTO 0) := (OTHERS => '1');

BEGIN
	-- synthesis translate_off
	gen_sim_fast:
	IF sim AND sim_fast GENERATE
		pu: ENTITY work.ram_data(arch) PORT MAP(
			clk  => clk,
			addr => addr,
			data => data,
			re   => re,
			we   => we
		);

		rdy <= '1';

	ELSE GENERATE
	-- synthesis translate_on

	rdy  <= '1'        WHEN (state = state_idle AND re = '0' AND we = '0')
	                     OR state = state_read_read_1
	                     OR state = state_write_fin
	   ELSE '0';
	data <= data_saved WHEN state = state_read_read_1
	   ELSE x"ZZZZ";

	PROCESS(ALL) IS BEGIN
		CASE state IS 
			WHEN state_idle =>
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"ZZ";
				mem_clk  <= '0';

				state_in <= state_read_addr_0  WHEN re
				       ELSE state_write_addr_0 WHEN we
				       ELSE state_idle;

			WHEN state_read_addr_0 =>
			--setup addr high
				mem_re   <= '1';
				mem_we   <= '0';
				mem_data <= addr_saved(15 DOWNTO 8);
				mem_clk  <= '0';

				state_in <= state_read_addr_1;
			WHEN state_read_addr_1 =>
			--clock high
				mem_re   <= '1';
				mem_we   <= '0';
				mem_data <= addr_saved(15 DOWNTO 8);
				mem_clk  <= '1';

				state_in <= state_read_addr_2;
			WHEN state_read_addr_2 =>
			--setup addr low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= addr_saved( 7 DOWNTO 0);
				mem_clk  <= '1';

				state_in <= state_read_addr_3;
			WHEN state_read_addr_3 =>
			--clock low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= addr_saved( 7 DOWNTO 0);
				mem_clk  <= '0';

				state_in <= state_read_wait_0;
			WHEN state_read_wait_0 =>
			--clock low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"ZZ";
				mem_clk  <= '0';

				state_in <= state_read_read_0 WHEN mem_rdy
				       ELSE state_read_wait_1;
			WHEN state_read_wait_1 =>
			--clock high
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"ZZ";
				mem_clk  <= '1';

				state_in <= state_read_wait_0;
			--read happens in clocks process
			WHEN state_read_read_0 =>
			--clock low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"ZZ";
				mem_clk  <= '1';

				state_in <= state_read_read_1;
			--output to cpu
			WHEN state_read_read_1 =>
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"ZZ";
				mem_clk  <= '0';

				state_in <= state_idle;
			
			WHEN state_write_addr_0 =>
			--setup addr high
				mem_re   <= '0';
				mem_we   <= '1';
				mem_data <= addr_saved(15 DOWNTO 8);
				mem_clk  <= '0';

				state_in <= state_write_addr_1;
			WHEN state_write_addr_1 =>
			--clock high
				mem_re   <= '0';
				mem_we   <= '1';
				mem_data <= addr_saved(15 DOWNTO 8);
				mem_clk  <= '1';

				state_in <= state_write_addr_2;
			WHEN state_write_addr_2 =>
			--setup addr low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= addr_saved( 7 DOWNTO 0);
				mem_clk  <= '1';

				state_in <= state_write_addr_3;
			WHEN state_write_addr_3 =>
			--clock low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= addr_saved( 7 DOWNTO 0);
				mem_clk  <= '0';

				state_in <= state_write_write_0;
			WHEN state_write_write_0 =>
			--setup data high
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= data_saved(15 DOWNTO 8);
				mem_clk  <= '0';

				state_in <= state_write_write_1;
			WHEN state_write_write_1 =>
			--clock high
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= data_saved(15 DOWNTO 8);
				mem_clk  <= '1';

				state_in <= state_write_write_2;
			WHEN state_write_write_2 =>
			--setup data low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= data_saved( 7 DOWNTO 0);
				mem_clk  <= '1';

				state_in <= state_write_write_3;
			WHEN state_write_write_3 =>
			--clock low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= data_saved( 7 DOWNTO 0);
				mem_clk  <= '0';

				state_in <= state_write_wait_0;
			WHEN state_write_wait_0 =>
			--clock low
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"00";
				mem_clk  <= '0';

				state_in <= state_write_wait_1;
			--write happens in clocks process
			WHEN state_write_wait_1 =>
			--clock high
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"00";
				mem_clk  <= '1';

				state_in <= state_write_fin WHEN mem_rdy
				       ELSE state_write_wait_0;
			--output to cpu
			WHEN state_write_fin =>
				mem_re   <= '0';
				mem_we   <= '0';
				mem_data <= x"00";
				mem_clk  <= '0';

				state_in <= state_idle;
		END CASE;
	END PROCESS;



	clocks:
	PROCESS(ALL) IS BEGIN
		IF rising_edge(clk) THEN
			state <= state_in WHEN state = state_idle OR state = state_read_read_1 OR state = state_write_fin
			                    OR change_counter = x"00"
			                    OR sim
			    ELSE state;

			addr_saved <= addr                               WHEN re OR we 
			         ELSE UNAFFECTED;
			data_saved <= data                               WHEN       we 
			         ELSE mem_data & data_saved( 7 DOWNTO 0) WHEN state = state_read_wait_0
			         ELSE data_saved(15 DOWNTO 8) & mem_data WHEN state = state_read_read_0
			         ELSE UNAFFECTED;


			change_counter <= change_counter - 1;
		END IF;
	END PROCESS;
	-- synthesis translate_off
	END GENERATE gen_sim_fast;
	-- synthesis translate_on
END ARCHITECTURE;
