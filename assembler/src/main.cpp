#include <iostream>
#include <cstdio> //snprintf

#include <vector>
#include <string>

#include <fcntl.h> //open
#include <unistd.h> //write

#include <cstring> //strlen

#include "./../../utility/src/log.h"
#include "./token.h"
#include "./pass.h"

using namespace std::literals;

std::vector<std::string_view> filenames;
std::vector<std::string> strings;

namespace Config{
	char const* output_file = "out.bin";
};

char const* const help_string = R"(SASM PARAMETERS FILE...
Synthesizing Assembler

parameters
	-h	
		displays this help
	-lx 
		sets log level to x
		 0 - errors 
		 1 - errors, warnings
		 2 - errors, warnings, info
	-o filename
		sets output file to the filename
		defaults to "out.bin"

)";


int main(int const argc, char const* const argv[])
{
	for(int i = 1; i < argc; i++)
	{
		char const* const cur_arg = argv[i];
		if('-' != cur_arg[0])
		{
			if(filenames.size() > 0)
			{
				Log::error("more than 1 file provided");	
				return -1;
			}


			filenames.emplace_back(cur_arg);
			continue;
		}

		int const cur_arg_len = strlen(cur_arg);
		for(int j = 1; j < cur_arg_len; j++)
		{
			switch(cur_arg[j])
			{
			case 'h':
				std::cout << help_string;
				return 0;
			case 'l':
				if(cur_arg_len - 1 == j
				|| cur_arg[j + 1] > '2'
				|| cur_arg[j + 1] < '0')
				{
					Log::error("argument out of range: "s + std::string(cur_arg + j, 2)); 
					return -2;
				}

				Log::log_level = static_cast<Log::Level>(cur_arg[j + 1] - '0');
				j++;
				break;
			case 'o':
				if(argc - 1 == i)
				{
					Log::error("no output file provided");
					return -3;
				}

				Config::output_file = argv[i + 1];
				i++;
				goto arguments_continue_outer;
				break;
			default:
				Log::warning("invalid parameter: "s + std::string(cur_arg + j, 1)); 
				break;
			}
		}

	arguments_continue_outer:
		continue;
	}
	
	if(0 == filenames.size())
	{
		Log::error("no input file provided");
		return -4;
	}
	

	std::vector<t_Token> tokens;
	tokenize(tokens, filenames[0]);
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in tokenization");
		return -5;
	}

//	for(t_Token const& token : tokens)
//		std::cout << token << ' ';
//	std::cout << "\n";;

	//verifies
	//extracts labels
	std::vector<t_Label> labels;
	std::vector<t_Token> tokens_processed;
	pass_first(tokens_processed, tokens, labels);
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in verification");
		return -6;
	}

//	for(t_Token const& token : tokens_processed)
//		std::cout << token << ' ';
//	std::cout << "\n";;

	std::string result;
	pass_second(result, tokens_processed, labels);
	if(Log::is_error)
	{
		Log::info("aborting further processing due to errors in binary generation");
		return -7;
	}

	//open file for writing
	int const output_fd = open(Config::output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	if(-1 == output_fd)
	{
		Log::perror("output file could not be opened");
		return -7;
	}	
	if(-1 == write(output_fd, result.c_str(), result.size()))
	{
		Log::perror("write failed");
		close(output_fd);
		return -8;
	}
	close(output_fd);

	//dump symbols into symbols.txt
	int const symbols_fd = open("symbols.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	if(-1 == symbols_fd)
	{
		Log::perror("output file could not be opened");
		return -9;
	}	
	
	//labels should not be that big anyway
	char data[512];
	for(t_Label const& label : labels)
	{
		// text size           space+nl+null  max digits of 65536
		if(label.name.size() +       3          +  5  >= 512)
		{	
			std::string err = "Label ";
			err.append(label.name);
			err.append(" too big to be saved in text file");
			Log::error(err);
			close(symbols_fd);
			return -10;
		}
		
		int const size = std::snprintf(
			data, 512, 
			"%*s %d\n", static_cast<int>(label.name.size()), &label.name[0], label.instruction);

		if(-1 == write(symbols_fd, data, size))
		{
			Log::perror("write failed");
			close(symbols_fd);
			return -11;
		}
	}
	close(symbols_fd);

	return 0;
}

