LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY memory_controller IS 
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
END ENTITY memory_controller;

ARCHITECTURE arch of memory_controller IS 
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
			write            : IN    std_ulogic;
			read             : IN    std_ulogic;
			ready            :   OUT std_ulogic);
	END COMPONENT mem_driver_low_level;

	COMPONENT cache IS 
		PORT(
			clk              : IN    std_ulogic;
			nres             : IN    std_ulogic;

			address          : IN    std_ulogic_vector(15 DOWNTO 0);
			io_data          : INOUT std_ulogic_vector(15 DOWNTO 0);
			write            : IN    std_ulogic;
			read             : IN    std_ulogic;
			hit              :   OUT std_ulogic;
			init             :   OUT std_ulogic);
	END COMPONENT cache;

	TYPE t_state IS (
		init,
		idle,
		mem_read_0,  mem_read_1,
		mem_write_0, mem_write_1, mem_write_2
		);

	SIGNAL state, state_in : t_state := init;

	SIGNAL cache_addr  : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL cache_data  : std_logic_vector(15 DOWNTO 0);
	SIGNAL cache_write : std_ulogic;
	SIGNAL cache_read  : std_ulogic;
	SIGNAL cache_hit   : std_ulogic;
	SIGNAL cache_init  : std_ulogic;

	SIGNAL ll_addr     : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL ll_data     : std_logic_vector(15 DOWNTO 0);
	SIGNAL ll_write    : std_ulogic;
	SIGNAL ll_read     : std_ulogic;
	SIGNAL ll_ready    : std_ulogic;

BEGIN
	u_mdll: mem_driver_low_level PORT MAP(
			p_mem_bus       => mem_bus,
			p_mem_addr_load => mem_addr_load,
			p_mem_nwe       => mem_nwe, 
			p_mem_ncs       => mem_ncs,
			p_mem_noe       => mem_noe,

			clk             => clk,
			nres            => nres,

			address         => ll_addr,
			io_data         => ll_data, 
			write           => ll_write,
			read            => ll_read,
			ready           => ll_ready);

	--write through cache only
	u_cache: cache PORT MAP(
			clk             => clk,
			nres            => nres,

			address         => cache_addr,
			io_data         => cache_data, 
			write           => cache_write,
			read            => cache_read,
			hit             => cache_hit,
			init            => cache_init);

	cache_addr <= address;
	ll_addr    <= address;

	PROCESS(ALL) IS 
	BEGIN
		CASE state IS 
		when init =>
			io_data      <= x"ZZZZ";
			ready        <= '0';
			output       <= '0';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= x"ZZZZ";
			ll_read      <= '0';
			ll_write     <= '0';

			IF cache_init THEN
				state_in <= idle;
			ELSE
				state_in <= state;
			END IF;
		WHEN idle =>
			IF write THEN
				io_data      <= x"ZZZZ";
				ready        <= '1';
				output       <= '0';

				cache_data   <= io_data;
				cache_read   <= '0';
				cache_write  <= '1';

				ll_data      <= x"ZZZZ";
				ll_read      <= '0';
				ll_write     <= '0';

				state_in     <= mem_write_0;
			-- cache read has to be single cycle 
			ELSIF read THEN
				IF    cache_hit THEN 
					io_data      <= cache_data;
					ready        <= '1';
					output       <= '1';

					cache_data   <= x"ZZZZ";
					cache_read   <= '1';
					cache_write  <= '0';

					ll_data      <= x"ZZZZ";
					ll_read      <= '0';
					ll_write     <= '0';

					state_in     <= idle;
				ELSE
					io_data      <= x"ZZZZ";
					ready        <= '1';
					output       <= '0';

					cache_data   <= x"ZZZZ";
					cache_read   <= '0';
					cache_write  <= '0';

					ll_data      <= x"ZZZZ";
					ll_read      <= '0';
					ll_write     <= '0';

					state_in     <= mem_read_0;
				END IF;
			ELSE
				io_data      <= x"ZZZZ";
				ready        <= '1';
				output       <= '0';

				cache_data   <= x"ZZZZ";
				cache_read   <= '0';
				cache_write  <= '0';

				ll_data      <= x"ZZZZ";
				ll_read      <= '0';
				ll_write     <= '0';

				state_in     <= idle;
			END IF;

		WHEN mem_read_0 => 
			io_data      <= x"ZZZZ";
			ready        <= '0';
			output       <= '0';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= x"ZZZZ";
			ll_read      <= '1';
			ll_write     <= '0';

			state_in     <= mem_read_1;

		WHEN mem_read_1 => 
			IF ll_ready THEN
				io_data      <= ll_data;
				ready        <= '0';
				output       <= '1';

				cache_data   <= ll_data;
				cache_read   <= '0';
				cache_write  <= '1';

				ll_data      <= x"ZZZZ";
				ll_read      <= '0';
				ll_write     <= '0';

				state_in     <= idle;
			ELSE
				io_data      <= x"ZZZZ";
				ready        <= '0';
				output       <= '0';

				cache_data   <= x"ZZZZ";
				cache_read   <= '0';
				cache_write  <= '0';

				ll_data      <= x"ZZZZ";
				ll_read      <= '0';
				ll_write     <= '1';

				state_in     <= state;
			END IF;

		WHEN mem_write_0 => 
			io_data      <= x"ZZZZ";
			ready        <= '0';
			output       <= '0';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= x"ZZZZ";
			ll_read      <= '0';
			ll_write     <= '1';

			state_in     <= mem_write_1;

		WHEN mem_write_1 => 
			io_data      <= x"ZZZZ";
			ready        <= '0';
			output       <= '0';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= io_data;
			ll_read      <= '0';
			ll_write     <= '0';

			state_in     <= mem_write_2;

		WHEN mem_write_2 => 
			io_data      <= x"ZZZZ";
			ready        <= '0';
			output       <= '1';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= x"ZZZZ";
			ll_read      <= '0';
			ll_write     <= '0';

			IF ll_ready THEN
				state_in     <= idle;
			ELSE
				state_in     <= state;
			END IF;
		END CASE;
	END PROCESS;

	PROCESS(clk) IS 
	BEGIN
		IF rising_edge(clk) THEN 
			IF NOT nres THEN
				state <= init;
			ELSE
				state <= state_in;
			END IF;
		END IF;
	END PROCESS;

END ARCHITECTURE arch;
