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
		GENERIC(
			g_cache_enabled    : boolean := False;
			g_cache_size_log_2 : integer RANGE 1 TO 15 := 1);
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
	SIGNAL nres  : std_ulogic := '0';

	SIGNAL mem_bus       : std_logic_vector(15 DOWNTO 0);
	SIGNAL mem_addr_load : std_ulogic;
	SIGNAL mem_nwe       : std_ulogic; 
	SIGNAL mem_ncs       : std_ulogic;
	SIGNAL mem_noe       : std_ulogic;

	SIGNAL c_addr        : t_memaddr := x"0000";
	SIGNAL c_data        : t_rword := x"0000";
	SIGNAL c_write       : std_ulogic := '0';
	SIGNAL c_read        : std_ulogic := '0';
	SIGNAL c_ready       : std_ulogic;
	SIGNAL c_output      : std_ulogic;

BEGIN
	u_sr: simram
		PORT MAP(
			mem_bus        => mem_bus,
			mem_addr_load  => mem_addr_load,
			mem_nwe        => mem_nwe, 
			mem_ncs        => mem_ncs,
			mem_noe        => mem_noe);

	mdll: memory_controller 
		GENERIC MAP(
			g_cache_enabled    => True,
			g_cache_size_log_2 => 5)
		PORT MAP(
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

		VARIABLE cycle_count_write : integer := 0;
		VARIABLE cycle_count_read  : integer := 0;
		VARIABLE cycle_count_init  : integer := 0;
		VARIABLE cycle_count       : integer := 0;

		PROCEDURE clock_low IS
		BEGIN
			clk <= '0';
			WAIT FOR cycle / 2.0 * 1000 MS;
		END PROCEDURE clock_low;

		PROCEDURE clock_high IS
		BEGIN
			clk <= '1';
			WAIT FOR cycle / 2.0 * 1000 MS;
		END PROCEDURE clock_high;

		PROCEDURE clock_tick IS
		BEGIN
			clock_low;
			clock_high;

			cycle_count       := cycle_count + 1;
		END PROCEDURE clock_tick;

		PROCEDURE clock_tick_write IS
		BEGIN
			clock_tick; 

			cycle_count_write := cycle_count_write + 1;
		END PROCEDURE clock_tick_write;

		PROCEDURE clock_tick_read IS
		BEGIN
			clock_tick; 

			cycle_count_read := cycle_count_read + 1;
		END PROCEDURE clock_tick_read;

		PROCEDURE clock_tick_init IS
		BEGIN
			clock_tick; 

			cycle_count_init := cycle_count_init + 1;
		END PROCEDURE clock_tick_init;

	BEGIN
		file_open(instr, "mem_trace.txt");

		nres <= '0';

		clock_tick_init;

		nres <= '1';
		
		WHILE NOT c_ready LOOP
			clock_tick_init;
		END LOOP;

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
					
				clock_tick_write;

				c_addr  <= addr;
				c_data  <= data;
				c_write <= '1';
				c_read  <= '0';

				WHILE NOT c_output LOOP
					clock_tick_write;
				END LOOP;

				c_addr  <= x"0000";
				c_data  <= x"0000";
				c_write <= '0';
				c_read  <= '0';

				WHILE NOT c_ready LOOP
					clock_tick_write;
				END LOOP;

			ELSE
				REPORT slv_to_str(addr) & " (expected: " & slv_to_str(data) & ")";
				
				c_addr  <= addr;
				c_data  <= x"ZZZZ";
				c_write <= '0';
				c_read  <= '1';

				WHILE NOT c_output LOOP
					clock_tick_read;
				END LOOP;

				--propagate changes
				clock_low;

				IF c_data /= data THEN
					REPORT "MISMATCH, RECEIVED: "  & slv_to_str(c_data);

					failed := True;
				END IF;

				clock_high;
				cycle_count_read := cycle_count_read + 1;

			END IF;
		
		END LOOP;
		file_close(instr);

		REPORT "";
		REPORT "end of sim";
		REPORT "";
		REPORT "init : " & integer'image(cycle_count_init) & " cycles";
		REPORT "write: " & integer'image(cycle_count_write) & " cycles";
		REPORT "read : " & integer'image(cycle_count_read) & " cycles";
		REPORT "";
		REPORT "total: " & integer'image(cycle_count) & " cycles";
		REPORT "or   " & real'image(real(cycle_count) * cycle) & " s";

		IF failed THEN
			REPORT "";
			REPORT "SIMULATION FAILED" SEVERITY ERROR;
		END IF;

		finish;
	END PROCESS;
END ARCHITECTURE arch;
