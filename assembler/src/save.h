#pragma once

#include <vector>
#include <cstdint>

#include "top/types/label.h"

/*
 * properly handles all formats
 * does error handling
 */
int save_instructions(std::vector<uint16_t>      & instructions);
/*
 * does error handling
 */
int save_symbols     (std::vector<t_Label>  const& labels);

