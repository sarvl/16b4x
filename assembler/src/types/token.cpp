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
		case num: case reg: case ccc: case fla: case ext:
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
		"num", "reg", "dir", "str", "ins", "ccc", "fla",
		"ext", "mst", "men", "mpr", "mus", "exs", "exe",
		"exo", "dlb", "ulb", "udf", "uvr", "prl", "prr",
		"slb"
		} [type];
}
std::string to_string(t_Instruction_Id::Type const  instr)
{
	return (char const* const[]) {
		"add", "and", "cal", "cmp", "div", "hcf",
		"hlt", "int", "irt", "jcs", "jcu", "jmp",
		"mcs", "mcu", "mov", "mrd", "mro", "mul",
		"mwo", "mwr", "not", "orr", "pop", "prd",
		"psh", "pwr", "pxr", "pxw", "ret", "shl",
		"shr", "sub", "tst", "xor", "xrd", "xwr",
		"nop", "exs", "exe", "exo"
		} [instr];
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
		"num", "reg", "dir", "str", "ins", "ccc", "fla",
		"ext", "mst", "men", "mpr", "mus", "exs", "exe",
		"exo", "dlb", "ulb", "udf", "uvr", "prl", "prr",
		"slb"
		} [type];
}

std::ostream&  operator<<(std::ostream& os, t_Instruction_Id::Type const instr)
{
	return
	os << (char const* const[]) {
		"add", "and", "cal", "cmp", "div", "hcf",
		"hlt", "int", "irt", "jcs", "jcu", "jmp",
		"mcs", "mcu", "mov", "mrd", "mro", "mul",
		"mwo", "mwr", "not", "orr", "pop", "prd",
		"psh", "pwr", "pxr", "pxw", "ret", "shl",
		"shr", "sub", "tst", "xor", "xrd", "xwr",
		"nop", "exs", "exe", "exo"
		} [instr];
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



//DOES NOT WORK FOR JMP REG AND CAL REG 
//MODIFY ACCORDINGLY
int instruction_to_opcode(t_Instruction_Id::Type const iid)
{
	using namespace t_Instruction_Id;
	switch(iid)
	{
	case iadd: return 0b00011000;
	case iand: return 0b00011011;
	case ical: return 0b00000010;
	case icmp: return 0b00010001;
	case idiv: return 0b00010010;
	case ihcf: return 0b00000001011;
	case ihlt: return 0b00000001010;
	case iint: return 0b00000001000;
	case iirt: return 0b00000001001;
	case ijcs: return 0b00000101;
	case ijcu: return 0b00000100;
	case ijmp: return 0b00000011;
	case imcs: return 0b00010111;
	case imcu: return 0b00010110;
	case imov: return 0b00010100;
	case imrd: return 0b00001100;
	case imro: return 0b00001110;
	case imul: return 0b00010000;
	case imwo: return 0b00001111;
	case imwr: return 0b00001101;
	case inot: return 0b00011010;
	case iorr: return 0b00011100;
	case ipop: return 0b00010101000;
	case iprd: return 0b00001000;
	case ipsh: return 0b00010101001;
	case ipwr: return 0b00001001;
	case ipxr: return 0b00000001110;
	case ipxw: return 0b00000001111;
	case iret: return 0b00010101100;
	case inop: return 0b00010101111;
	case ishl: return 0b00011110;
	case ishr: return 0b00011111;
	case isub: return 0b00011001;
	case itst: return 0b00010011;
	case ixor: return 0b00011101;
	case ixrd: return 0b00001010;
	case ixwr: return 0b00001011;
	//should not be reachable
	default:   return -1;
	}
}

