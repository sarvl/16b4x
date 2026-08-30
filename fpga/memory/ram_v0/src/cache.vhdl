LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

USE work.pkg_types.ALL;

ENTITY cache IS 
	GENERIC(
		g_enabled    : boolean := False;
		g_size_log_2 : integer RANGE 1 TO 15 := 1);
	PORT(
		i_clk        : IN    std_ulogic;
		i_nres       : IN    std_ulogic;

		i_addr       : IN    t_memaddr;
		io_data      : INOUT t_rword;
		i_write      : IN    std_ulogic;
		i_read       : IN    std_ulogic;
		o_hit        :   OUT std_ulogic;
		o_init       :   OUT std_ulogic);
END ENTITY cache;

ARCHITECTURE arch of cache IS 
	CONSTANT c_addr_high : integer := i_addr'high;

	TYPE t_cache_entry IS RECORD
		valid : std_ulogic;
		tag   : std_ulogic_vector(c_addr_high - g_size_log_2 DOWNTO 0);

		data  : t_word;
	END RECORD t_cache_entry;

	CONSTANT entry_empty : t_cache_entry := 
		(valid => '0', tag => (OTHERS => '0'), data => (OTHERS => '0'));

	TYPE t_ram IS ARRAY(natural RANGE<>) OF t_cache_entry;
	SIGNAL ram : t_ram(2 ** g_size_log_2 - 1 DOWNTO 0) := (OTHERS => entry_empty);

	SIGNAL cur_entry : t_cache_entry;

	SIGNAL addr_tag  : std_ulogic_vector(c_addr_high - g_size_log_2 DOWNTO 0);
	SIGNAL entry_id  : std_ulogic_vector(g_size_log_2          - 1  DOWNTO 0);
	
BEGIN
	addr_tag  <= i_addr(c_addr_high      DOWNTO g_size_log_2);
	entry_id  <= i_addr(g_size_log_2 - 1 DOWNTO            0);

	g0: IF g_enabled GENERATE 
		o_init <= '1';
		o_hit  <= (cur_entry.tag ?= addr_tag) AND cur_entry.valid;
	ELSE GENERATE
		o_init <= '1';
		o_hit  <= '0';
	END GENERATE;

	cur_entry <= ram(to_integer(unsigned(entry_id)));

	io_data   <= cur_entry.data WHEN o_hit AND i_read AND NOT i_write
	        ELSE x"ZZZZ";

	PROCESS(i_clk) IS 
	BEGIN
		IF rising_edge(i_clk) THEN
			IF i_write THEN
				ram(to_integer(unsigned(entry_id))) <= (valid => '1', tag => addr_tag, data => io_data);
			END IF;
		END IF;
	END PROCESS;

END ARCHITECTURE arch;
