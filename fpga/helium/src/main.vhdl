LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY main IS
	PORT(
		--52
		clk         : IN std_ulogic;
		--4
		nreset      : IN std_ulogic;
		--3
		int         : IN std_ulogic;

		--70 TO 77
		screen_data : OUT std_ulogic_vector( 7 DOWNTO  0) := "00000000";
		--68
		screen_rs   : OUT std_ulogic := '0';
		--69
		screen_en   : OUT std_ulogic := '0';

		--57
		timer_out   : OUT std_ulogic := '0';

		--16,15,14,13,11,10
		leds : OUT std_ulogic_vector(5 DOWNTO 0));

END ENTITY main;


ARCHITECTURE arch OF main IS 
	SIGNAL timer_1ms_passed : std_ulogic;

	SIGNAL mem_bus    : std_logic_vector(15 DOWNTO 0) := x"ZZZZ";
	SIGNAL mem_addr   : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL mem_write  : std_ulogic;

	SIGNAL port_bus   : std_logic_vector(15 DOWNTO 0) := x"ZZZZ";
	SIGNAL port_addr  : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL port_write : std_ulogic;


	SIGNAL interrupt  : std_ulogic;
	SIGNAL int_id     : unsigned(1 DOWNTO 0);
BEGIN
	timer:  ENTITY work.timer(arch)  PORT MAP(clk              => clk,
	                                          nreset           => nreset,
	                                          timer_1us_passed => OPEN,
	                                          timer_1ms_passed => timer_1ms_passed);
	memory: ENTITY work.memory(arch) PORT MAP(clk              => clk,
	                                          iobus            => mem_bus,
	                                          addr             => mem_addr,
	                                          wen              => mem_write);
	
	cpu:    ENTITY work.cpu(helium)  PORT MAP(clk              => clk,
	                                          nreset           => nreset,
	                                          mem_bus          => mem_bus,
	                                          mem_addr         => mem_addr,
	                                          mem_write        => mem_write,
	                                          port_bus         => port_bus,
	                                          port_addr        => port_addr,
	                                          port_write       => port_write,
	                                          ex_interrupt     => '0',
	                                          ex_int_id        => (OTHERS => '0'));
	timer_out <= timer_1ms_passed;
		

	interrupt <= timer_1ms_passed OR NOT int;
	int_id    <= "01" WHEN timer_1ms_passed
	        ELSE "10" WHEN NOT int
	        ELSE "00";

	ports: PROCESS (ALL) IS 
	BEGIN
		IF port_write THEN 
			port_bus <= x"ZZZZ";
		ELSE
			port_bus <= x"BEEF";
		END IF;

		IF rising_edge(clk) THEN 
			IF port_write THEN 
				CASE port_addr IS 
					WHEN x"0000" => leds        <= NOT port_bus(5 DOWNTO 0);
					WHEN x"0001" => screen_data <= port_bus(7 DOWNTO 0);
					WHEN x"0002" => screen_rs   <= port_bus(0);
					WHEN x"0003" => screen_en   <= port_bus(0);
					WHEN OTHERS  => NULL;
				END CASE;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
