#pragma once
#include <cstdint>

//this file contains *most* of what is needed to add or change instructions
//additionally;
//1. expand t_Instruction_Id in types/token.h
//2. add cases to switch in process/code_gen.cpp


//=== tokenization ===

constexpr static char const* const instr_strings[] = {
	"add", "and", "ann", "cal", "cmp", "crd", 
	"cwr", "dvs", "dvu", "hlt", "int", "irt", 
	"jmp", "lop", "mov", "mrd", "mlu", "mls",
	"mwr", "neg", "nop", "not", "orr", "pop", 
	"prd", "prf", "psh", "pwr", "ret", "rng", 
	"shl", "shr", "srd", "sub", "swr", "tst", 
	"xor", "xrd", "xwr",

	"jaa", "jbe", "jbz", "jcc", "jae", "jaz",
	"jge", "jgz", "jgg", "jle", "jlz", "jll",
	"jnc", "jbb", "jno", "jns", "jnz", "jne",
	"joo", "jss", "jzz", "jee", 

	"maa", "mbe", "mbz", "mcc", "mae", "maz",
	"mge", "mgz", "mgg", "mle", "mlz", "mll",
	"mnc", "mbb", "mno", "mns", "mnz", "mne",
	"moo", "mss", "mzz", "mee", 

	"saa", "sbe", "sbz", "scc", "sae", "saz",
	"sge", "sgz", "sgg", "sle", "slz", "sll",
	"snc", "sbb", "sno", "sns", "snz", "sne",
	"soo", "sss", "szz", "see"
	};


//=== verification ===
//
/* defines what each instruction has to verify
 * exp = exs ... exe
 * 'r' = reg
 * 'x' = ext
 * 'i' = num | udf | uvr | constant exp
 * 'v' = 'i' | ulb | exp
 * 'o' = 'r' | 'v'
 * 'a' = 'r' | 'i'
 * for convenience, each entry is 3 chars
 * but ' ' indicates that no more need to be parsed
 */
constexpr static char const* const pattern_table[] = {
//	add, and, ann, cal, cmp, crd, cwr, dvs,
	"ro","ro","rr","o ","ro","ra","ar","rr",
//  dvu, hlt, int, irt, jmp, lop, mlu, mls, 
	"rr","  ","i ","  ","v ","v ","ro","ro",
//  mov, mrd, mwr, neg, nop, not, orr, pop, 
	"ro","ro","or","r ","  ","r ","ro","r ",
//	prd, prf, psh, pwr, ret, rng, shl, shr, 
	"ra","r ","r ","ar","  ","r ","ro","ro",
//	srd, sub, swr, tst, xor, xrd, xwr,
	"ri","ro","ir","ro","ro","rx","xo",

//	jaa, jbe, jbz, jcc, jae, jaz, jge, jgz, 
	"v ","v ","v ","v ","v ","v ","v ","v ",
//	jgg, jle, jlz, jll, jnc, jbb, jno, jns, 
	"v ","v ","v ","v ","v ","v ","v ","v ",
//	jnz, jne, joo, jss, jzz, jee,
	"v ","v ","v ","v ","v ","v ",

//	maa, mbe, mbz, mcc, mae, maz, mge, mgz, 
	"rr","rr","rr","rr","rr","rr","rr","rr",
//	mgg, mle, mlz, mll, mnc, mbb, mno, mns, 
	"rr","rr","rr","rr","rr","rr","rr","rr",
//	mnz, mne, moo, mss, mzz, mee,
	"rr","rr","rr","rr","rr","rr",

//	saa, sbe, sbz, scc, sae, saz, sge, sgz, 
	"r ","r ","r ","r ","r ","r ","r ","r ",
//	sgg, sle, slz, sll, snc, sbb, sno, sns, 
	"r ","r ","r ","r ","r ","r ","r ","r ",
//	snz, sne, soo, sss, szz, see,
	"r ","r ","r ","r ","r ","r "
	};

//=== code gen ===

