LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY screen_driver IS 
	PORT(
		clk        : IN    std_ulogic;
		nres       : IN    std_ulogic;
		
		data       : IN    std_ulogic_vector(7 DOWNTO 0);

		valid      : IN    std_ulogic;
		ready      :   OUT std_ulogic;
		
		screen_res :   OUT std_ulogic;
		screen_dc  :   OUT std_ulogic;
		screen_cs  :   OUT std_ulogic;
		screen_clk :   OUT std_ulogic;
		screen_din :   OUT std_ulogic);
END ENTITY screen_driver;


ARCHITECTURE arch OF screen_driver IS
	TYPE t_data IS ARRAY (natural RANGE<>) OF std_ulogic_vector(7 DOWNTO 0);

	CONSTANT commands_init : t_data(0 TO 20) := (
--		x"20", x"00", x"A1", x"8D", x"14", x"AF"
		x"A8", x"3F", x"D3", x"00", x"40", x"A0", x"C0", x"DA", x"02", x"20", 
		x"00", x"81", x"FF", x"A4", x"A6", x"D5", x"80", x"8D", x"14", x"AF",
		x"40"
	);

	SIGNAL state    , state_in       : unsigned(3 DOWNTO 0);
	SIGNAL wait_cnt , wait_cnt_in    : unsigned(22 DOWNTO 0);
	SIGNAL cmd_index, cmd_index_in   : unsigned(4 DOWNTO 0);
	

	CONSTANT wait_cnt_reset_short    : unsigned(wait_cnt'range) := (2 DOWNTO 0 => '1', OTHERS => '0');
	CONSTANT wait_cnt_reset_long     : unsigned(wait_cnt'range) := (OTHERS => '1');

	SIGNAL bit_ind, bit_ind_in       : unsigned(2 DOWNTO 0);

	SIGNAL data_saved, data_saved_in : std_ulogic_vector(7 DOWNTO 0);
BEGIN
	screen_din <= data_saved(to_integer(bit_ind));


	PROCESS(ALL) IS 
	BEGIN
		IF nres = '0' THEN 
			state_in     <= x"0";

			ready        <= '0';
			screen_res   <= '1';
			screen_dc    <= '0';
			screen_cs    <= '0';
			screen_clk   <= '0';

			data_saved_in <= x"00";

			wait_cnt_in   <= wait_cnt_reset_long;
			
			bit_ind_in   <= "111";
			cmd_index_in <= "00000";

		ELSE

		CASE state IS

			WHEN x"0" => 
				ready        <= '0';
				screen_res   <= '0';
				screen_dc    <= '0';
				screen_cs    <= '0';
				screen_clk   <= '0';
				
				data_saved_in <= data_saved;

				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= "111";

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_long;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;
				
			WHEN x"1" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '0';
				screen_cs    <= '1';
				screen_clk   <= '0';
				
				data_saved_in <= data_saved;
				
				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= "111";

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_short;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN x"2" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '0';
				screen_cs    <= '0';
				screen_clk   <= '0';
				
				data_saved_in <= data_saved;
				
				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= "111";

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_long;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN x"3" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '0';
				screen_cs    <= '0';
				screen_clk   <= '0';
				
				data_saved_in <= data_saved;
				
				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= "111";

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_short;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN x"4" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '0';
				screen_cs    <= '0';
				screen_clk   <= '0';
				
				data_saved_in <= commands_init(to_integer(cmd_index));

				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= "111";

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_short;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;
				
			WHEN x"5" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '0';
				screen_cs    <= '0';
				screen_clk   <= '0';
				
				data_saved_in <= commands_init(to_integer(cmd_index));

				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_short;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN x"6" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '0';
				screen_cs    <= '0';
				screen_clk   <= '1';
				
				data_saved_in <= commands_init(to_integer(cmd_index));

				if wait_cnt = "0" THEN 
					IF bit_ind = "000" THEN
						IF cmd_index = "10100" THEN
							wait_cnt_in <= wait_cnt_reset_long;
							state_in <= state + 1;
							cmd_index_in <= cmd_index;

						ELSE
							wait_cnt_in <= wait_cnt_reset_short;
							state_in <= x"3";
							cmd_index_in <= cmd_index + 1;
						END IF;

					ELSE 
						wait_cnt_in <= wait_cnt_reset_short;
						state_in     <= state - 1;
						cmd_index_in <= cmd_index;
					END IF;

					bit_ind_in  <= bit_ind - 1;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN x"7" => 
				ready        <= '1';
				screen_res   <= '1';
				screen_dc    <= '1';
				screen_cs    <= '0';
				screen_clk   <= '0';
				

				IF valid = '1' THEN 
					data_saved_in <= data;
					state_in    <= state + 1;
				ELSE
					data_saved_in <= data_saved;
					state_in    <= state;
				END IF;

				cmd_index_in <= cmd_index;
				bit_ind_in   <= "111";

				wait_cnt_in <= wait_cnt_reset_short;

			WHEN x"8" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '1';
				screen_cs    <= '0';
				screen_clk   <= '0';
				
				data_saved_in <= data_saved;

				if wait_cnt = "0" THEN 
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state + 1;
					wait_cnt_in <= wait_cnt_reset_short;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN x"9" => 
				ready        <= '0';
				screen_res   <= '1';
				screen_dc    <= '1';
				screen_cs    <= '0';
				screen_clk   <= '1';
				
				data_saved_in <= data_saved;

				if wait_cnt = "0" THEN 
					IF bit_ind = "000" THEN
						cmd_index_in <= cmd_index;
						state_in <= state - 2;

					ELSE 
						cmd_index_in <= cmd_index;
						state_in     <= state - 1;
					END IF;

					wait_cnt_in <= wait_cnt_reset_short;
					bit_ind_in  <= bit_ind - 1;
				ELSE
					cmd_index_in <= cmd_index;
					bit_ind_in   <= bit_ind;

					state_in    <= state;
					wait_cnt_in <= wait_cnt - 1;
				END IF;

			WHEN OTHERS => 
				ready        <= '-';
				screen_res   <= '-';
				screen_dc    <= '-';
				screen_cs    <= '0';
				screen_clk   <= '0';
				data_saved_in <= (OTHERS => '-');
				
				cmd_index_in <= (OTHERS => '-');
				state_in     <= state;
				bit_ind_in   <= "---";
				wait_cnt_in  <= (OTHERS => '-');
		END CASE;
		END IF;
	END PROCESS;


	PROCESS(clk) IS
	BEGIN
		IF rising_edge(clk) THEN
			state      <= state_in;
			wait_cnt   <= wait_cnt_in;
			cmd_index  <= cmd_index_in;
			bit_ind    <= bit_ind_in;

			data_saved <= data_saved_in;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
