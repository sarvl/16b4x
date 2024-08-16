#include "./input.h"

#include <iostream>

#include <cstring> //strlen, strcmp

#include "top/gen_utils/log.h"
#include "top/warning.h"
#include "top/config.h"

using namespace std::literals;

extern char const* const string_help;
extern char const* const string_help_warn;
extern char const* const string_help_format;
extern char const* const string_help_directives;
extern char const* const string_help_expressions;
extern char const* const string_help_labels;
extern char const* const string_help_macros;

int parse_input(int const argc, char const* const argv[], std::vector<t_Define>& defines)
{
	for(int i = 1; i < argc; i++)
	{
		char const* const cur_arg = argv[i];
		if('-' != cur_arg[0])
		{
			if(Config::filenames.size() > 0)
			{
				Log::error("more than 1 file provided");	
				return -1;
			}


			Config::filenames.emplace_back(cur_arg);
			continue;
		}

		int const cur_arg_len = strlen(cur_arg);

		for(int j = 1; j < cur_arg_len; j++)
		{
			switch(cur_arg[j])
			{
			case 'h':
				if(cur_arg_len - 1 == j)
				{
					std::cout << string_help;
					return 0;
				}

				switch(cur_arg[j + 1])
				{
				case 'W':
					std::cout << string_help_warn;
					return 0;
				case 'f':
					std::cout << string_help_format;
					return 0;
				case 'd':
					std::cout << string_help_directives;
					return 0;
				case 'e':
					std::cout << string_help_expressions;
					return 0;
				case 'L':
					std::cout << string_help_labels;
					return 0;
				case 'M':
					std::cout << string_help_macros;
					return 0;
				default:
				{
					std::string err = "invalid argument to -h: X";
					err.back() = cur_arg[j + 1];
					Log::error(err); 
					return -2;
				}
				}

				break;
			case 'a':
				Log::error_abort = true;
				break;
			case 'd':
			{
				if(cur_arg_len - 1 == j)
				{
					Log::error("lack of argument to -d");
					return -2;
				}
				j++;

				int const name_beg = j;
				//find '='
				while( j   < cur_arg_len
				   && '=' != cur_arg[j])
					j++;

				int const name_pend = j;
				if(j == cur_arg_len)
				{
					defines.emplace_back(std::string_view(cur_arg + name_beg, name_pend - name_beg), 1);
					break;
				}
				int num = 0;
				j++;
				while(j < cur_arg_len)
				{
					if(cur_arg[j] < '0' || '9' < cur_arg[j])
					{
						Log::error("non digit character in -d VAL");
						return -2;
					}

					num = num * 10 + (cur_arg[j] - '0');
					j++;
				}

				defines.emplace_back(std::string_view(cur_arg + name_beg, name_pend - name_beg), num);
				break;
			}
			case 'l':
				if(cur_arg_len - 1 == j)
				{
					Log::error("lack of argument to -l");
					return -2;
				}
				
				if(cur_arg[j + 1] == 'e')
					Log::log_level = Log::Level::error;
				else if(cur_arg[j + 1] == 'w')
					Log::log_level = Log::Level::warning;
				else if(cur_arg[j + 1] == 'i')
					Log::log_level = Log::Level::info;
				else
				{
					std::string err = "invalid argument to -l: X";
					err.back() = cur_arg[j + 1];
					Log::error(err); 
					return -2;
				}

				j++;
				break;
			case 'f':
			{
				std::string_view format;
				//argument is in next
				if(cur_arg_len - 1 == j)
				{
					if(argc - 1 == i)
					{
						Log::error("no format provided");
						return -3;
					}
					
					format = argv[i + 1];
					i++;
				}
				else
				{
					if('h' == cur_arg[j + 1])
					{
						std::cout << string_help_format;
						return 0;
					}

					format = cur_arg + j + 1;
				}
				
				for(int k = 0; k < Config::Format::count; k++)
				{
					std::string_view const& cur_fmt = Config::Format::name_list[k];
					if(0 == strncmp(&format[0], 
					                &cur_fmt[0],
					                 cur_fmt.size()
					               ))
					{
						Config::Format::format = Config::Format::index_to_fmt(k);	
						goto arguments_continue_outer;
					}
				}
				std::string err = "invalid format name "s + std::string(format);
				Log::error(err);
				return -3;
			}
			case 'o':
				if(argc - 1 == i)
				{
					Log::error("no output file provided");
					return -3;
				}

				Config::file_output = argv[i + 1];
				i++;
				goto arguments_continue_outer;
				break;
			case 'W':
			{
				if(cur_arg_len - 1 == j)
				{
					Log::error("lack of argument to -W");
					return -2;
				}

				int mode = 0;
				uint64_t warns = 0;

				if('h'  == cur_arg[j + 1])
				{
					std::cout << string_help_warn;
					return 0;
				}
				//warn turn off
				if('N' == cur_arg[j + 1])
				{
					if(cur_arg_len == j)
					{
						Log::error("lack of argument to -WN");
						return -2;
					}

					mode = 1;
					j++;
				}
				//warn promote
				if('E' == cur_arg[j + 1])
				{
					if(cur_arg_len == j)
					{
						Log::error("lack of argument to -WE");
						return -2;
					}
					
					mode = 2;
					j++;
				}

				j++;

				while(j < cur_arg_len)
				{
					for(int k = 0; k <= Warning::count; k++)
					{
						std::string_view const& cur_warn = Warning::name_list[k];
						if(0 == strncmp( cur_arg + j, 
						                &cur_warn[0],
						                 cur_warn.size()
						               ))
						{
							j += cur_warn.size();
							if(k == Warning::count)
								warns = ~(0ULL); // all
							else
								warns |= 1ULL << k;

							goto warn_next_arg;
						}
					}
					{
					std::string err = "invalid warning name "s + std::string(cur_arg + j);
					Log::error(err);
					return -3;
					}
				warn_next_arg:
					if(j == cur_arg_len)
						break;
					//stray comma is ok
					if(',' != cur_arg[j])
					{
						Log::error("warnings not separated by comma");
						return -3;
					}
					j++;
				}

				     if(0 == mode)
					Warning::enabled   |=  warns;
				else if(1 == mode)
				{
					Warning::enabled   &= ~warns;
					Warning::promoted &= ~warns;
				}
				else if(2 == mode)
					Warning::promoted |=  warns & Warning::enabled;

				break;
			}
			case 'D':
				if(cur_arg_len - 1 == j)
				{
					j++;
					break;
				}
				switch(cur_arg[j + 1])
				{
				case 'T':
					 Config::Debug::abort = true;
				[[fallthrough]];
				case 't':
					 Config::Debug::to_print |= Config::Debug::token;
					 break;
				case 'V':
					 Config::Debug::abort = true;
				[[fallthrough]];
				case 'v':
					 Config::Debug::to_print |= Config::Debug::verify;
					 break;
				case 'P':
					 Config::Debug::abort = true;
				[[fallthrough]];
				case 'p':
					 Config::Debug::to_print |= Config::Debug::preprocess;
					 break;
				default:
					break;
				}


				j += 2;
				break;
			default:
				Log::warning("invalid parameter: "s + std::string(cur_arg + j, 1)); 
				break;
			}
		}

	arguments_continue_outer:
		continue;
	}
	
	if(0 == Config::filenames.size())
	{
		Log::error("no input file provided");
		return -4;
	}

	return 1;
}
