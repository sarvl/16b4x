LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY tld IS 
	PORT(
		clk        : IN    std_ulogic;
		nres       : IN    std_ulogic;

		screen_res :   OUT std_ulogic;
		screen_dc  :   OUT std_ulogic;
		screen_cs  :   OUT std_ulogic;
		screen_clk :   OUT std_ulogic;
		screen_din :   OUT std_ulogic;

		leds       :   OUT std_ulogic_vector(5 DOWNTO 0);
		leds_outer :   OUT std_ulogic_vector(7 DOWNTO 0));
END ENTITY tld;

ARCHITECTURE arch of tld IS 
	COMPONENT video_card IS 
		PORT(
			clk        : IN    std_ulogic;
			nres       : IN    std_ulogic;

			addr       : IN    std_ulogic_vector(15 DOWNTO 0);
			data       : IN    std_ulogic_vector( 7 DOWNTO 0);
			write_en   : IN    std_ulogic;
			ready      :   OUT std_ulogic;
			
			screen_res :   OUT std_ulogic;
			screen_dc  :   OUT std_ulogic;
			screen_cs  :   OUT std_ulogic;
			screen_clk :   OUT std_ulogic;
			screen_din :   OUT std_ulogic);
	END COMPONENT video_card;

	SIGNAL addr       : unsigned(15 DOWNTO 0) := x"0000";
	SIGNAL ramind_col : unsigned(6 DOWNTO 0) := (OTHERS => '0');
	SIGNAL ramind_row : unsigned(1 DOWNTO 0) := (OTHERS => '0');
	SIGNAL offset_col : unsigned(6 DOWNTO 0) := (OTHERS => '0');
	SIGNAL offset_row : unsigned(1 DOWNTO 0) := (OTHERS => '0');

	SIGNAL data       : unsigned( 7 DOWNTO 0) := x"00";
	SIGNAL ready      : std_ulogic;

	SIGNAL cnter : unsigned(17 DOWNTO 0) := (OTHERS => '0');
	SIGNAL offset : unsigned(addr'range) := (OTHERS => '0');
BEGIN
	vc: video_card PORT MAP(clk        => clk,
	                        nres       => nres,
	                        data       => std_ulogic_vector(data),
	                        addr       => std_ulogic_vector(addr),
	                        write_en   => ready,
	                        ready      => ready,
	                        screen_res => screen_res,
	                        screen_dc  => screen_dc ,
	                        screen_cs  => screen_cs ,
	                        screen_clk => screen_clk,
	                        screen_din => screen_din);

	leds <= NOT std_ulogic_vector(addr(15 DOWNTO 11)) & NOT ready;
	leds_outer <= std_ulogic_vector(data);

	addr   <= "000" & (ramind_row + offset_row) & "000" & "0" & (ramind_col + offset_col);
	offset <= "000" & (ramind_row             ) & "000" & "0" & (ramind_col             );

	WITH offset SELECT data <=
		x"7E" WHEN x"0000",
		x"7E" WHEN x"0001",
		x"18" WHEN x"0002",
		x"18" WHEN x"0003",
		x"18" WHEN x"0004",
		x"7E" WHEN x"0005",
		x"7E" WHEN x"0006",
		x"00" WHEN x"0007",
		x"00" WHEN x"0008",
		x"42" WHEN x"0009",
		x"42" WHEN x"000A",
		x"7E" WHEN x"000B",
		x"7E" WHEN x"000C",
		x"7E" WHEN x"000D",
		x"42" WHEN x"000E",
		x"42" WHEN x"000F",
		x"00" WHEN x"0010",
		x"00" WHEN x"0011",
		x"66" WHEN x"0012",
		x"66" WHEN x"0013",
		x"81" WHEN x"0014",
		x"C3" WHEN x"0015",
		x"42" WHEN x"0016",
		x"66" WHEN x"0017",
		x"7E" WHEN x"0018",
		x"00" WHEN OTHERS;

	PROCESS(clk) IS 
	BEGIN
		IF rising_edge(clk) THEN
			IF NOT nres THEN
				cnter <= (OTHERS => '0');
				ramind_col <= (OTHERS => '0');
				ramind_row <= (OTHERS => '0');
				offset_col <= (OTHERS => '0');
				offset_row <= (OTHERS => '0');
			ELSE
				cnter <= cnter + 1;
				
				IF ready THEN
					IF ramind_col + 1 = 0 THEN
						ramind_row <= ramind_row + 1; 
					ELSE
						ramind_row <= ramind_row; 
					END IF;
					ramind_col <= ramind_col + 1;
				ELSE
					ramind_col <= UNAFFECTED;
					ramind_row <= UNAFFECTED;
				END IF;

				IF cnter = 0 THEN
					IF offset_col + 1 = 0 THEN
						offset_row <= offset_row + 1; 
					ELSE
						offset_row <= offset_row; 
					END IF;
					offset_col <= offset_col + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
