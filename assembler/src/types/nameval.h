#pragma once

#include <string_view>

struct t_Nameval{
	std::string_view name;
	int value;

	t_Nameval();
	t_Nameval(std::string_view const n_name, int const n_value);
};
