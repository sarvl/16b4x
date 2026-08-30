LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY tld IS 
	PORT(
		clk        : IN    std_ulogic;
		nres       : IN    std_ulogic;

		mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
		mem_addr_load  :   OUT std_ulogic;
		mem_nwe        :   OUT std_ulogic;
		mem_ncs        :   OUT std_ulogic;
		mem_noe        :   OUT std_ulogic;

		leds       :   OUT std_ulogic_vector(5 DOWNTO 0);
		leds_outer :   OUT std_ulogic_vector(7 DOWNTO 0));
END ENTITY tld;

ARCHITECTURE arch of tld IS 
	COMPONENT mem_driver_low_level IS 
		PORT(
			p_mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
			p_mem_addr_load  :   OUT std_ulogic;
			p_mem_nwe        :   OUT std_ulogic;
			p_mem_ncs        :   OUT std_ulogic;
			p_mem_noe        :   OUT std_ulogic;

			clk              : IN    std_ulogic;
			nres             : IN    std_ulogic;

			address          : IN    std_ulogic_vector(15 DOWNTO 0);
			io_data          : INOUT std_ulogic_vector(15 DOWNTO 0);
			write_en         : IN    std_ulogic;
			valid            : IN    std_ulogic;
			ready            :   OUT std_ulogic);
	END COMPONENT mem_driver_low_level;

	SIGNAL cnter : unsigned(27 DOWNTO 0) := (OTHERS => '0');
	SIGNAL slow_clk : std_ulogic;

	SIGNAL state, state_in : unsigned(4 DOWNTO 0) := (OTHERS => '0');

	SIGNAL addr, data : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL we, valid, ready : std_ulogic := '0';
BEGIN
	mdll: mem_driver_low_level PORT MAP(
			p_mem_bus        => mem_bus,
			p_mem_addr_load  => mem_addr_load,
			p_mem_nwe        => mem_nwe, 
			p_mem_ncs        => mem_ncs,
			p_mem_noe        => mem_noe,

			clk              => slow_clk,
			nres             => nres,

			address          => addr,
			io_data          => data, 
			write_en         => we,
			valid            => valid,
			ready            => ready);

	leds <= NOT (slow_clk & slow_clk) & NOT mem_addr_load & mem_nwe & mem_noe & mem_ncs;
--	leds <= NOT slow_clk & NOT std_ulogic_vector(state);

	leds_outer <= mem_bus(7 DOWNTO 0);

	slow_clk <= cnter(24);

	PROCESS(clk) IS
	BEGIN
		IF rising_edge(clk) THEN 
			cnter <= cnter + 1;
		END IF;
	END PROCESS;


	PROCESS(ALL) IS 
	BEGIN
		CASE state IS 
		--initial write
		WHEN "00000" => 
			addr <= x"0001";
			data <= x"ZZZZ";

			we    <= '1';
			valid <= '1';

			IF ready THEN
				state_in <= state + 1;
			ELSE	
				state_in <= state;
			END IF;
		--cycle delayed data
		WHEN "00001" => 
			addr <= x"0001";
			data <= x"AAAA";

			we       <= '0';
			valid    <= '0';
			state_in <= state + 1;
		--wait until ready
		WHEN "00010" => 
			addr <= x"0001";
			data <= x"ZZZZ";

			we    <= '0';
			valid <= '0';

			IF ready THEN
				state_in <= state + 1;
			ELSE	
				state_in <= state;
			END IF;
		--repeat
		WHEN "00011" => 
			addr <= x"0002";
			data <= x"ZZZZ";

			we    <= '1';
			valid <= '1';

			IF ready THEN
				state_in <= state + 1;
			ELSE	
				state_in <= state;
			END IF;
		--cycle delayed data
		WHEN "00100" => 
			addr <= x"0000";
			data <= x"3333";

			we       <= '0';
			valid    <= '0';
			state_in <= state + 1;
		--wait until ready
		WHEN "00101" => 
			addr <= x"0000";
			data <= x"ZZZZ";

			we    <= '0';
			valid <= '0';

			IF ready THEN
				state_in <= state + 1;
			ELSE	
				state_in <= state;
			END IF;
		--read first data
		WHEN "00110" => 
			addr <= x"0001";
			data <= x"ZZZZ";

			we    <= '0';
			valid <= '1';

			IF ready THEN
				state_in <= state + 1;
			ELSE	
				state_in <= state;
			END IF;
		--set up next 
		WHEN "00111" => 
			addr <= x"0002";
			data <= x"ZZZZ";

			we    <= '0';
			valid <= '1';

			IF ready THEN
				state_in <= state + 1;
			ELSE	
				state_in <= state;
			END IF;
		WHEN OTHERS =>
			addr <= x"0000";
			data <= x"ZZZZ";

			we    <= '0';
			valid <= '0';
			state_in <= state;
		END CASE;
	END PROCESS;

	PROCESS(slow_clk) IS 
	BEGIN
		IF rising_edge(slow_clk) THEN 
			IF NOT nres THEN
				state <= (OTHERS => '0');
			ELSE
				state <= state_in;
			END IF;
		END IF;
	END PROCESS;

END ARCHITECTURE arch;
