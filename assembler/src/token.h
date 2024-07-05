#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <ostream>


struct t_Token{
	enum Type{
		tex, //text
		num, //number
		reg, //register
		dir, //directive
		bcl, //bracket curly left  {
		bcr, //bracket curly right }
		col, //colon
		str, //string
		ins, //instruction
		ccc, //conditions codes
		fla, //flags
		ext  //external register
	};

	Type type;
	int val; //used to identify *whatever* particular token needs
	std::string_view file_name;
	int line_num;

	t_Token();
	t_Token(Type n_type, int const n_val, std::string_view const n_fn, int const n_ln);
};

enum class t_Instruction_Id
{
	iadd, iand, ical, icmp, idiv, ihcf,
	ihlt, iint, iirt, ijcs, ijcu, ijmp,
	imcs, imcu, imov, imrd, imro, imul,
	imwo, imwr, inot, iorr, ipop, iprd,
	ipsh, ipwr, ipxr, ipxw, iret, ishl,
	ishr, isub, itst, ixor, ixrd, ixwr,
	inop
};

	
extern std::vector<std::string> strings;

std::ostream&  operator<<(std::ostream& os, t_Token const& token);
std::ostream&  operator<<(std::ostream& os, t_Instruction_Id const instr);

void tokenize(
	std::vector<t_Token>     & output,
	std::string_view     const  file_name
	);

