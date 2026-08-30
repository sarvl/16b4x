LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE std.env.finish;
USE ieee.numeric_std.ALL;
USE std.textio.ALL;
USE ieee.std_logic_textio.ALL;

USE work.pkg_types.ALL;
USE work.pkg_string_conv.ALL;

ENTITY tld IS 
END ENTITY tld;

ARCHITECTURE arch of tld IS 
	COMPONENT simram IS
		PORT(
			mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
			mem_addr_load  : IN    std_ulogic;
			mem_nwe        : IN    std_ulogic;
			mem_ncs        : IN    std_ulogic;
			mem_noe        : IN    std_ulogic;
		);
	END COMPONENT simram;

	COMPONENT memory_controller IS 
		PORT(
			i_clk            : IN    std_ulogic;
			i_nres           : IN    std_ulogic;

			eio_mem_bus      : INOUT std_ulogic_vector(15 DOWNTO 0);
			eo_mem_addr_load :   OUT std_ulogic;
			eo_mem_nwe       :   OUT std_ulogic;
			eo_mem_ncs       :   OUT std_ulogic;
			eo_mem_noe       :   OUT std_ulogic;

			i_addr           : IN    t_memaddr;
			io_data          : INOUT t_rword;

			i_read           : IN    std_ulogic;
			i_write          : IN    std_ulogic;
			o_ready          :   OUT std_ulogic;
			o_output         :   OUT std_ulogic);
	END COMPONENT memory_controller;

	CONSTANT frequency : real := 27.0E6; 
	CONSTANT cycle     : real := 1.0 / frequency;

	SIGNAL clk   : std_ulogic := '0';
	SIGNAL nres  : std_ulogic := '1';

	SIGNAL mem_bus       : std_logic_vector(15 DOWNTO 0);
	SIGNAL mem_addr_load : std_ulogic;
	SIGNAL mem_nwe       : std_ulogic; 
	SIGNAL mem_ncs       : std_ulogic;
	SIGNAL mem_noe       : std_ulogic;

	SIGNAL c_addr        : t_memaddr;
	SIGNAL c_data        : t_rword;
	SIGNAL c_write       : std_ulogic;
	SIGNAL c_read        : std_ulogic;
	SIGNAL c_ready       : std_ulogic;
	SIGNAL c_output      : std_ulogic;

BEGIN
	u_sr: simram PORT MAP(
			mem_bus        => mem_bus,
			mem_addr_load  => mem_addr_load,
			mem_nwe        => mem_nwe, 
			mem_ncs        => mem_ncs,
			mem_noe        => mem_noe);

	mdll: memory_controller PORT MAP(
			eio_mem_bus      => mem_bus,
			eo_mem_addr_load => mem_addr_load,
			eo_mem_nwe       => mem_nwe, 
			eo_mem_ncs       => mem_ncs,
			eo_mem_noe       => mem_noe,

			i_clk            => clk,
			i_nres           => nres,

			i_addr           => c_addr,
			io_data          => c_data, 
			i_write          => c_write,
			i_read           => c_read,
			o_ready          => c_ready,
			o_output         => c_output);

	PROCESS IS
		FILE instr : text; 

		VARIABLE index : unsigned(15 DOWNTO 0) := x"0000";
		VARIABLE size  : natural;
		VARIABLE temp_storage : string(13 DOWNTO 1);

		VARIABLE addr : std_ulogic_vector(15 DOWNTO 0);
		VARIABLE data : std_ulogic_vector(15 DOWNTO 0);
	
		VARIABLE is_write : boolean;
		VARIABLE failed   : boolean := False;

		VARIABLE cycle_count : integer := 0;

		PROCEDURE clock_tick IS
		BEGIN
			clk <= '0';
			WAIT FOR cycle / 2.0 * 1000 MS;
			clk <= '1';
			WAIT FOR cycle / 2.0 * 1000 MS;

			cycle_count := cycle_count + 1;
		END PROCEDURE clock_tick;
	BEGIN
		file_open(instr, "mem_trace.txt");

		nres <= '0';
		
		clock_tick;

		nres <= '1';

		WHILE NOT endfile(instr) LOOP
			read(instr, temp_storage, size);
			
			addr := str_to_slv(temp_storage(11 DOWNTO  8));
			data := str_to_slv(temp_storage( 4 DOWNTO  1));

			is_write := temp_storage(13) = 'W';

			WHILE NOT c_ready LOOP
				clock_tick;
			END LOOP;

			IF is_write THEN
				REPORT slv_to_str(addr) & " <= " & slv_to_str(data);

				c_addr  <= addr;
				c_data  <= data;
				c_write <= '1';
				c_read  <= '0';
					
				clock_tick;

				c_addr  <= addr;
				c_data  <= data;
				c_write <= '1';
				c_read  <= '0';

				WHILE NOT c_output LOOP
					clock_tick;
				END LOOP;

			ELSE
				REPORT slv_to_str(addr) & " (expected: " & slv_to_str(data) & ")";
				
				c_addr  <= addr;
				c_data  <= x"ZZZZ";
				c_write <= '0';
				c_read  <= '1';

				WHILE NOT c_output LOOP
					clock_tick;
				END LOOP;

				c_addr  <= addr;
				c_data  <= x"ZZZZ";
				c_write <= '0';
				c_read  <= '0';

				IF c_data /= data THEN
					REPORT "MISMATCH, RECEIVED: "  & slv_to_str(c_data);

					failed := True;
				END IF;

				clock_tick;

			END IF;
		
		END LOOP;
		file_close(instr);

		REPORT "";
		REPORT "end of sim";
		REPORT "took " & integer'image(cycle_count) & " cycles";
		REPORT "or   " & to_string(real(cycle_count) * cycle) & " s";

		IF failed THEN
			REPORT "";
			REPORT "SIMULATION FAILED" SEVERITY ERROR;
		END IF;

		finish;
	END PROCESS;
END ARCHITECTURE arch;
