#pragma once

#include <vector>
#include <string_view>
#include <cstdint>

#include "./token.h"


struct t_Macro{
	enum t_Par_Type  : uint32_t{
		num = 0x00000001,
		reg = 0x00000002,
		str = 0x00000004,
		ins = 0x00000008,
		ccc = 0x00000010,
		fla = 0x00000020,
		ext = 0x00000040,
		exp = 0x00000080,
		mus = 0x00000100,
		ulb = 0x00000200,
		udf = 0x00000400,
		uvr = 0x00000800,

		all = 0x00000FFF
	};

	std::string_view name;
	std::vector<std::string_view> params;
	std::vector<t_Par_Type>       params_types;

	std::vector<t_Token> tokens;
};

std::ostream& operator<<(std::ostream& os, t_Macro::t_Par_Type t);


