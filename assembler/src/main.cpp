#include <iostream>
#include <cstdio> //snprintf

#include <vector>
#include <string>

#include <fcntl.h> //open
#include <unistd.h> //write

#include <cstring> //strlen

#include <utility> //std::move

//signals
#include <signal.h>
#include <unistd.h>

#include "top/gen_utils/log.h"
#include "top/types/token.h"
#include "top/command_line/input.h"
#include "top/config.h"
#include "top/save.h"
#include "top/process/process.h"

using namespace std::literals;

static std::vector<std::string> strings;

//necessary to use because SSO with vector increasing its size will  lead to string_view invalidation
//therefore procedure is used to reserve enough space
//strings_get and strings_size are used to not expose 'strings' 
void strings_add(std::string const& str)
{
	strings.emplace_back(str);
	strings.back().reserve(sizeof(std::string) + 1);
}

std::string& strings_get(int const ind)
{
	return strings[ind];
}

std::string& strings_back()
{
	return strings.back();
}

int strings_size()
{
	return strings.size();
}

void sig_handler(int const signo)
{
	Log::internal_error("SIGSEGV");
}


int main(int const argc, char const* const argv[])
{
	signal(SIGSEGV, sig_handler);

	std::vector<t_Define> cmdl_defines;
	{
	const int res = parse_input(argc, argv, cmdl_defines);
	if(1 != res)
		return res;
	}

	std::vector<t_Token> tokens[2];
	tokenize(tokens[0], Config::filenames[0]);
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in tokenization");
		return -5;
	}

	if(Config::Debug::to_print & Config::Debug::Print::token)
	{
		for(t_Token const& token : tokens[0])
			std::cout << token << '\n';
		Config::Debug::to_print &= ~Config::Debug::Print::token;

		if(Config::Debug::abort && Config::Debug::to_print == 0)
			return 0;
	}

	//verifies
	verify(tokens[1], tokens[0]);
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in verification");
		return -6;
	}

	if(Config::Debug::to_print & Config::Debug::Print::verify)
	{
		for(t_Token const& token : tokens[1])
			std::cout << token << '\n';
		Config::Debug::to_print &= ~Config::Debug::Print::verify;

		if(Config::Debug::abort && Config::Debug::to_print == 0)
			return 0;
	}

	tokens[0].clear();
	std::vector<t_Label> labels;
	preprocess(tokens[0], labels, tokens[1], std::move(cmdl_defines));
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in preprocessing");
		return -7;
	}

	if(Config::Debug::to_print & Config::Debug::Print::preprocess)
	{
		for(t_Token const& token : tokens[0])
			std::cout << token << '\n';
		for(t_Label const& label : labels)
			std::cout << label.name << ' ' << label.value << '\n';
		Config::Debug::to_print &= ~Config::Debug::Print::preprocess;

		if(Config::Debug::abort && Config::Debug::to_print == 0)
			return 0;
	}

	std::vector<uint16_t> result;
	code_gen(result, tokens[0], labels);
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in code gen");
		return -8;
	}

	if(0 == result.size())
	{
		Log::info("no instructions have been generated");
		return 0;
	}

	//fill labels

	{
	int const res = save_instructions(result);
	if(0 != res)
		return res;
	}

	{
	int const res = save_symbols(labels);
	if(0 != res)
		return res;
	}

	return 0;
}

