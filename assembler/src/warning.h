#pragma once

#include <cstdint>
#include <string_view>

//new warning can be added by changing config.h
namespace Warning{
	enum id : uint64_t{
		immnofit     = 0x0000'0000'0000'0001,
		extradata    = 0x0000'0000'0000'0002,
		custom       = 0x0000'0000'0000'0004,
		extravals    = 0x0000'0000'0000'0008,
		warn_count
	};
	constexpr int count = __builtin_ctz(warn_count - 1) + 1;

	constexpr std::string_view name_list[count + 1] = {
		"immnofit", "extradata", "custom" , "extravals",

		"all" //handled separately
	};

	extern uint64_t enabled;
	extern uint64_t promoted;

	bool is_error(Warning::id const w);
	bool is_warn(Warning::id const w);
};
