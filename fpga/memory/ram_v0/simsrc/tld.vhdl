LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE std.env.finish;
USE ieee.numeric_std.ALL;
USE std.textio.ALL;
USE ieee.std_logic_textio.ALL;

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
			clk        : IN    std_ulogic;
			nres       : IN    std_ulogic;

			mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
			mem_addr_load  :   OUT std_ulogic;
			mem_nwe        :   OUT std_ulogic;
			mem_ncs        :   OUT std_ulogic;
			mem_noe        :   OUT std_ulogic;

			address        : IN    std_ulogic_vector(15 DOWNTO 0);
			io_data        : INOUT std_ulogic_vector(15 DOWNTO 0);

			read           : IN    std_ulogic;
			write          : IN    std_ulogic;
			ready          :   OUT std_ulogic;
			output         :   OUT std_ulogic);
	END COMPONENT memory_controller;

	CONSTANT frequency : real := 27.0E6; 
	CONSTANT cycle     : real := 1.0 / frequency;

	SIGNAL clk : std_ulogic := '0';
	SIGNAL nres : std_ulogic := '1';

	SIGNAL state, state_in : unsigned(4 DOWNTO 0) := (OTHERS => '0');

	SIGNAL mem_bus       : std_logic_vector(15 DOWNTO 0);
	SIGNAL mem_addr_load : std_ulogic;
	SIGNAL mem_nwe       : std_ulogic; 
	SIGNAL mem_ncs       : std_ulogic;
	SIGNAL mem_noe       : std_ulogic;

	SIGNAL c_addr        : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL c_data        : std_logic_vector(15 DOWNTO 0); 
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
			mem_bus        => mem_bus,
			mem_addr_load  => mem_addr_load,
			mem_nwe        => mem_nwe, 
			mem_ncs        => mem_ncs,
			mem_noe        => mem_noe,

			clk              => clk,
			nres             => nres,

			address          => c_addr,
			io_data          => c_data, 
			write            => c_write,
			read             => c_read,
			ready            => c_ready,
			output           => c_output);

	PROCESS IS
		-- Convert string to std_logic_vector, assuming characters in '0' to '9',
		-- 'A' to 'F', or 'a' to 'f'.
		FUNCTION str_to_slv(str : string) RETURN std_logic_vector IS
		  ALIAS str_norm : string(1 TO str'length) IS str;
		  VARIABLE char_v : character := ' ';
		  VARIABLE val_of_char_v : natural := 0;
		  VARIABLE res_v : std_logic_vector(4 * str'length - 1 DOWNTO 0) := (OTHERS => '0');
		BEGIN
		  FOR str_norm_idx IN str_norm'range LOOP
		    char_v := str_norm(str_norm_idx);
		    CASE char_v IS
		      WHEN '0' TO '9' => val_of_char_v := character'pos(char_v) - character'pos('0');
		      WHEN 'A' TO 'F' => val_of_char_v := character'pos(char_v) - character'pos('A') + 10;
		      WHEN 'a' TO 'f' => val_of_char_v := character'pos(char_v) - character'pos('a') + 10;
		      WHEN OTHERS => REPORT "str_to_slv: Invalid characters for convert" SEVERITY ERROR;
		    END CASE;
		    res_v(res_v'left - 4 * str_norm_idx + 4 DOWNTO res_v'left - 4 * str_norm_idx + 1) :=
		      std_logic_vector(to_unsigned(val_of_char_v, 4));
		  END LOOP;

		  RETURN res_v;

		END FUNCTION;

		FUNCTION slv_to_str(slv : std_ulogic_vector(15 DOWNTO 0)) RETURN string IS
			VARIABLE index : natural;
			VARIABLE str   : string(8 DOWNTO 1);
			VARIABLE sub   : std_ulogic_vector(3 DOWNTO 0);
		BEGIN
			index := 0;
	
			WHILE index < 4 LOOP
				sub(3) := slv(4 * index + 3);
				sub(2) := slv(4 * index + 2);
				sub(1) := slv(4 * index + 1);
				sub(0) := slv(4 * index + 0);
	
				CASE sub IS 
					WHEN x"0"   => str(index + 1) := '0';
					WHEN x"1"   => str(index + 1) := '1';
					WHEN x"2"   => str(index + 1) := '2';
					WHEN x"3"   => str(index + 1) := '3';
					WHEN x"4"   => str(index + 1) := '4';
					WHEN x"5"   => str(index + 1) := '5';
					WHEN x"6"   => str(index + 1) := '6';
					WHEN x"7"   => str(index + 1) := '7';
					WHEN x"8"   => str(index + 1) := '8';
					WHEN x"9"   => str(index + 1) := '9';
					WHEN x"A"   => str(index + 1) := 'A';
					WHEN x"B"   => str(index + 1) := 'B';
					WHEN x"C"   => str(index + 1) := 'C';
					WHEN x"D"   => str(index + 1) := 'D';
					WHEN x"E"   => str(index + 1) := 'E';
					WHEN x"F"   => str(index + 1) := 'F';
					WHEN x"Z"   => str(index + 1) := '_';
					WHEN OTHERS => str(index + 1) := 'X';
				END CASE;
	
				index := index + 1;
			END LOOP;
			
			RETURN str;
	
		END FUNCTION slv_to_str;

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
			REPORT "SIMULATION FAILED";
		END IF;

		finish;
	END PROCESS;
END ARCHITECTURE arch;
