LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY cache IS 
	PORT(
		clk              : IN    std_ulogic;
		nres             : IN    std_ulogic;

		address          : IN    std_ulogic_vector(15 DOWNTO 0);
		io_data          : INOUT std_ulogic_vector(15 DOWNTO 0);
		write            : IN    std_ulogic;
		read             : IN    std_ulogic;
		hit              :   OUT std_ulogic;
		init             :   OUT std_ulogic);
END ENTITY cache;

ARCHITECTURE arch of cache IS 
	CONSTANT size_log_2 : integer := 5;

	TYPE t_cache_entry IS RECORD
		valid : std_ulogic;
		tag   : std_ulogic_vector(15 - size_log_2 DOWNTO 0);

		data  : std_ulogic_vector(15 DOWNTO 0);
	END RECORD t_cache_entry;

	CONSTANT entry_empty : t_cache_entry := (valid => '0', tag => (OTHERS => '0'), data => (OTHERS => '0'));

	TYPE t_ram IS ARRAY(natural RANGE<>) OF t_cache_entry;
	SIGNAL ram : t_ram(2 ** size_log_2 - 1 DOWNTO 0) := (OTHERS => entry_empty);

	SIGNAL cur_entry : t_cache_entry;

	SIGNAL addr_tag  : std_ulogic_vector(15 - size_log_2 DOWNTO 0);
	SIGNAL entry_id  : std_ulogic_vector(size_log_2 - 1  DOWNTO 0);
	
BEGIN
	addr_tag  <= address(            15 DOWNTO size_log_2);
	entry_id  <= address(size_log_2 - 1 DOWNTO          0);

	cur_entry <= ram(to_integer(unsigned(entry_id)));
	hit       <= (cur_entry.tag ?= addr_tag) AND cur_entry.valid;

	io_data   <= cur_entry.data WHEN hit AND read AND NOT write
	        ELSE x"ZZZZ";

	init <= '1';

	PROCESS(clk) IS 
	BEGIN
		IF rising_edge(clk) THEN
			IF write THEN
				ram(to_integer(unsigned(entry_id))) <= (valid => '1', tag => addr_tag, data => io_data);
			END IF;
		END IF;
	END PROCESS;

END ARCHITECTURE arch;
