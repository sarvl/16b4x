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
		twr : std_ulogic;
		trd : std_ulogic;
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
		fin : std_ulogic;
	END RECORD t_controls;
END PACKAGE p_decode;


LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
USE work.p_decode.ALL;

ENTITY decoder IS 
	PORT(
		signals : OUT t_controls;
		rf      : OUT std_ulogic_vector( 2 DOWNTO 0);
		rs      : OUT std_ulogic_vector( 2 DOWNTO 0);
		immse   : OUT std_ulogic_vector(15 DOWNTO 0);
		immze   : OUT std_ulogic_vector(15 DOWNTO 0);
		jmp_flg : OUT std_ulogic_vector( 2 DOWNTO 0);
		ret_flg : OUT std_ulogic_vector( 3 DOWNTO 0);
		alu_op  : OUT std_ulogic_vector( 2 DOWNTO 0);

		instr   : IN  std_ulogic_vector(15 DOWNTO 0);

		cycle   : IN  std_ulogic);
END ENTITY;


ARCHITECTURE primary OF decoder IS 
	TYPE t_rom IS ARRAY(natural RANGE<>) OF std_ulogic_vector(30 DOWNTO 0);
 
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_so_ri_0 : t_rom := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x00 - inaccessible, always uses rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x01 - inaccessible, always uses rom_lo_01
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_1_0_0_0_0_0_0_1"), --0x02 - cal imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_0_0_0_0_0_0_0_1"), --0x03 - jmp imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_1_1_0_0_0_0_0_0_0_1"), --0x04 - jcu flg imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_1_1_1_0_0_0_0_0_0_0_1"), --0x05 - jcs flg imm
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x06 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x07 - invalid, requires extension P
		(b"0_1_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_1"), --0x08 - prd d imm
		(b"0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_1"), --0x09 - pwr imm s
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0A - invalid, xrd requires rom_so_rr
		(b"0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0B - xwr e imm
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0C - mrd d imm
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0D - mwr imm s
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - mro d imm
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0F - mwo imm s
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x10 - invalid, requires extention M
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x11 - cmp d imm
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x12 - invalid, requires extension D
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x13 - tst d imm
		(b"0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1"), --0x14 - mov d imm
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x15 - inaccessible, always uses rom_lo_15
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x16 - invalid, requires extention C
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x17 - invalid, requires extension C
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x18 - add d imm
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x19 - sub d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1A - not d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1B - and d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1C - orr d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1D - xor d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1E - shl d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_1")  --0x1F - shr d imm 
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_so_ri_1 : t_rom := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - inaccessible, always uses rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - inaccessible, always uses rom_lo_01
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x02 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x03 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x04 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x05 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x06 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x07 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x08 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x09 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0A - invalid, xrd requires rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0B - invalid, takes only 1 cycle
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0C - mrd d imm, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0D - mwr imm s, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0E - mro d imm, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0F - mwo imm s, now NOP
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - invalid, requires extention M
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x02 - invalid, requires extension D
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x03 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x04 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x05 - inaccessible, always uses rom_lo_05
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x06 - invalid, requires extention C
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x07 - invalid, requires extension C
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x08 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x09 - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0A - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0B - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0C - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0D - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0")  --0x0F - invalid, takes only 1 cycle 
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_so_rr_0 : t_rom := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x00 - inaccessible, always uses rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x01 - inaccessible, always uses rom_lo_01
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_1"), --0x02 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_1"), --0x03 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_1"), --0x04 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_1_0_0_0_0_0_0_0_1"), --0x05 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x06 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x07 - invalid, requires extension P
		(b"0_1_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_1"), --0x08 - prd r r
		(b"0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_1"), --0x09 - pwr r r
		(b"0_1_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0A - xrd r, e 
		(b"0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0B - xwr e r
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0C - mrd r r
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0D - mwr r r
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - mro r r
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0F - mwo r r
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x10 - invalid, requires extention M
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x11 - cmp r r
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x12 - invalid, requires extension D
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x13 - tst r r
		(b"0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x14 - mov r r
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x15 - inaccessible, always uses rom_lo_15
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x16 - invalid, requires extention C
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x17 - invalid, requires extension C
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x18 - add r, r
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x19 - sub r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1A - not r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1B - and r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1C - orr r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1D - xor r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x1E - shl r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1")  --0x1F - shr r, r 
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_so_rr_1 : t_rom := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - inaccessible, always uses rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - inaccessible, always uses rom_lo_01
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x02 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x03 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x04 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x05 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x06 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x07 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x08 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x09 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0A - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0B - invalid, takes only 1 cycle
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0C - mrd d s, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x0D - mwr d s, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - mro d s, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0F - mwo d s, now NOP
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - invalid, requires extention M
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x02 - invalid, requires extension D
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x03 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x04 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x05 - inaccessible, always uses rom_lo_05
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x06 - invalid, requires extention C
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x07 - invalid, requires extension C
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x08 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x09 - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0A - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0B - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0C - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0D - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0")  --0x0F - invalid, takes only 1 cycle 
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_lo_01_0 : t_rom := (
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_1_0_0_0_0_1"), --0b000 - int i
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_1_0_1"), --0b001 - irt
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0"), --0b010 - hlt, enable interrupts and wait in next stage
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0"), --0b011 - hcf, moves to next stage and waits there forever
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b100 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b101 - invalid
		(b"0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_1"), --0b110 - pxr d i
		(b"0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_1")  --0b111 - pxw i s
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_lo_01_1 : t_rom := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b000 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b001 - invalid, takes only 1 cycle
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0"), --0b010 - hlt, waits here until interrupt
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0"), --0b011 - hcf, waits here forever
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b100 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b101 - invalid 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b110 - invalid, takes only 1 cycle
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0")  --0b111 - invalid, takes only 1 cycle
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_lo_15_0 : t_rom := (
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b000 - pop r
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b001 - psh r 
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_1"), --0b010 - cal r 
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_1"), --0b011 - jmp r
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_1"), --0b100 - ret flags
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b101 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b110 - invalid
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1")  --0b111 - nop
	);
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom_lo_15_1 : t_rom := (
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b000 - pop r, now NOP
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b001 - psh r, now NOP
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b010 - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b011 - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b100 - invalid, takes only 1 cycle 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b101 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b110 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0")  --0b111 - invalid, takes only 1 cycle 
	);

	SIGNAL short_opcode_ri  : std_ulogic_vector(4 DOWNTO 0);
	SIGNAL short_opcode_rr  : std_ulogic_vector(4 DOWNTO 0);
	SIGNAL long_opcode_part : std_ulogic_vector(2 DOWNTO 0);

	SIGNAL short_opcode_ri_i  : integer RANGE 31 DOWNTO 0;
	SIGNAL short_opcode_rr_i  : integer RANGE 31 DOWNTO 0;
	SIGNAL long_opcode_part_i : integer RANGE  7 DOWNTO 0;

	SIGNAL controls_v         : std_ulogic_vector(30 DOWNTO 0);

	SIGNAL prv                : std_ulogic;
BEGIN
	short_opcode_ri  <= instr(15 DOWNTO 11);
	short_opcode_rr  <= instr( 4 DOWNTO  0);
	long_opcode_part <= instr(10 DOWNTO  8);

	short_opcode_ri_i  <= to_integer(unsigned(short_opcode_ri ));
	short_opcode_rr_i  <= to_integer(unsigned(short_opcode_rr ));
	long_opcode_part_i <= to_integer(unsigned(long_opcode_part));

	controls_v <= rom_lo_01_0(long_opcode_part_i) WHEN short_opcode_ri = "00001" AND cycle = '0'
	         ELSE rom_lo_01_1(long_opcode_part_i) WHEN short_opcode_ri = "00001" AND cycle = '1'
	         ELSE rom_lo_15_0(long_opcode_part_i) WHEN short_opcode_ri = "10101" AND cycle = '0'
	         ELSE rom_lo_15_1(long_opcode_part_i) WHEN short_opcode_ri = "10101" AND cycle = '1'
	         ELSE rom_so_rr_0(short_opcode_rr_i)  WHEN short_opcode_ri = "00000" AND cycle = '0'
	         ELSE rom_so_rr_1(short_opcode_rr_i)  WHEN short_opcode_ri = "00000" AND cycle = '1'
	         ELSE rom_so_ri_0(short_opcode_ri_i)  WHEN cycle = '0'
	         ELSE rom_so_ri_1(short_opcode_ri_i);  --  cycle = '1'

	--if immediate top bits are 00 then immediate < 64 => int is privileged
	prv <= controls_v(1) OR (NOT immze(7) AND NOT immze(6) AND controls_v(4));
 
	signals <= (controls_v(30), controls_v(29), controls_v(28), controls_v(27), controls_v(26),
	            controls_v(25), controls_v(24), controls_v(23), controls_v(22), controls_v(21), 
	            controls_v(20), controls_v(19), controls_v(18), controls_v(17), controls_v(16),
	            controls_v(15), controls_v(14), controls_v(13), controls_v(12), controls_v(11),
	            controls_v(10), controls_v( 9), controls_v( 8), controls_v( 7), controls_v( 6),
	            controls_v( 5), controls_v( 4), controls_v( 3), prv           , controls_v( 1),
	            controls_v( 0));


	immse <= ( 6 DOWNTO 0 => instr( 7 DOWNTO 1), OTHERS => instr(0)) WHEN short_opcode_ri = "00001" AND long_opcode_part = "000" --LO IMM
	    ELSE ( 3 DOWNTO 0 => instr( 4 DOWNTO 1), OTHERS => instr(0)) WHEN short_opcode_ri = "00001" AND long_opcode_part(2 DOWNTO 1) = "11" --LO REG
	    ELSE ( 9 DOWNTO 0 => instr(10 DOWNTO 1), OTHERS => instr(0)) WHEN short_opcode_ri(4 DOWNTO 1) = "0001" --JMP I or CAL I 
	    ELSE ( 6 DOWNTO 0 => instr(10 DOWNTO 4), OTHERS => instr(0)) WHEN short_opcode_ri(4 DOWNTO 1) = "0010" --JCU   or JCS   
	    ELSE ( 6 DOWNTO 4 => instr(10 DOWNTO 8), 3 DOWNTO 0 => instr(4 DOWNTO 1), OTHERS => instr(0));

	immze <= ( 6 DOWNTO 0 => instr( 7 DOWNTO 1),  7 => instr(0), OTHERS => '0') WHEN short_opcode_ri = "00001" AND long_opcode_part = "000" --LO IMM
	    ELSE ( 3 DOWNTO 0 => instr( 4 DOWNTO 1),  4 => instr(0), OTHERS => '0') WHEN short_opcode_ri = "00001" AND long_opcode_part(2 DOWNTO 1) = "11" --LO REG
	    ELSE ( 9 DOWNTO 0 => instr(10 DOWNTO 1), 10 => instr(0), OTHERS => '0') WHEN short_opcode_ri(4 DOWNTO 1) = "0001" --JMP I or CAL I 
	    ELSE ( 6 DOWNTO 0 => instr(10 DOWNTO 4),  7 => instr(0), OTHERS => '0') WHEN short_opcode_ri(4 DOWNTO 1) = "0010" --JCU   or JCS   
	    ELSE ( 6 DOWNTO 4 => instr(10 DOWNTO 8), 3 DOWNTO 0 => instr(4 DOWNTO 1), 7 => instr(0), OTHERS => '0');


	rf      <= instr( 7 DOWNTO 5);
	rs      <= instr(10 DOWNTO 8);
	jmp_flg <= instr( 3 DOWNTO 1);
	ret_flg <= instr( 7 DOWNTO 4);

	alu_op  <= short_opcode_rr(2 DOWNTO 0) WHEN short_opcode_ri = "00000"
	      ELSE short_opcode_ri(2 DOWNTO 0);

END ARCHITECTURE primary;


ARCHITECTURE microcode OF decoder IS 
	TYPE t_rom IS ARRAY(natural RANGE<>) OF std_ulogic_vector(30 DOWNTO 0);
 
		-- i r f m m p p x x p p i i t t o p p i a s j j c r i i i p m f
		-- n w w w r w r w r x x w r w r f s o i l i c m a e n r e r s i 
		-- v r r r d r d r d r w r d r d f h p m u g c p l t t t n v w n
	CONSTANT rom : t_rom := (
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1"), --0x0 - fin, back to regular execution
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_1"), --0x1 - int imm 
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x2 - sav T R
		(b"0_1_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x3 - res R T/imm
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x4 - mrd R, R/imm
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x5 - mwr R/imm, R
		(b"0_1_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x6 - xrd R, E  
		(b"0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0x7 - xwr E, R/imm
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x8 - add R, R/imm
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0x9 - sub R, R/imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_1_0_0_0_0_0_0_0_1"), --0xA - jcs CCC R/imm
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1"), --0xB - tst R, R/imm
		(b"0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0xC - ird R, R/imm
		(b"0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0xD - iwr R/imm, R
		(b"0_1_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1"), --0xE - pxr R, imm  
		(b"0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1")  --0xF - pxw imm, R  
	);

	SIGNAL iim : std_ulogic;

	SIGNAL controls_v : std_ulogic_vector(30 DOWNTO 0);
BEGIN
	iim <= instr(11);

	rf    <= instr(10 DOWNTO 8);
	rs    <= instr( 2 DOWNTO 0);

	immse <= (6 DOWNTO 0 => instr(6 DOWNTO 0), OTHERS => instr(0));
	immze <= (7 DOWNTO 0 => instr(7 DOWNTO 0), OTHERS => '0'     );

	jmp_flg <= instr(10 DOWNTO  8);
	alu_op  <= instr(14 DOWNTO 12);
	ret_flg <= instr(7 DOWNTO 4); --does not matter so do the same as primary

	controls_v <= rom(to_integer(unsigned(instr(15 DOWNTO 12))));

	signals <= (controls_v(30), controls_v(29), controls_v(28), controls_v(27), controls_v(26),
	            controls_v(25), controls_v(24), controls_v(23), controls_v(22), controls_v(21), 
	            controls_v(20), controls_v(19), controls_v(18), controls_v(17), controls_v(16),
	            controls_v(15), controls_v(14), controls_v(13), iim           , controls_v(11),
	            controls_v(10), controls_v( 9), controls_v( 8), controls_v( 7), controls_v( 6),
	            controls_v( 5), controls_v( 4), controls_v( 3), controls_v( 2), controls_v( 1),
	            controls_v( 0));

END ARCHITECTURE microcode;
