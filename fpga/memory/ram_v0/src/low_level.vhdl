LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY mem_driver_low_level IS 
	PORT(
		p_mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
		p_mem_addr_load  :   OUT std_ulogic;
		p_mem_nwe        :   OUT std_ulogic;
		p_mem_ncs        :   OUT std_ulogic;
		p_mem_noe        :   OUT std_ulogic;

		clk              : IN    std_ulogic;
		nres             : IN    std_ulogic;

		address          : IN    std_ulogic_vector(15 DOWNTO 0);
		io_data          : INOUT std_ulogic_vector(15 DOWNTO 0);
		write            : IN    std_ulogic;
		read             : IN    std_ulogic;
		ready            :   OUT std_ulogic);
END ENTITY mem_driver_low_level;


ARCHITECTURE arch of mem_driver_low_level IS 

	TYPE t_state IS (
		idle,
		rd_addr_set_0, rd_addr_set_1, rd_data_read,
		wr_addr_set_0, wr_addr_set_1, wr_data_set_0, wr_data_set_1
		);

	SIGNAL state, state_in : t_state := idle;

	SIGNAL data, data_in, addr, addr_in : std_ulogic_vector(15 DOWNTO 0);

BEGIN
	p_mem_ncs <= '0';

	PROCESS(ALL) IS
	BEGIN
		CASE state IS
		WHEN idle =>
			p_mem_bus       <= x"ZZZZ";
			p_mem_addr_load <= '0';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '1';
			
			addr_in <= address;
			data_in <= data;
			io_data <= data;
			ready   <= '1';

			IF write THEN
				state_in <= wr_addr_set_0;
			ELSIF read THEN
				state_in <= rd_addr_set_0;
			ELSE
				state_in <= idle;
			END IF;
				
		WHEN rd_addr_set_0 =>
			p_mem_bus       <= addr;
			p_mem_addr_load <= '1';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= rd_addr_set_1;

		WHEN rd_addr_set_1 =>
			p_mem_bus       <= addr;
			p_mem_addr_load <= '0';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= rd_data_read;

		WHEN rd_data_read =>
			p_mem_bus       <= x"ZZZZ";
			p_mem_addr_load <= '0';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '0';
			
			addr_in <= addr;
			data_in <= p_mem_bus;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= idle;

		WHEN wr_addr_set_0 =>
			p_mem_bus       <= addr;
			p_mem_addr_load <= '1';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= io_data;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= wr_addr_set_1;

		WHEN wr_addr_set_1 =>
			p_mem_bus       <= addr;
			p_mem_addr_load <= '0';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= wr_data_set_0;

		WHEN wr_data_set_0 =>
			p_mem_bus       <= data;
			p_mem_addr_load <= '0';
			p_mem_nwe       <= '1';
			p_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= wr_data_set_1;

		WHEN wr_data_set_1 =>
			p_mem_bus       <= data;
			p_mem_addr_load <= '0';
			p_mem_nwe       <= '0';
			p_mem_noe       <= '1';
			
			addr_in <= addr;
			data_in <= data;
			io_data <= x"ZZZZ";
			ready   <= '0';

			state_in <= idle;
		END CASE;
	END PROCESS;

	
	PROCESS(clk) IS
	BEGIN
		IF rising_edge(clk) THEN
			IF NOT nres THEN
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
