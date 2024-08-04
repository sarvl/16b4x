LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;


USE work.p_arithmetic.to_uint;
USE work.p_decode.ALL;
USE work.p_word.t_word;

ENTITY decoder IS 
	PORT(
		signals : OUT t_controls;
		rf      : OUT std_ulogic_vector( 2 DOWNTO 0);
		rs      : OUT std_ulogic_vector( 2 DOWNTO 0);
		immse   : OUT t_word;
		immze   : OUT t_word;
		jmp_flg : OUT std_ulogic_vector( 2 DOWNTO 0);
		ret_flg : OUT std_ulogic_vector( 3 DOWNTO 0);
		alu_op  : OUT std_ulogic_vector( 2 DOWNTO 0);

		instr   : IN  t_word);
END ENTITY;


ARCHITECTURE primary OF decoder IS 
		-- i r f m m p p x x p p i i o p p i a s j j c r i i i p m h
		-- n w w w r w r w r x x w r f s o i l i c m a e n r e r s l
		-- v r r r d r d r d r w r d f h p m u g c p l t t t n v w t
	CONSTANT rom_so_ri : t_decode_rom(0 TO 31) := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - inaccessible, always uses rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x01 - inaccessible, always uses rom_lo_01
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_1_0_0_0_0_0_0_0"), --0x02 - cal imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_0_0_0_0_0_0_0_0"), --0x03 - jmp imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_1_1_0_0_0_0_0_0_0_0"), --0x04 - jcu flg imm
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_1_1_1_0_0_0_0_0_0_0_0"), --0x05 - jcs flg imm
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x06 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x07 - invalid, requires extension P
		(b"0_1_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0"), --0x08 - prd d imm
		(b"0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0"), --0x09 - pwr imm s
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0A - invalid, xrd requires rom_so_rr
		(b"0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0B - xwr e imm
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0C - mrd d imm
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0D - mwr imm s
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - mro d imm
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0F - mwo imm s
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x10 - invalid, requires extention M
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x11 - cmp d imm
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x12 - invalid, requires extension D
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x13 - tst d imm
		(b"0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x14 - mov d imm
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x15 - inaccessible, always uses rom_lo_15
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x16 - invalid, requires extention C
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x17 - invalid, requires extension C
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x18 - add d imm
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x19 - sub d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1A - not d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1B - and d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1C - orr d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1D - xor d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1E - shl d imm 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0")  --0x1F - shr d imm 
	);
		-- i r f m m p p x x p p i i o p p i a s j j c r i i i p m h
		-- n w w w r w r w r x x w r f s o i l i c m a e n r e r s l 
		-- v r r r d r d r d r w r d f h p m u g c p l t t t n v w t
	CONSTANT rom_so_rr : t_decode_rom(0 TO 31) := (
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x00 - inaccessible, always uses rom_so_rr
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x01 - inaccessible, always uses rom_lo_01
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0"), --0x02 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0"), --0x03 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0"), --0x04 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_1_0_0_0_0_0_0_0_0"), --0x05 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x06 - invalid, requires extension P
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x07 - invalid, requires extension P
		(b"0_1_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0"), --0x08 - prd r r
		(b"0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0"), --0x09 - pwr r r
		(b"0_1_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0A - xrd r, e 
		(b"0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0B - xwr e r
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0C - mrd r r
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0D - mwr r r
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0E - mro r r
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x0F - mwo r r
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x10 - invalid, requires extention M
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x11 - cmp r r
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x12 - invalid, requires extension D
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x13 - tst r r
		(b"0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x14 - mov r r
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x15 - inaccessible, always uses rom_lo_15
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x16 - invalid, requires extention C
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x17 - invalid, requires extension C
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x18 - add r, r
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x19 - sub r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1A - not r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1B - and r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1C - orr r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1D - xor r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0x1E - shl r, r 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0")  --0x1F - shr r, r 
	);
		-- i r f m m p p x x p p i i o p p i a s j j c r i i i p m h
		-- n w w w r w r w r x x w r f s o i l i c m a e n r e r s l 
		-- v r r r d r d r d r w r d f h p m u g c p l t t t n v w t
	CONSTANT rom_lo_01 : t_decode_rom(0 TO 7) := (
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_1_0_0_0_0_0"), --0b000 - int i
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_1_0_0"), --0b001 - irt
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_1"), --0b010 - hlt, enable interrupts
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_1"), --0b011 - hcf, 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b100 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b101 - invalid
		(b"0_1_0_0_0_0_0_0_0_1_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0"), --0b110 - pxr d i
		(b"0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0")  --0b111 - pxw i s
	);
		-- i r f m m p p x x p p i i o p p i a s j j c r i i i p m h
		-- n w w w r w r w r x x w r f s o i l i c m a e n r e r s l 
		-- v r r r d r d r d r w r d f h p m u g c p l t t t n v w t
	CONSTANT rom_lo_15 : t_decode_rom(0 TO 7) := (
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b000 - pop r
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b001 - psh r 
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0"), --0b010 - cal r 
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0"), --0b011 - jmp r
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0"), --0b100 - ret flags
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b101 - invalid
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0b110 - invalid
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0")  --0b111 - nop
	);

	SIGNAL short_opcode_ri  : std_ulogic_vector(4 DOWNTO 0);
	SIGNAL short_opcode_rr  : std_ulogic_vector(4 DOWNTO 0);
	SIGNAL long_opcode_part : std_ulogic_vector(2 DOWNTO 0);

	SIGNAL short_opcode_ri_i  : integer RANGE 31 DOWNTO 0;
	SIGNAL short_opcode_rr_i  : integer RANGE 31 DOWNTO 0;
	SIGNAL long_opcode_part_i : integer RANGE  7 DOWNTO 0;

	SIGNAL controls_v         : t_decode_vector;

	SIGNAL prv                : std_ulogic;

--	ATTRIBUTE syn_noprune : integer;
--	ATTRIBUTE syn_noprune OF rom_lo_01 : SIGNAL IS 1;
--	ATTRIBUTE syn_noprune OF rom_lo_15 : SIGNAL IS 1;
--	ATTRIBUTE syn_noprune OF rom_so_rr : SIGNAL IS 1;
--	ATTRIBUTE syn_noprune OF rom_so_ri : SIGNAL IS 1;
--
--	ATTRIBUTE syn_keep : integer;
--	ATTRIBUTE syn_keep OF rom_lo_01 : SIGNAL IS 1;
--	ATTRIBUTE syn_keep OF rom_lo_15 : SIGNAL IS 1;
--	ATTRIBUTE syn_keep OF rom_so_rr : SIGNAL IS 1;
--	ATTRIBUTE syn_keep OF rom_so_ri : SIGNAL IS 1;

--	ATTRIBUTE syn_preserve : integer;
--	ATTRIBUTE syn_preserve OF rom_lo_01 : SIGNAL IS 1;
--	ATTRIBUTE syn_preserve OF rom_lo_15 : SIGNAL IS 1;
--	ATTRIBUTE syn_preserve OF rom_so_rr : SIGNAL IS 1;
--	ATTRIBUTE syn_preserve OF rom_so_ri : SIGNAL IS 1;

BEGIN
	short_opcode_ri  <= instr(15 DOWNTO 11);
	short_opcode_rr  <= instr( 4 DOWNTO  0);
	long_opcode_part <= instr(10 DOWNTO  8);

	short_opcode_ri_i  <= to_uint(short_opcode_ri );
	short_opcode_rr_i  <= to_uint(short_opcode_rr );
	long_opcode_part_i <= to_uint(long_opcode_part);

	controls_v <= rom_lo_01(long_opcode_part_i) WHEN short_opcode_ri = "00001"
	         ELSE rom_lo_15(long_opcode_part_i) WHEN short_opcode_ri = "10101"
	         ELSE rom_so_rr(short_opcode_rr_i)  WHEN short_opcode_ri = "00000"
	         ELSE rom_so_ri(short_opcode_ri_i);

	--if immediate top bits are 00 then immediate < 64 => int is privileged
	prv <= controls_v(1) OR (NOT immze(7) AND NOT immze(6) AND controls_v(4));
 
	signals <= (controls_v(28), controls_v(27), controls_v(26), controls_v(25), controls_v(24), 
	            controls_v(23), controls_v(22), controls_v(21), controls_v(20), controls_v(19), 
	            controls_v(18), controls_v(17), controls_v(16), controls_v(15), controls_v(14), 
	            controls_v(13), controls_v(12), controls_v(11), controls_v(10), controls_v( 9), 
	            controls_v( 8), controls_v( 7), controls_v( 6), controls_v( 5), controls_v( 4), 
	            controls_v( 3), prv           , controls_v( 1), controls_v( 0));


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
		-- i r f m m p p x x p p i i o p p i a s j j c r i i i p m h
		-- n w w w r w r w r x x w r f s o i l i c m a e n r e r s l 
		-- v r r r d r d r d r w r d f h p m u g c p l t t t n v w t
	CONSTANT rom : t_decode_rom(0 TO 31) := (
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0"), --0x00 - fin      # return from microcode
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_1_0_0_0_0_0"), --0x08 - int I    # interrupt I 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x10 -          # 
		(b"0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x18 - mov R, I # R <-- I
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x20 - mrd R, R # R <-- M[R]
		(b"0_1_0_0_1_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x28 - mrd R, I # R <-- M[I]
		(b"0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x30 - mwr R, R # M[R] <-- R
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x38 -          # 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x40 -          #
		(b"0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_1_1_1_0_0_0_0_0_0_0_0"), --0x48 - jcs CCC I# IF(CCC) IP <-- IP + I
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x50 -          #
		(b"0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x58 - xwr E, I # E <-- I
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x60 -          #
		(b"0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x68 - ird R, I # R <-- I[I]
		(b"0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x70 - iwr R, R # I[R] <-- R
		(b"0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x78 - iwr R, I # I[I] <-- R
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x80 -          # 
		(b"0_1_0_0_0_0_1_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x88 - prd R, I # R <-- P[I]
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0x90 -          # 
		(b"0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0x98 - pwr I, R # P[I] <-- R
		(b"0_1_0_0_0_0_0_0_0_1_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0_0"), --0xA0 - pxr R, I # R <-- PX[I]
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xA8 -          # 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xB0 -          # 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xB8 -          # 
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_1_0_0_0_0_0_0_0_0_0_0_0"), --0xC0 - add R, R # R <-- R + R
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0xC8 - add R, I # R <-- R + I
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xD0 -          #
		(b"0_1_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0"), --0xD8 - sub R, I # R <-- R - I
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xE0 -          # 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xE8 -          # 
		(b"1_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0"), --0xF0 -          # 
		(b"0_0_1_0_0_0_0_0_0_0_0_0_0_0_0_0_1_1_0_0_0_0_0_0_0_0_0_0_0")  --0xF8 - tst R, I #       R & I
	);

	SIGNAL controls_v : t_decode_vector;
BEGIN
	rf    <= instr(10 DOWNTO 8);
	rs    <= instr( 2 DOWNTO 0);

	immse <= (6 DOWNTO 0 => instr(6 DOWNTO 0), OTHERS => instr(7));
	immze <= (7 DOWNTO 0 => instr(7 DOWNTO 0), OTHERS => '0'     );

	jmp_flg <= instr(10 DOWNTO  8);
	alu_op <= "0" & instr(13 DOWNTO 12);
		
	ret_flg <= instr(7 DOWNTO 4); --does not matter so do the same as primary

	controls_v <= rom(to_uint(instr(15 DOWNTO 11)));

	signals <= (controls_v(28), controls_v(27), controls_v(26), controls_v(25), controls_v(24), 
	            controls_v(23), controls_v(22), controls_v(21), controls_v(20), controls_v(19), 
	            controls_v(18), controls_v(17), controls_v(16), controls_v(15), controls_v(14), 
	            controls_v(13), controls_v(12), controls_v(11), controls_v(10), controls_v( 9), 
	            controls_v( 8), controls_v( 7), controls_v( 6), controls_v( 5), controls_v( 4), 
	            controls_v( 3), controls_v( 2), controls_v( 1), controls_v( 0));

END ARCHITECTURE microcode;
