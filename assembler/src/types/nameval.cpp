#include "./nameval.h"

t_Nameval::t_Nameval(){}
t_Nameval::t_Nameval(std::string_view const n_name, int const n_value)
	: name(n_name), value(n_value) {}
