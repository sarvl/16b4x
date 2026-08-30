LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY simram IS
	PORT(
		mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
		mem_addr_load  : IN    std_ulogic;
		mem_nwe        : IN    std_ulogic;
		mem_ncs        : IN    std_ulogic;
		mem_noe        : IN    std_ulogic);
END ENTITY simram;


ARCHITECTURE arch OF simram IS
	TYPE t_ram IS ARRAY(natural RANGE<>) OF std_ulogic_vector(15 DOWNTO 0);

	SIGNAL ram : t_ram(2**16 - 1 DOWNTO 0) := (OTHERS => x"0000");

	SIGNAL addr : std_ulogic_vector(15 DOWNTO 0);
BEGIN
	addr <= mem_bus WHEN mem_addr_load
	   ELSE addr;

	mem_bus <= ram(to_integer(unsigned(addr))) WHEN NOT mem_ncs AND NOT mem_noe
	      ELSE x"ZZZZ";

	PROCESS(ALL) IS
	BEGIN
		IF falling_edge(mem_nwe) THEN
			IF NOT mem_ncs THEN
				ram(to_integer(unsigned(addr))) <= mem_bus;
			END IF;
		END IF;
	END PROCESS;

END ARCHITECTURE arch;
