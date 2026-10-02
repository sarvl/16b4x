LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.pkg_types.ALL;

ENTITY mem_driver_low_level IS 
	PORT(
		eio_mem_bus      : INOUT std_ulogic_vector(15 DOWNTO 0);
		eo_mem_addr_load :   OUT std_ulogic;
		eo_mem_nwe       :   OUT std_ulogic;
		eo_mem_ncs       :   OUT std_ulogic;
		eo_mem_noe       :   OUT std_ulogic;

		i_clk            : IN    std_ulogic;
		i_nres           : IN    std_ulogic;

		i_addr           : IN    t_memaddr;
		io_data          : INOUT t_rword;
		i_write          : IN    std_ulogic;
		i_read           : IN    std_ulogic;
		o_ready          :   OUT std_ulogic);
END ENTITY mem_driver_low_level;

ARCHITECTURE arch of mem_driver_low_level IS 
	TYPE t_state IS (
		idle,
		rd_addr_set_0, rd_addr_set_1, rd_data_read,
		wr_addr_set_0, wr_addr_set_1, wr_data_set_0, wr_data_set_1
		);

	SIGNAL state, state_in : t_state := idle;
	SIGNAL data, data_in   : t_word;
	SIGNAL addr, addr_in   : t_memaddr;

BEGIN
	eo_mem_ncs <= '0';

	PROCESS(ALL) IS
	BEGIN
		CASE state IS
		WHEN idle =>
			eio_mem_bus      <= x"ZZZZ";
			eo_mem_addr_load <= '0';
			eo_mem_nwe       <= '1';
			eo_mem_noe       <= '1';
			
			addr_in <= i_addr;
			data_in <= data;
			io_data <= data;
			o_ready <= '1';

			IF i_write THEN
				state_in <= wr_addr_set_0;
			ELSIF i_read THEN
				state_in <= rd_addr_set_0;
			ELSE
				state_in <= idle;
			END IF;
				
		WHEN rd_addr_set_0 =>
			eio_mem_bus       <= addr;
			eo_mem_addr_load  <= '1';
			eo_mem_nwe        <= '1';
			eo_mem_noe        <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= rd_addr_set_1;

		WHEN rd_addr_set_1 =>
			eio_mem_bus      <= addr;
			eo_mem_addr_load <= '0';
			eo_mem_nwe       <= '1';
			eo_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= rd_data_read;

		WHEN rd_data_read =>
			eio_mem_bus       <= x"ZZZZ";
			eo_mem_addr_load <= '0';
			eo_mem_nwe       <= '1';
			eo_mem_noe       <= '0';
			
			addr_in <= addr;
			data_in <= eio_mem_bus;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= idle;

		WHEN wr_addr_set_0 =>
			eio_mem_bus       <= addr;
			eo_mem_addr_load <= '1';
			eo_mem_nwe       <= '1';
			eo_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= io_data;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= wr_addr_set_1;

		WHEN wr_addr_set_1 =>
			eio_mem_bus       <= addr;
			eo_mem_addr_load <= '0';
			eo_mem_nwe       <= '1';
			eo_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= wr_data_set_0;

		WHEN wr_data_set_0 =>
			eio_mem_bus       <= data;
			eo_mem_addr_load <= '0';
			eo_mem_nwe       <= '1';
			eo_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= wr_data_set_1;

		WHEN wr_data_set_1 =>
			eio_mem_bus       <= data;
			eo_mem_addr_load <= '0';
			eo_mem_nwe       <= '0';
			eo_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			o_ready <= '0';

			state_in <= idle;
		END CASE;
	END PROCESS;

	
	PROCESS(i_clk) IS
	BEGIN
		IF rising_edge(i_clk) THEN
			IF NOT i_nres THEN
				state <= idle;
				addr  <= (OTHERS => '0');
				data  <= (OTHERS => '0');
			ELSE
				state <= state_in;

				addr  <= addr_in;
				data  <= data_in;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
