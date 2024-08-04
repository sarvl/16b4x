LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.p_arithmetic.to_uint;
USE work.p_word.t_word;
USE work.p_mem.t_mem;

ENTITY reg_file IS
	PORT(
		clk    : IN  std_ulogic;
		nreset : IN  std_ulogic;

		rd     : IN  std_ulogic_vector(2 DOWNTO 0);
		r0     : IN  std_ulogic_vector(2 DOWNTO 0);
		r1     : IN  std_ulogic_vector(2 DOWNTO 0);

		din    : IN  t_word;
		d0     : OUT t_word;
		d1     : OUT t_word;

		we     : IN  std_ulogic);
END ENTITY reg_file;


ARCHITECTURE arch OF reg_file IS
	SIGNAL rf : t_mem(7 DOWNTO 0) := (OTHERS => x"0000");
BEGIN
	d0 <= rf(to_uint(r0));
	d1 <= rf(to_uint(r1));
	
	PROCESS(ALL) IS 
	BEGIN
		IF rising_edge(clk) THEN
			IF NOT nreset THEN 
				FOR i IN 0 TO 7 LOOP
					rf(i) <= x"0000";
				END LOOP;
			ELSIF we THEN
				rf(to_uint(rd)) <= din;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
