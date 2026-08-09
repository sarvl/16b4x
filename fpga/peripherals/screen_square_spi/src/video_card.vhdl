LIBRARY ieee;

USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.all;

ENTITY video_card IS 
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
END ENTITY video_card;

ARCHITECTURE arch of video_card IS 
	COMPONENT screen_driver IS 
		PORT(
			clk        : IN    std_ulogic;
			nres       : IN    std_ulogic;
			
			data       : IN    std_ulogic_vector(7 DOWNTO 0);

			valid      : IN    std_ulogic;
			ready      :   OUT std_ulogic;

			screen_res :   OUT std_ulogic;
			screen_dc  :   OUT std_ulogic;
			screen_cs  :   OUT std_ulogic;
			screen_clk :   OUT std_ulogic;
			screen_din :   OUT std_ulogic);
	END COMPONENT screen_driver;

	TYPE t_ram IS ARRAY(natural RANGE<>) OF std_ulogic_vector(7 DOWNTO 0);

	SIGNAL ram : t_ram(4 * 128 - 1 DOWNTO 0) := (OTHERS => x"00");

	SIGNAL ramind_col : unsigned(6 DOWNTO 0) := (OTHERS => '1');
	SIGNAL ramind_row : unsigned(1 DOWNTO 0) := "11";

	SIGNAL sddata              : std_ulogic_vector(7 DOWNTO 0);
	SIGNAL sdready, sdreadyprv : std_ulogic;
	SIGNAL sdvalid             : std_ulogic;

BEGIN
	sd: screen_driver PORT MAP(clk        => clk,
	                           nres       => nres,
	                           data       => sddata,
							   valid      => sdvalid,
	                           ready      => sdready,
	                           screen_res => screen_res,
	                           screen_dc  => screen_dc ,
	                           screen_cs  => screen_cs ,
	                           screen_clk => screen_clk,
	                           screen_din => screen_din);

	ready <= NOT sdready;

	PROCESS(clk) IS 
		VARIABLE x_offset : integer := 0;
		VARIABLE y_offset : integer := 0;

		VARIABLE offset   : integer := 0;
	BEGIN
		IF rising_edge(clk) THEN
			IF NOT nres THEN 
				ramind_col <= (OTHERS => '1');
				ramind_row <= "11";
				sdreadyprv <= '0';
			ELSE
				sdreadyprv <= sdready;
				IF sdready AND NOT sdreadyprv THEN 
					sdvalid    <= '1';

					ramind_col <= ramind_col - 1;
					IF ramind_col - 1 = 0 THEN
						ramind_row <= ramind_row - 1;
					ELSE
						ramind_row <= ramind_row;
					END IF;

					x_offset := to_integer(ramind_col);
					y_offset := to_integer(ramind_row);

					offset   := y_offset * 128 + x_offset;

					sddata   <= ram(offset);
				ELSE
					sdvalid <= '0';
					sddata  <= UNAFFECTED;
				
					ramind_col <= ramind_col;
					ramind_row <= ramind_row;

					x_offset := to_integer(unsigned(addr( 6 DOWNTO 0)));
					y_offset := to_integer(unsigned(addr(12 DOWNTO 11)));

					offset   := y_offset * 128 + x_offset;

					IF write_en THEN
						ram(offset) <= data;
					END IF;
				END IF;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
