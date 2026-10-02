LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.pkg_types.ALL;

PACKAGE pkg_string_conv IS
	FUNCTION str_to_slv(str : string) RETURN std_logic_vector;
	FUNCTION slv_to_str(slv : std_ulogic_vector) RETURN string;
END PACKAGE pkg_string_conv;

PACKAGE BODY pkg_string_conv IS
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


	FUNCTION slv_to_str(slv : std_ulogic_vector) RETURN string IS
		CONSTANT len   : natural := slv'length / 4;

		VARIABLE index : natural;
		VARIABLE str   : string(len DOWNTO 1);
		VARIABLE sub   : std_ulogic_vector(3 DOWNTO 0);
	BEGIN
		ASSERT slv'length MOD 4 = 0 REPORT "slv_to_str: Length must be a multiple of 4" SEVERITY ERROR;

		index := 0;
	
		WHILE index < len LOOP
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

END PACKAGE BODY pkg_string_conv;
