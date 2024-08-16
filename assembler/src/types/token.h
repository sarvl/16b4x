#pragma once

#include <string_view>
#include <ostream>

struct t_Token{
	enum Type{
		num, //number
		reg, //register
		dir, //directive
		str, //string
		ins, //instruction
		ccc, //conditions codes
		fla, //flags
		ext, //external register
		mst, //macro start
		men, //macro end
		mpr, //macro parameter
		mus, //macro use
		exs, //expression start
		exe, //expression end
		exo, //expression operator
		dlb, //define label
		ulb, //label use
		udf, //define use
		uvr, //variable use
		prl, //parentheses left
		prr, //parentheses right
		slb  //special label
	};


	Type type;
	int val; //used to identify *whatever* particular token needs
	std::string_view file_name;
	int line_num;

	t_Token();
	t_Token(Type const n_type, int const n_val, std::string_view const n_fn, int const n_ln);
};

namespace t_Instruction_Id {
	enum Type{
		iadd, iand, ical, icmp, idiv, ihcf,
		ihlt, iint, iirt, ijcs, ijcu, ijmp,
		imcs, imcu, imov, imrd, imro, imul,
		imwo, imwr, inot, iorr, ipop, iprd,
		ipsh, ipwr, ipxr, ipxw, iret, ishl,
		ishr, isub, itst, ixor, ixrd, ixwr,
		inop
	};
};

//DOES NOT WORK FOR JMP REG AND CAL REG 
//MODIFY ACCORDINGLY
int instruction_to_opcode(t_Instruction_Id::Type const iid);

namespace t_Directive{
	enum Type{
		inc, alg, adr, ddw, rps, rpe, wst, 
		wns, wes, wph, wpp, def, ass, isd, 
		cif, cel, cen, cas, inf, war, err, 
		tof, isa, sam, cei, tst
	};
};

std::string to_string(t_Token                const& token);
std::string to_string(t_Token::Type          const  type);
std::string to_string(t_Instruction_Id::Type const  instr);
std::string to_string(t_Directive::Type      const  dir);

std::ostream&  operator<<(std::ostream& os, t_Token                const& token);
std::ostream&  operator<<(std::ostream& os, t_Token::Type          const  type);
std::ostream&  operator<<(std::ostream& os, t_Instruction_Id::Type const  instr);
std::ostream&  operator<<(std::ostream& os, t_Directive::Type      const  dir);
