LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;


ENTITY ghdl_stub IS 
END ENTITY ghdl_stub;

ARCHITECTURE arch OF ghdl_stub IS 
	SIGNAL clk, nreset : std_ulogic := '0';
	
	SIGNAL count : unsigned(7 DOWNTO 0) := x"00";
BEGIN
	stub: ENTITY work.main(arch) PORT MAP (clk         => clk,  nreset    => nreset, 
	                                       screen_data => OPEN, screen_rs => OPEN,
	                                       screen_en   => OPEN, leds      => OPEN,
	                                       int => count ?/= x"FF");


	nreset <= '1';



	PROCESS(ALL) IS
	BEGIN
		IF rising_edge(clk) THEN
			IF count = x"FF" THEN
				count <= x"00";
			ELSE
				count <= count + 1;
			END IF;
		END IF;
	END PROCESS;
	PROCESS IS 
	BEGIN
		clk <= '0';
		WAIT FOR 1 NS;
		clk <= '1';
		WAIT FOR 1 NS;
	END PROCESS;


END ARCHITECTURE arch;
