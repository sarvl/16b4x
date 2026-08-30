LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

USE work.pkg_types.ALL;

ENTITY memory_controller IS 
	GENERIC(
		g_cache_enabled    : boolean := False;
		g_cache_size_log_2 : integer RANGE 1 TO 15 := 1);
	PORT(
		i_clk              : IN    std_ulogic;
		i_nres             : IN    std_ulogic;

		eio_mem_bus        : INOUT std_ulogic_vector(15 DOWNTO 0);
		eo_mem_addr_load   :   OUT std_ulogic;
		eo_mem_nwe         :   OUT std_ulogic;
		eo_mem_ncs         :   OUT std_ulogic;
		eo_mem_noe         :   OUT std_ulogic;

		i_addr             : IN    t_memaddr;
		io_data            : INOUT t_rword;

		i_read             : IN    std_ulogic;
		i_write            : IN    std_ulogic;
		o_ready            :   OUT std_ulogic;
		o_output           :   OUT std_ulogic);
END ENTITY memory_controller;

ARCHITECTURE arch of memory_controller IS 
	COMPONENT mem_driver_low_level IS 
		PORT(
			eio_mem_bus      : INOUT std_ulogic_vector(15 DOWNTO 0);
			eo_mem_addr_load :   OUT std_ulogic;
			eo_mem_nwe       :   OUT std_ulogic;
			eo_mem_ncs       :   OUT std_ulogic;
			eo_mem_noe       :   OUT std_ulogic;

			i_clk            : IN    std_ulogic;
			i_nres           : IN    std_ulogic;

			i_addr           : IN    t_memaddr;
			io_data          : INOUT t_word;
			i_write          : IN    std_ulogic;
			i_read           : IN    std_ulogic;
			o_ready          :   OUT std_ulogic);
	END COMPONENT mem_driver_low_level;

	COMPONENT cache IS 
		GENERIC(
			g_enabled    : boolean := False;
			g_size_log_2 : integer RANGE 1 TO 15 := 1);
		PORT(
			i_clk   : IN    std_ulogic;
			i_nres  : IN    std_ulogic;

			i_addr  : IN    t_memaddr;
			io_data : INOUT t_word;
			i_write : IN    std_ulogic;
			i_read  : IN    std_ulogic;
			o_hit   :   OUT std_ulogic;
			o_init  :   OUT std_ulogic);
	END COMPONENT cache;

	TYPE t_state IS (
		init,
		idle,
		mem_read_0,  mem_read_1,
		mem_write_0, mem_write_1, mem_write_2
		);

	SIGNAL state, state_in : t_state := init;

	SIGNAL cache_addr  : t_memaddr;
	SIGNAL cache_data  : t_rword;
	SIGNAL cache_write : std_ulogic;
	SIGNAL cache_read  : std_ulogic;
	SIGNAL cache_hit   : std_ulogic;
	SIGNAL cache_init  : std_ulogic;

	SIGNAL ll_addr     : t_memaddr;
	SIGNAL ll_data     : t_rword;
	SIGNAL ll_write    : std_ulogic;
	SIGNAL ll_read     : std_ulogic;
	SIGNAL ll_ready    : std_ulogic;

BEGIN
	u_mdll: mem_driver_low_level 
		PORT MAP(
			eio_mem_bus      => eio_mem_bus,
			eo_mem_addr_load => eo_mem_addr_load,
			eo_mem_nwe       => eo_mem_nwe, 
			eo_mem_ncs       => eo_mem_ncs,
			eo_mem_noe       => eo_mem_noe,

			i_clk            => i_clk,
			i_nres           => i_nres,

			i_addr           => ll_addr,
			io_data          => ll_data, 
			i_write          => ll_write,
			i_read           => ll_read,
			o_ready          => ll_ready);

	--write through cache only
	u_cache: cache 
		GENERIC MAP(
			g_enabled    => g_cache_enabled,
			g_size_log_2 => g_cache_size_log_2)
		PORT MAP(
			i_clk        => i_clk,
			i_nres       => i_nres,

			i_addr       => cache_addr,
			io_data      => cache_data, 
			i_write      => cache_write,
			i_read       => cache_read,
			o_hit        => cache_hit,
			o_init       => cache_init);

	cache_addr <= i_addr;
	ll_addr    <= i_addr;

	PROCESS(ALL) IS 
	BEGIN
		CASE state IS 
		when init =>
			io_data      <= x"ZZZZ";
			o_ready      <= '0';
			o_output     <= '0';

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
			IF i_write THEN
				io_data      <= x"ZZZZ";
				o_ready      <= '1';
				o_output     <= '0';

				cache_data   <= io_data;
				cache_read   <= '0';
				cache_write  <= '1';

				ll_data      <= x"ZZZZ";
				ll_read      <= '0';
				ll_write     <= '0';

				state_in     <= mem_write_0;
			-- cache read has to be single cycle 
			ELSIF i_read THEN
				IF    cache_hit THEN 
					io_data      <= cache_data;
					o_ready      <= '1';
					o_output     <= '1';

					cache_data   <= x"ZZZZ";
					cache_read   <= '1';
					cache_write  <= '0';

					ll_data      <= x"ZZZZ";
					ll_read      <= '0';
					ll_write     <= '0';

					state_in     <= idle;
				ELSE
					io_data      <= x"ZZZZ";
					o_ready      <= '1';
					o_output     <= '0';

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
				o_ready      <= '1';
				o_output     <= '0';

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
			o_ready      <= '0';
			o_output     <= '0';

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
				o_ready      <= '0';
				o_output     <= '1';

				cache_data   <= ll_data;
				cache_read   <= '0';
				cache_write  <= '1';

				ll_data      <= x"ZZZZ";
				ll_read      <= '0';
				ll_write     <= '0';

				state_in     <= idle;
			ELSE
				io_data      <= x"ZZZZ";
				o_ready      <= '0';
				o_output     <= '0';

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
			o_ready      <= '0';
			o_output     <= '0';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= x"ZZZZ";
			ll_read      <= '0';
			ll_write     <= '1';

			state_in     <= mem_write_1;

		WHEN mem_write_1 => 
			io_data      <= x"ZZZZ";
			o_ready      <= '0';
			o_output     <= '0';

			cache_data   <= x"ZZZZ";
			cache_read   <= '0';
			cache_write  <= '0';

			ll_data      <= io_data;
			ll_read      <= '0';
			ll_write     <= '0';

			state_in     <= mem_write_2;

		WHEN mem_write_2 => 
			io_data      <= x"ZZZZ";
			o_ready      <= '0';
			o_output     <= '1';

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

	PROCESS(i_clk) IS 
	BEGIN
		IF rising_edge(i_clk) THEN 
			IF NOT i_nres THEN
				state <= init;
			ELSE
				state <= state_in;
			END IF;
		END IF;
	END PROCESS;

END ARCHITECTURE arch;