//0 indicates that particular entry is NEVER to be used
//takes care of aligning opcode properly
constexpr static uint16_t instr_imm[] = {
//	iadd,  iand,  iann,  ical,  icmp,  icrd,  icwr,  idvs,
	0xC000,0xD000,0xD800,0xA800,0x8800,0x1200,0x1300,   0  ,
//	idvu,  ihlt,  iint,  iirt,  ijmp,  ilop,  imov,  imrd, 
	   0  ,0x0104,0x1000,0x0004,0x5000,0x1F00,0xA000,0x6000,
//	imlu,  imls,   imwr,  ineg,  inop,  inot,  iorr,  ipop, 
	0x8000,0x9800,0x6800,   0  ,0x0006,   0  ,0xE000,  0   ,
//	iprd,  iprf,  ipsh,  ipwr,  iret,  irng,  ishl,  ishr,  
	0x4000,   0  ,   0  ,0x4800,0x0506,   0  ,0xF000,0xF800,
//	isrd,  isub,  iswr,  itst,  ixor,  ixrd,  ixwr,
	0x7000,0xC800,0x7800,0x9000,0xE800,   0  ,0x5800,

//	ijaa,  ijbe,  ijbz,  ijcc,  ijae,  ijaz,  ijge,  ijgz,
	0x2000,0x2100,0x2100,0x2200,0x2200,0x2200,0x2300,0x2300, 
//	ijgg,  ijle,  ijlz,  ijll,  ijnc,  ijbb,  ijno,  ijns,
	0x2400,0x2500,0x2500,0x2600,0x2700,0x2700,0x2800,0x2900, 
//	ijnz,  ijne,  ijoo,  ijss,  ijzz,  ijee,  
	0x2A00,0x2A00,0x2B00,0x2C00,0x2D00,0x2D00, 

//	imaa,  imbe,  imbz,  imcc,  imae,  imaz,  imge,  imgz,
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
//	imgg,  imle,  imlz,  imll,  imnc,  imbb,  imno,  imns,
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
//	imnz,  imne,  imoo,  imss,  imzz,  imee,  
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,

//	isaa,  isbe,  isbz,  iscc,  isae,  isaz,  isge,  isgz,
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
//	isgg,  isle,  islz,  isll,  isnc,  isbb,  isno,  isns,
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
//	isnz,  isne,  isoo,  isss,  iszz,  isee
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  
};
constexpr static uint16_t instr_reg[] = {
//	iadd,  iand,  iann,  ical,  icmp,  icrd,  icwr,  idvs,
	0x0018,0x001A,0x001B,0x0406,0x0011,0x0015,0x0016,0x0003,
//	idvu,  ihlt,  iint,  iirt,  ijmp,  ilop  ,imov,  imwr,  
	0x0002,0x0104,   0  ,0x0004,   0  ,   0  ,0x0014,0x000C,
//	imlu,  imls,  imwr,  ineg,  inop,  inot,  iorr,  ipop, 
	0x0010,0x0013,0x000D,0x0706,0x0006,0x0606,0x001C,0x0306,
//	iprd,  iprf,  ipsh,  ipwr,  iret,  irng,  ishl,  ishr, 
	0x0008,0x0007,0x0206,0x0009,0x0506,0x0106,0x001E,0x001F,
//	isrd,  isub,  iswr,  itst,  ixor,  ixrd,  ixwr,
	0x000E,0x0019,0x000F,0x0012,0x001D,0x000A,0x000B,

//	ijaa,  ijbe,  ijbz,  ijcc,  ijae,  ijaz,  ijge,  ijgz,
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
//	ijgg,  ijle,  ijlz,  ijll,  ijnc,  ijbb,  ijno,  ijns,
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
//	ijnz,  ijne,  ijoo,  ijss,  ijzz,  ijee,  
	   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,

//	imaa,  imbe,  imbz,  imcc,  imae,  imaz,  imge,  imgz,
	0xB000,0xB002,0xB002,0xB004,0xB004,0xB004,0xB006,0xB006,
//	imgg,  imle,  imlz,  imll,  imnc,  imbb,  imno,  imns,
	0xB008,0xB00A,0xB00A,0xB00C,0xB00E,0xB00E,0xB800,0xB802,
//	imnz,  imne,  imoo,  imss,  imzz,  imee,  
	0xB804,0xB804,0xB806,0xB808,0xB80A,0xB80A,

//	isaa,  isbe,  isbz,  iscc,  isae,  isaz,  isge,  isgz,
	0x0001,0x0101,0x0101,0x0201,0x0201,0x0201,0x0301,0x0301,
//	isgg,  isle,  islz,  isll,  isnc,  isbb,  isno,  isns,
	0x0401,0x0501,0x0501,0x0601,0x0701,0x0701,0x0017,0x0117,
//	isnz,  isne,  isoo,  isss,  iszz,  isee
	0x0217,0x0217,0x0317,0x0417,0x0517,0x0517
};

