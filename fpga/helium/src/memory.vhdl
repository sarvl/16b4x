LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY memory IS 
	PORT(
		clk   : IN    std_ulogic;
		iobus : INOUT std_logic_vector(15 DOWNTO 0);
		addr  : IN    std_ulogic_vector(15 DOWNTO 0);
		wen   : IN    std_ulogic);
END ENTITY memory;

ARCHITECTURE arch OF memory IS 
	TYPE t_ram IS ARRAY(natural RANGE <>) OF std_ulogic_vector(15 DOWNTO 0);

	--for now
	SIGNAL ram : t_ram(127 DOWNTO 0) := (
		000 => x"1866",
		001 => x"AF00",
		002 => x"AF00",
		003 => x"AF00",
		004 => x"AF00",
		005 => x"AF00",
		006 => x"AF00",
		007 => x"AF00",
		008 => x"A082",
		009 => x"689E",
		010 => x"0900",
		011 => x"AF00",
		012 => x"AF00",
		013 => x"AF00",
		014 => x"AF00",
		015 => x"AF00",
		016 => x"618E",
		017 => x"049A",
		018 => x"4880",
		019 => x"698E",
		020 => x"0900",
		021 => x"AF00",
		022 => x"AF00",
		023 => x"AF00",
		024 => x"0900",
		025 => x"AF00",
		026 => x"AF00",
		027 => x"AF00",
		028 => x"AF00",
		029 => x"AF00",
		030 => x"AF00",
		031 => x"AF00",
		032 => x"60FE",
		033 => x"98E2",
		034 => x"2FD5",
		035 => x"A0E0",
		036 => x"68FE",
		037 => x"AC00",
		038 => x"02AA",
		039 => x"17F1",
		040 => x"4824",
		041 => x"17ED",
		042 => x"4802",
		043 => x"17E9",
		044 => x"A042",
		045 => x"4846",
		046 => x"17E3",
		047 => x"A040",
		048 => x"4846",
		049 => x"17DD",
		050 => x"054B",
		051 => x"AC00",
		052 => x"A000",
		053 => x"A0C0",
		054 => x"4804",
		055 => x"4806",
		056 => x"A310",
		057 => x"A020",
		058 => x"17D7",
		059 => x"A648",
		060 => x"17C7",
		061 => x"C842",
		062 => x"2FD3",
		063 => x"A062",
		064 => x"A310",
		065 => x"17C9",
		066 => x"A054",
		067 => x"17B9",
		068 => x"C842",
		069 => x"2FD3",
		070 => x"C862",
		071 => x"2F87",
		072 => x"A018",
		073 => x"17B9",
		074 => x"A002",
		075 => x"17B5",
		076 => x"A00C",
		077 => x"17B1",
		078 => x"A022",
		079 => x"A410",
		080 => x"17AB",
		081 => x"A402",
		082 => x"17A7",
		083 => x"A412",
		084 => x"17A3",
		085 => x"A418",
		086 => x"179F",
		087 => x"A200",
		088 => x"179B",
		089 => x"A41E",
		090 => x"1797",
		091 => x"A41A",
		092 => x"1793",
		093 => x"A41C",
		094 => x"178F",
		095 => x"A412",
		096 => x"178B",
		097 => x"A506",
		098 => x"1787",
		099 => x"A506",
		100 => x"1783",
		101 => x"A412",
		102 => x"177F",
		103 => x"A402",
		104 => x"177B",
		105 => x"A410",
		106 => x"1777",
		107 => x"1FFF",
		OTHERS => x"0000");
	SIGNAL address : integer RANGE 127 DOWNTO 0;
BEGIN
	address <= to_integer(unsigned(addr(6 DOWNTO 0)));

	iobus <= ram(address) WHEN NOT wen
	    ELSE x"ZZZZ";
	
	PROCESS(ALL) IS
	BEGIN
		IF rising_edge(clk) THEN
			IF wen THEN
				ram(address) <= iobus;
			END IF;
		END IF;
	END PROCESS;
END ARCHITECTURE arch;
