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
		iadd, iand, iann, ical, icmp, icrd,
		icwr, idvu, idvs, ifls, ihlt, iint,
		iirt, ijmp, imls, imlu, imov, imrd, 
		imwr, ineg, inop, inot, iorr, ipop, 
		iprd, iprf, ipsh, ipwr, iret, irng, 
		ishl, ishr, isrd, isub, iswr, itst,
		ixor, ixrd, ixwr,

		ijaa, ijbe, ijbz, ijcc, ijae, ijaz,
		ijge, ijgz, ijgg, ijle, ijlz, ijll,
		ijnc, ijbb, ijno, ijns, ijnz, ijne,
		ijoo, ijss, ijzz, ijee, 

		imaa, imbe, imbz, imcc, imae, imaz,
		imge, imgz, imgg, imle, imlz, imll,
		imnc, imbb, imno, imns, imnz, imne,
		imoo, imss, imzz, imee, 

		isaa, isbe, isbz, iscc, isae, isaz,
		isge, isgz, isgg, isle, islz, isll,
		isnc, isbb, isno, isns, isnz, isne,
		isoo, isss, iszz, isee
	}; 
};

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
