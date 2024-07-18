LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY reg_file IS
	PORT(
		clk    : IN  std_ulogic;
		nreset : IN  std_ulogic;

		rd     : IN  std_ulogic_vector(2 DOWNTO 0);
		r0     : IN  std_ulogic_vector(2 DOWNTO 0);
		r1     : IN  std_ulogic_vector(2 DOWNTO 0);

		din    : IN  std_ulogic_vector(15 DOWNTO 0);
		d0     : OUT std_ulogic_vector(15 DOWNTO 0);
		d1     : OUT std_ulogic_vector(15 DOWNTO 0);

		wen    : IN  std_ulogic);
END ENTITY reg_file;


ARCHITECTURE arch OF reg_file IS
	TYPE t_rf IS ARRAY (7 DOWNTO 0) OF std_ulogic_vector(15 DOWNTO 0);

	SIGNAL rf : t_rf := (OTHERS => x"0000");
BEGIN
	d0 <= rf(to_integer(unsigned(r0)));
	d1 <= rf(to_integer(unsigned(r1)));
	
	PROCESS(ALL) IS 
	BEGIN
		IF rising_edge(clk) THEN
			IF NOT nreset THEN 
				FOR i IN 0 TO 7 LOOP
					rf(i) <= x"0000";
				END LOOP;
			ELSIF wen THEN
				rf(to_integer(unsigned(rd))) <= din;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
