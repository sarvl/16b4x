LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

PACKAGE p_byte IS 
	SUBTYPE t_ubyte  IS std_ulogic_vector( 7 DOWNTO 0);
	SUBTYPE t_rbyte  IS  std_logic_vector( 7 DOWNTO 0);

	SUBTYPE t_byte   IS  t_ubyte;
END PACKAGE p_byte;

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

PACKAGE p_word IS 
	SUBTYPE t_uword  IS std_ulogic_vector(15 DOWNTO 0);
	SUBTYPE t_rword  IS  std_logic_vector(15 DOWNTO 0);

	SUBTYPE t_udword IS std_ulogic_vector(31 DOWNTO 0);
	SUBTYPE t_rdword IS  std_logic_vector(31 DOWNTO 0);

	SUBTYPE t_word   IS  t_uword;
	SUBTYPE t_dword  IS t_udword;
END PACKAGE p_word;

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE work.p_byte.ALL;
USE work.p_word.ALL;

PACKAGE p_mem IS 
	TYPE t_reg IS RECORD 
		din  : t_word;
		dout : t_word;
		we   : std_ulogic;
	END RECORD;

	TYPE t_mem_rbyte IS ARRAY(natural RANGE <>) OF t_rbyte;
	TYPE t_mem_rword IS ARRAY(natural RANGE <>) OF t_rword;

	TYPE t_mem_byte IS ARRAY(natural RANGE <>) OF t_byte;
	TYPE t_mem_word IS ARRAY(natural RANGE <>) OF t_word;

	SUBTYPE t_mem IS t_mem_word;

END PACKAGE p_mem;

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

PACKAGE p_arithmetic IS 
	--convienient casts
	FUNCTION "+"(v0, v1 : std_ulogic_vector) RETURN std_ulogic_vector; 
	FUNCTION "-"(v0, v1 : std_ulogic_vector) RETURN std_ulogic_vector; 

	FUNCTION "+"(v0 : std_ulogic_vector; v1 : integer) RETURN std_ulogic_vector; 
	FUNCTION "-"(v0 : std_ulogic_vector; v1 : integer) RETURN std_ulogic_vector; 

	FUNCTION to_sint(vec : std_ulogic_vector) RETURN integer;
	FUNCTION to_uint(vec : std_ulogic_vector) RETURN integer;

END PACKAGE p_arithmetic;

PACKAGE BODY p_arithmetic IS 
	FUNCTION "+"(v0, v1 : std_ulogic_vector) RETURN std_ulogic_vector IS 
	BEGIN
		RETURN std_ulogic_vector(unsigned(v0) + unsigned(v1));
	END FUNCTION "+";
	FUNCTION "-"(v0, v1 : std_ulogic_vector) RETURN std_ulogic_vector IS 
	BEGIN
		RETURN std_ulogic_vector(unsigned(v0) - unsigned(v1));
	END FUNCTION "-";

	FUNCTION "+"(v0 : std_ulogic_vector; v1 : integer) RETURN std_ulogic_vector IS 
	BEGIN
		RETURN std_ulogic_vector(unsigned(v0) + to_unsigned(v1, v0'length));
	END FUNCTION "+";
	FUNCTION "-"(v0 : std_ulogic_vector; v1 : integer) RETURN std_ulogic_vector IS 
	BEGIN
		RETURN std_ulogic_vector(unsigned(v0) - to_unsigned(v1, v0'length));
	END FUNCTION "-";

	FUNCTION to_sint(vec : std_ulogic_vector) RETURN integer IS BEGIN
		RETURN to_integer(signed(vec));
	END FUNCTION to_sint;
	FUNCTION to_uint(vec : std_ulogic_vector) RETURN integer IS BEGIN
		RETURN to_integer(unsigned(vec));
	END FUNCTION to_uint;

END PACKAGE BODY p_arithmetic;


LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

PACKAGE p_decode IS 
	TYPE t_controls IS RECORD
		inv : std_ulogic;
		rwr : std_ulogic;
		fwr : std_ulogic;
		mwr : std_ulogic;
		mrd : std_ulogic;
		pwr : std_ulogic;
		prd : std_ulogic;
		xwr : std_ulogic;
		xrd : std_ulogic;
		pxr : std_ulogic;
		pxw : std_ulogic;
		iwr : std_ulogic;
		ird : std_ulogic;
		off : std_ulogic;
		psh : std_ulogic;
		pop : std_ulogic;
		iim : std_ulogic;
		alu : std_ulogic;
		sig : std_ulogic;
		jcc : std_ulogic;
		jmp : std_ulogic;
		cal : std_ulogic;
		ret : std_ulogic;
		int : std_ulogic;
		irt : std_ulogic;
		ien : std_ulogic;
		prv : std_ulogic;
		msw : std_ulogic;
		hlt : std_ulogic;
	END RECORD t_controls;

	SUBTYPE t_decode_vector IS std_ulogic_vector(28 DOWNTO 0);
	TYPE t_decode_rom    IS ARRAY(natural RANGE<>) OF t_decode_vector;
END PACKAGE p_decode;
