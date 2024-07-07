LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY timer IS 
	PORT(
		clk              : IN  std_ulogic;

		nreset           : IN  std_ulogic;

		timer_1us_passed   : OUT std_ulogic;
		timer_1ms_passed   : OUT std_ulogic);
END ENTITY timer;


ARCHITECTURE arch OF timer IS 
	SIGNAL timer_c     : unsigned(5 DOWNTO 0) := (OTHERS => '0');
	SIGNAL timer_us    : unsigned(9 DOWNTO 0) := (OTHERS => '0');
BEGIN
	
	--clock = 50MHz
	--1/(50*10^6) * (2^5 + 2^4 + 2^1) = 1/10^6 = 1us
	timer_1us_passed    <= timer_c     ?= "110010";
	--1000 = 2^9 + 2^8 + 2^7 + 2^6 + 2^5 + 2^3
	timer_1ms_passed    <= timer_us    ?= "1111101000";

	PROCESS(clk) IS
	BEGIN
		IF rising_edge(clk) THEN 
			timer_c  <= (OTHERS=>'0') WHEN NOT nreset OR timer_1us_passed
			       ELSE timer_c  + 1;

			timer_us <= (OTHERS=>'0') WHEN NOT nreset OR timer_1ms_passed
			       ELSE timer_us + 1  WHEN timer_1us_passed
				   ELSE UNAFFECTED;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
