LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY alu IS 
	PORT(
		din0 : IN  std_ulogic_vector(15 DOWNTO 0);
		din1 : IN  std_ulogic_vector(15 DOWNTO 0);
		dout : OUT std_ulogic_vector(15 DOWNTO 0);

		op   : IN  std_ulogic_vector(2 DOWNTO 0);
		flags: OUT std_ulogic_vector(3 DOWNTO 0));

END ENTITY alu;


ARCHITECTURE arch OF alu IS
	--not a mistake with size
	--needed to check for a carry
	SIGNAL temp0, temp1 : unsigned(16 DOWNTO 0);
	SIGNAL res          : unsigned(16 DOWNTO 0);
	SIGNAL issub        : std_ulogic;
BEGIN
	temp0 <= unsigned("0" & din0);
	temp1 <= unsigned("0" & din1);
	issub <= op(0);	

	WITH op SELECT res <= 
		            temp0  +  temp1             WHEN "000",
		            temp0  -  temp1             WHEN "001",
		                  NOT temp1             WHEN "010",
		            temp0 AND temp1             WHEN "011",
		            temp0 OR  temp1             WHEN "100",
		            temp0 XOR temp1             WHEN "101",
		shift_left( temp0  , to_integer(temp1)) WHEN "110",
		shift_right(temp0  , to_integer(temp1)) WHEN "111",
		(OTHERS => 'X')                         WHEN OTHERS;

	dout <= std_ulogic_vector(res(15 DOWNTO 0));

	--sign
	flags(3) <= dout(15);
	--overflow
	flags(2) <= (NOT issub XOR (din0(15) XOR din1(15)))
	        AND (    issub XOR (din1(15) XOR res(15) ));
	--carry
	flags(1) <= res(16);
	--zero
	flags(0) <= dout ?= x"0000";

END ARCHITECTURE arch;
