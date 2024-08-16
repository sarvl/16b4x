#pragma once

#include "top/types/define.h"
#include <vector>
/*
 * returns error code when parsing failed
 * returns 0 when no error but dont continue
 * returns 1 when should continue
 */
int parse_input(int const argc, char const* const argv[], std::vector<t_Define>& defines);
