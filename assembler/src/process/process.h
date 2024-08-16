#pragma once

#include "top/types/types.h"

#include <vector>
#include <string_view>
#include <cstdint>

void tokenize(
	std::vector<t_Token>      & output,
	std::string_view     const  file_name
	);

void preprocess(
	std::vector<t_Token>       & output,
	std::vector<t_Label>       & labels,
	std::vector<t_Token>  const& tokens,
	//copy because no longer needed by caller
	std::vector<t_Define>        cmdl_defines
	);

void verify(
	std::vector<t_Token>      & output,
	std::vector<t_Token> const& tokens
	);

void code_gen(
	std::vector<uint16_t>      & output,
	std::vector<t_Token>  const& tokens,
	std::vector<t_Label>  const& labels
	);

int evaluate_const_expr(
	std::vector<t_Token>    const& tokens,
	std::vector<t_Variable> const& variables,
	std::vector<t_Define>   const& defines,
	int                          & tid
	);

int evaluate_expr(
	std::vector<t_Token>    const& tokens,
	std::vector<t_Label>    const& labels,
	int                          & tid,
	int                     const  addr = -1
	);
