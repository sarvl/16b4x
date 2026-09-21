LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

USE work.pkg_types.ALL;

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
	COMPONENT memory_controller IS 
		GENERIC(
			g_cache_enabled    : boolean := False;
			g_cache_size_log_2 : integer RANGE 1 TO 15 := 1);
		PORT(
			i_clk            : IN    std_ulogic;
			i_nres           : IN    std_ulogic;

			eio_mem_bus      : INOUT std_ulogic_vector(15 DOWNTO 0);
			eo_mem_addr_load :   OUT std_ulogic;
			eo_mem_nwe       :   OUT std_ulogic;
			eo_mem_ncs       :   OUT std_ulogic;
			eo_mem_noe       :   OUT std_ulogic;

			i_addr           : IN    t_memaddr;
			io_data          : INOUT t_rword;

			i_read           : IN    std_ulogic;
			i_write          : IN    std_ulogic;
			o_ready          :   OUT std_ulogic;
			o_output         :   OUT std_ulogic);
	END COMPONENT memory_controller;

	TYPE t_entry IS RECORD
		valid    : std_ulogic;
		is_write : std_ulogic;
		addr     : std_ulogic_vector(15 DOWNTO 0);
		data     : std_ulogic_vector(15 DOWNTO 0);
	END RECORD;

	TYPE t_testram IS ARRAY(natural RANGE<>) of t_entry;

	CONSTANT testdata : t_testram(0 TO 127) := (
		('1', '1', x"0000", x"A000"), ('1', '1', x"0001", x"A021"), ('1', '1', x"0002", x"A0E8"), ('1', '1', x"0003", x"88E0"), ('1', '1', x"0004", x"2505"), ('1', '1', x"0005", x"0054"),
		('1', '1', x"0006", x"0114"), ('1', '1', x"0007", x"0238"), ('1', '1', x"0008", x"C8E1"), ('1', '1', x"0009", x"24FB"), ('1', '1', x"000A", x"6A10"), ('1', '1', x"000B", x"6B04"),
		('1', '1', x"000C", x"6B25"), ('1', '1', x"000D", x"6B46"), ('1', '1', x"000E", x"6B67"), ('1', '1', x"000F", x"6B88"), ('1', '1', x"0010", x"6BA9"), ('1', '1', x"0011", x"6BCA"),
		('1', '1', x"0012", x"6BEB"), ('1', '1', x"0013", x"0104"), ('1', '0', x"0000", x"A000"), ('1', '0', x"0001", x"A021"), ('1', '0', x"0002", x"A0E8"), ('1', '0', x"0003", x"88E0"),
		('1', '0', x"0004", x"2505"), ('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"),
		('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"), ('1', '0', x"0005", x"0054"),
		('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"), ('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"),
		('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"), ('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"),
		('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"), ('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"),
		('1', '0', x"0009", x"24FB"), ('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"),
		('1', '0', x"0005", x"0054"), ('1', '0', x"0006", x"0114"), ('1', '0', x"0007", x"0238"), ('1', '0', x"0008", x"C8E1"), ('1', '0', x"0009", x"24FB"), ('1', '0', x"000A", x"6A10"),
		('1', '1', x"0050", x"0015"), ('1', '0', x"000B", x"6B04"), ('1', '1', x"0064", x"0015"), ('1', '0', x"000C", x"6B25"), ('1', '1', x"0065", x"0022"), ('1', '0', x"000D", x"6B46"),
		('1', '1', x"0066", x"000D"), ('1', '0', x"000E", x"6B67"), ('1', '1', x"0067", x"0000"), ('1', '0', x"000F", x"6B88"), ('1', '1', x"0068", x"0000"), ('1', '0', x"0010", x"6BA9"),
		('1', '1', x"0069", x"0000"), ('1', '0', x"0011", x"6BCA"), ('1', '1', x"006A", x"0000"), ('1', '0', x"0012", x"6BEB"), ('1', '1', x"006B", x"0000"), ('1', '0', x"0013", x"0104"),
		OTHERS => ('0', '0', x"0000", x"0000")
	);

	SIGNAL cnt      : unsigned(31 DOWNTO 0);
	SIGNAL clk_slow : std_ulogic := '0';
	SIGNAL ind      : integer := 0;

	SIGNAL entry : t_entry;

	SIGNAL c_addr        : t_memaddr := x"0000";
	SIGNAL c_data        : t_rword := x"0000";
	SIGNAL c_write       : std_ulogic := '0';
	SIGNAL c_read        : std_ulogic := '0';
	SIGNAL c_ready       : std_ulogic;
	SIGNAL c_output      : std_ulogic;

	SIGNAL state, state_in : std_ulogic_vector(1 DOWNTO 0);

	SIGNAL err, err_in : std_ulogic := '0';
BEGIN
	mdll: memory_controller 
		GENERIC MAP(
			g_cache_enabled    => False,
			g_cache_size_log_2 => 5)
		PORT MAP(
			eio_mem_bus      => mem_bus,
			eo_mem_addr_load => mem_addr_load,
			eo_mem_nwe       => mem_nwe, 
			eo_mem_ncs       => mem_ncs,
			eo_mem_noe       => mem_noe,

			i_clk            => clk_slow,
			i_nres           => nres,

			i_addr           => c_addr,
			io_data          => c_data, 
			i_write          => c_write,
			i_read           => c_read,
			o_ready          => c_ready,
			o_output         => c_output);

	leds       <= (NOT c_ready) & (NOT c_output) & (NOT c_read) & (NOT c_write) & (NOT clk_slow) & (NOT err);
	leds_outer <= c_addr(3 DOWNTO 0) & c_data(3 DOWNTO 0);
--	leds_outer <= std_ulogic_vector(cnt(7 DOWNTO 0));

	clk_slow <= cnt(22);

	PROCESS(clk) IS 
	BEGIN
		IF rising_edge(clk) THEN 
			IF NOT nres THEN
				cnt <= (OTHERS => '0');
			ELSE
				cnt <= cnt + 1;
			END IF;
		END IF;
	END PROCESS;

	PROCESS(clk_slow) IS 
	BEGIN
		IF rising_edge(clk_slow) THEN 
			IF NOT nres THEN
				ind <= 0;
				state <= (OTHERS => '0');
				err <= '0';
			ELSE
				IF c_output THEN
					ind <= ind + 1;
				ELSE
					ind <= UNAFFECTED;
				END IF;

				state <= state_in;
				err <= err OR err_in;
			END IF;
		END IF;
	END PROCESS;

	entry <= testdata(ind);

	c_addr <= entry.addr;
	c_data <= entry.data WHEN entry.is_write
	     ELSE x"ZZZZ";

	PROCESS(ALL) IS
	BEGIN
		CASE state IS 
		WHEN "00" => 
			c_read  <= '0';
			c_write <= '0';

			err_in <= '0';
			IF NOT entry.valid THEN
				state_in <= state;
			ELSIF c_ready THEN
				IF entry.is_write THEN
					state_in <= "11";
				ELSE
					state_in <= "10";
				END IF;
			ELSE
				state_in <= state;
			END IF;
		--read
		WHEN "10" => 
			c_read  <= '1';
			c_write <= '0';

			IF c_output THEN
				IF c_data /= entry.data THEN
					state_in <= "11";
					err_in <= '1';
				ELSE
					state_in <= "00";
					err_in <= '0';
				END IF;
			ELSE
				state_in <= state;
				err_in <= '0';
			END IF;
		--write
		WHEN "11" => 
			c_read  <= '0';
			c_write <= '1';
			err_in <= '0';

			IF c_output THEN
				state_in <= "00";
			ELSE
				state_in <= state;
			END IF;
		WHEN OTHERS =>
			c_read  <= '0';
			c_write <= '0';

			err_in <= '0';
			state_in <= (OTHERS => '1');
		END CASE;
	END PROCESS;
END ARCHITECTURE arch;
