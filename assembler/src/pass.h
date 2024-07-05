#pragma once

#include <vector>
#include <string>
#include "./token.h"

struct t_Label{
	std::string_view name;
	int instruction;


	t_Label();
	t_Label(std::string_view const n_name, int const n_ins);
};

void pass_first(
	std::vector<t_Token>      & output, 
	std::vector<t_Token> const& tokens, 
	std::vector<t_Label>      & labels
	);

void pass_second(
	std::string               & result,
	std::vector<t_Token> const& tokens, 
	std::vector<t_Label> const& labels
	);
	
