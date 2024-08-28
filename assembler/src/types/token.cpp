#include "./token.h"

#include <vector>

void         strings_add(std::string const& str);
std::string& strings_get(int const ind);
std::string& strings_back();
int          strings_size();

t_Token::t_Token(){}
t_Token::t_Token(Type const n_type, int const n_val, std::string_view const n_fn, int const n_ln)
	: type(n_type), val(n_val), file_name(n_fn), line_num(n_ln) {}



std::ostream&  operator<<(std::ostream& os, t_Token const& token)
{

	using enum t_Token::Type;
	os << "[" << token.type;
	switch(token.type)
	{
		case num: case reg: case ext:
			os << " " << token.val << "]";
			break;
		case dlb: case ulb: case udf: case uvr: case mus:
		case mpr: case str:
			os << " " << strings_get(token.val) << "]"; 
			break;
		case mst: case men: case exs: case exe: case prl: 
		case prr: 
			os << "]";
			break;
		case dir: os << " " << static_cast<t_Directive::Type>(token.val)      << "]"; break;
		case ins: os << " " << static_cast<t_Instruction_Id::Type>(token.val) << "]"; break;
		case exo: case slb:
			os << " " << static_cast<char>(token.val) << "]"; 
			break;
	}

	return os;
}


#pragma GCC diagnostic ignored "-Wpedantic"
std::string to_string(t_Token::Type          const  type)
{
	return (char const* const[]) {
		"num", "reg", "dir", "str", "ins", 
		"ext", "mst", "men", "mpr", "mus", "exs", "exe",
		"exo", "dlb", "ulb", "udf", "uvr", "prl", "prr",
		"slb"
		} [type];
}
std::string to_string(t_Instruction_Id::Type const  instr)
{
	return (char const* const[]) {
		"add", "and", "ann", "cal", "cmp", "crd",
		"cwr", "dvu", "dvs", "fls", "hlt", "int",
		"irt", "jmp", "mls", "mlu", "mov", "mrd", 
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
		} [instr & 0xFF];
}
std::string to_string(t_Directive::Type      const  dir)
{
	return (char const* const[]) {
		"inc", "alg", "adr", "ddw", "rps", "rpe", "wst", 
		"wns", "wes", "wph", "wpp", "def", "ass", "isd",
		"cif", "cel", "cen", "cas", "inf", "war", "err", 
		"tof", "isa", "sam", "cei", "tst"
		} [dir];
}

std::ostream&  operator<<(std::ostream& os, t_Token::Type const type)
{
	return 
	os << (char const* const[]) {
		"num", "reg", "dir", "str", "ins", 
		"ext", "mst", "men", "mpr", "mus", "exs", "exe",
		"exo", "dlb", "ulb", "udf", "uvr", "prl", "prr",
		"slb"
		} [type];
}

std::ostream&  operator<<(std::ostream& os, t_Instruction_Id::Type const instr)
{
	return
	os << (char const* const[]) {
		"add", "and", "ann", "cal", "cmp", "crd",
		"cwr", "dvu", "dvs", "fls", "hlt", "int",
		"irt", "jmp", "mls", "mlu", "mov", "mrd", 
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
		} [instr & 0xFF];
}

std::ostream&  operator<<(std::ostream& os, t_Directive::Type const dir)
{
	return
	os << (char const* const[]) {
		"inc", "alg", "adr", "ddw", "rps", "rpe",
		"wst", "wns", "wes", "wph", "wpp", "def", "ass", 
		"isd", "cif", "cel", "cen", "cas", "inf", "war",
		"err", "tof", "isa", "sam", "cei", "tst"
		} [dir];
}
#pragma GCC diagnostic pop
