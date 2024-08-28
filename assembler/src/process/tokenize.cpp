#include "./process.h"

#include <fcntl.h> //open, close
#include <sys/mman.h> //mmap, munmap
#include <unistd.h> //lseek

#include <cstring> //strncmp

#include "top/gen_utils/log.h"
#include "top/gen_utils/file.h"
#include "top/types/token.h"
#include "top/utils.h"


using namespace std::literals;

void         strings_add(std::string const& str);
std::string& strings_get(int const ind);
std::string& strings_back();
int          strings_size();

void tokenize(
	std::vector<t_Token>     & output,
	std::string_view    const  file_name
	)
{
	Log::is_error = false;
	Log::is_warning = false;


	File::t_File file;
	if(-1 == File::create_error_handled(&file, file_name))
		return;
	char const* const data = file.data;
	int  const        size = file.size;

	int line_num = 1;
	int off = 0;
	bool negative = false;

	int begin = 0;
	int line  = 0;

	bool in_macro      = false;
	bool in_string     = false;
	bool in_comment    = false;
	bool in_expression = false;

	std::vector<char> modes_stack;
	std::vector<int>  expressions_started, arg_lists_started;
	
	while(off < size)
	{
		if(in_string)
		{
			if(off + 1 <= size
			&& '\\' != data[off   ]
			&& '"'  == data[off + 1])
			{
				output.emplace_back(t_Token::str, strings_size(), file_name, line);
				//bounds are correct
				strings_add(std::string(data + begin, off + 1 - begin));

				off += 2;
				in_string = false;
				continue;
			}

			if('\n' == data[off])
				line_num++;
			off++;
			continue;
		}
	
		if(in_comment)
		{
			if(off + 1 < size
			&& '*' == data[off]
			&& '/' == data[off + 1])
			{
				
				off += 2;
				in_comment = false;
				continue;
			}
		
			if('\n' == data[off])
				line_num++;
			off++;
			continue;
		}

		switch(data[off])
		{
		case '+': case '*': case '&': case '|': case '^': 
		case '~': case '?': case '!': case '=': case '<':
		case '>': 
			if(!in_expression)
				goto process_stray_character;

			if(off < size - 1
			&& !Utils::is_whitespace(data[off + 1])
			&& '}' != data[off + 1])
			{
				Log::error("operator must be separated from other tokens", file_name, line_num);
				off++;
				break;
			}
			output.emplace_back(t_Token::exo, data[off], file_name, line_num);
			off++;

			break;
		case '/':
			if(in_expression)
			{
				output.emplace_back(t_Token::exo, '/', file_name, line_num);
				off++;
				break;
			}
			if(off + 1 == size)
				goto process_stray_character;
			if('*' == data[off + 1])
			{
				line = line_num;
				in_comment = true;
				break;
			}
			if('/' != data[off + 1])
				goto process_stray_character;

		[[fallthrough]];
		case ';':
			while(off   < size
			   && '\n' != data[off])
			   off++;
			break;

		case '"':
			in_string = true;
			line      = line_num;
			begin     = off + 1;
			off++;
			break;
		case '(':
			if(0   == output.size()
			|| t_Token::mus != output.back().type)
				Log::error("'(' must follow usage of macro", file_name, line_num);

			output.emplace_back(t_Token::prl, 0, file_name, line_num);
			modes_stack.emplace_back(')');
			arg_lists_started.emplace_back(line_num);
			off++;
			in_expression = false;
			break;
		case ')':
			if(0 == arg_lists_started.size())
			{
				Log::error("unexpected macro arg list end ')', no arg list to end", file_name, line_num);
				off++;
				break;
			}

			output.emplace_back(t_Token::prr, 0, file_name, line_num);
			if(')' != modes_stack.back())
			{
				Log::error("unexpected ')', expected '}'", file_name, line_num);
				Log::info("^ that should match '{' at :", file_name, expressions_started.back());
			}
			else
			{
				arg_lists_started.pop_back();
				modes_stack.pop_back();
			}
			in_expression = expressions_started.size() != 0 && '}' == modes_stack.back();
			off++;
			break;
		case '{':
			output.emplace_back(t_Token::exs, 0, file_name, line_num);
			in_expression = true;
			expressions_started.emplace_back(line_num);
			modes_stack.emplace_back('}');
			off++;
			break;
		case '}':
			if(expressions_started.size() > 0)
			{
				output.emplace_back(t_Token::exe, 0, file_name, line_num);

				if('}' != modes_stack.back())
				{
					Log::error("unexpected '}', expected ')'", file_name, line_num);
					Log::info("^ that should match '(' at :", file_name, arg_lists_started.back());
				}
				else
				{
					modes_stack.pop_back();
					expressions_started.pop_back();
				}
				in_expression = expressions_started.size() != 0 && '}' == modes_stack.back();
			}
			else
				Log::error("expression terminator outside of expression", file_name, line_num);
			off++;
			break;
		case ':':
			if(0 == output.size()
			|| t_Token::ulb != output.back().type)
			{
				Log::error("':' must follow label", file_name, line_num);
				off++;
				break;
			}

			output.back().type = t_Token::dlb;
			off++;

			break;
		case '.':
			if(in_expression)
			{
				output.emplace_back(t_Token::exo, '.', file_name, line_num);
				off++;
				break;
			}

			if(off == size - 1)
				goto process_stray_character;

			if('.' == data[off + 1]
			|| ',' == data[off + 1]
			|| '>' == data[off + 1]
			|| '<' == data[off + 1])
			{
				output.emplace_back(t_Token::slb, data[off + 1], file_name, line_num);
				off += 2;
				break;
			}

			if(!Utils::is_id_char(data[off + 1]))
				goto process_stray_character;
			
			goto process_string;
			
		case ',':
			if(in_expression)
				output.emplace_back(t_Token::exo, ',', file_name, line_num);
			off++;
			break;
		case '\n':
			line_num++;
		[[fallthrough]];
		case ' ':
		case '\t':
			off++;
			break;

		case 'R':
			//check whether the last char 
			if(off == size - 1)
				goto process_string;

			//check whether next is a valid number
			if(data[off + 1] < '0'
			|| data[off + 1] > '9')
				goto process_string;

			if('8' == data[off + 1]
			|| '9' == data[off + 1])
			{
				Log::error("invalid register provided", file_name, line_num);
				off += 2;
				break;
			}

			//check whether some separator after that 
			if(off == size - 2
			|| !Utils::is_id_char(data[off + 2]))
			{
				output.emplace_back(t_Token::reg, data[off + 1] - '0', file_name, line_num);
				off += 2;
				break;
			}
			goto process_string;

		case '-':
			if(off == size - 1
			|| !Utils::is_digit(data[off + 1]))
			{
				if(in_expression)
				{
					output.emplace_back(t_Token::exo, '-', file_name, line_num);
					off++;
					break;
				}
				goto process_stray_character;
			}
			negative = true;

			off++;
		[[fallthrough]];
		#pragma GCC diagnostic ignored "-Wpedantic"
		case '0'...'9':
		#pragma GCC diagnostic pop
		{
			int num = 0;
			//check for hex
			if(off < size - 1
			&& data[off + 1] == 'x')
			{
				off += 2;

				while(off < size)
				{
					     if(data[off] >= '0' && data[off] <= '9')
						num = (num << 4) + (data[off] - '0');
					else if(data[off] >= 'a' && data[off] <= 'f')
						num = (num << 4) + (data[off] - 'a' + 10);
					else if(data[off] >= 'A' && data[off] <= 'F')
						num = (num << 4) + (data[off] - 'A' + 10);
					else
						goto num_end_process;;

					off++;
				}
				goto num_end_process;
			}
			//check for binary
			if(off < size - 1
			&& data[off + 1] == 'b')
			{
				off += 2;

				while(off < size)
				{
					if(Utils::is_bit(data[off]))
						num = (num << 1) + (data[off] - '0');
					else
						goto num_end_process;;

					off++;
				}
				goto num_end_process;
			}
			//decimal
			while(off < size)
			{
				 if(Utils::is_digit(data[off]))
					num = (num * 10) + (data[off] - '0');
				else
					goto num_end_process;;

				off++;
			}
			

		num_end_process:
			if(negative)
				num = -num;
				
			output.emplace_back(t_Token::num, num, file_name, line_num);
			negative = false;
			break;

		}
		case '%':
		{
			using namespace t_Directive;
			int const beg = off + 1;
			off++;
			while(off < size && Utils::is_id_char(data[off]))
				off++;

			if(0 == off - beg)
			{
				if(in_expression)
					output.emplace_back(t_Token::exo, '%', file_name, line_num);
				else
					Log::error("stray directive symbol '%'", file_name, line_num);
				break;
			}

			constexpr static char const*             arr_2_c[] = 
				{"dw", "ws", "if"};
			constexpr static t_Directive::Type const arr_2_d[] =
				{ddw, wst, cif};

			constexpr static char const*             arr_3_c[] = 
				{"inc", "adr", "end", "wes", "wns", "rpe",
				 "rps"};
			constexpr static t_Directive::Type const arr_3_d[] = 
				{inc, adr, cen, wes, wns, rpe,
				 rps };
			
			constexpr static char const*             arr_4_c[] = 
				{"wpsh", "wpop", "warn", "else", "info", "same"};
			constexpr static t_Directive::Type const arr_4_d[] = 
				{wph, wpp, war, cel, inf, sam};

			constexpr static char const*             arr_5_c[] = 
				{"align", "error", "elsif"};
			constexpr static t_Directive::Type const arr_5_d[] = 
				{alg, err, cei};


			constexpr static char const*             arr_6_c[] = 
				{"define", "assert", "assign", "typeof"};
			constexpr static t_Directive::Type const arr_6_d[] = 
				{def, cas, ass, tof};

			constexpr static char const*             arr_8_c[] = 
				{"tostring"};
			constexpr static t_Directive::Type const arr_8_d[] = 
				{tst};

			constexpr static char const*             arr_9_c[] = 
				{"isdefined"};
			constexpr static t_Directive::Type const arr_9_d[] = 
				{isd};

			constexpr static char const*             arr_10_c[] = 
				{"isassigned"};
			constexpr static t_Directive::Type const arr_10_d[] = 
				{isa};

			constexpr static struct{char const* const* arr_c; t_Directive::Type const* arr_d; int const size;} Pairs[] = {
				{nullptr , nullptr ,                                    0}, {nullptr, nullptr,                                  0},
				{arr_2_c , arr_2_d , sizeof( arr_2_c)/sizeof( arr_2_c[0])}, {arr_3_c, arr_3_d, sizeof(arr_3_c)/sizeof(arr_3_c[0])}, 
				{arr_4_c , arr_4_d , sizeof( arr_4_c)/sizeof( arr_2_c[0])}, {arr_5_c, arr_5_d, sizeof(arr_5_c)/sizeof(arr_5_c[0])}, 
				{arr_6_c , arr_6_d , sizeof( arr_6_c)/sizeof( arr_2_c[0])}, {nullptr, nullptr,                                  0}, 
				{arr_8_c , arr_8_d , sizeof( arr_8_c)/sizeof( arr_8_c[0])}, {arr_9_c, arr_9_d, sizeof(arr_9_c)/sizeof(arr_9_c[0])},
				{arr_10_c, arr_10_d, sizeof(arr_10_c)/sizeof(arr_10_c[0])}, {nullptr, nullptr,                                  0}
				};
		
			int const str_len = off - beg;
			if((2 <= str_len && str_len <=  6) 
			||  9 <= str_len || str_len <= 10)
			{
				auto const arr_c = Pairs[str_len].arr_c;
				auto const arr_d = Pairs[str_len].arr_d;
				auto const size  = Pairs[str_len].size;

				for(int i = 0; i < size; i++)
				{
					if(0 == strncmp(data + beg, arr_c[i], str_len))
					{
						output.emplace_back(t_Token::dir, arr_d[i], file_name, line_num);
						goto next_char;
					}
				}
			}

			std::string error = "directive "s + std::string(data  + beg, off  - beg) + " invalid";
			Log::error(error, file_name, line_num);
			break;
		}
		//macros
		case '_':
		{
			if(!in_macro)
			{
				Log::error("macro argument symbol outside macro", file_name, line_num);
				off++;
				break;
			}
			int const beg = off + 1;
			off++;
			while(off < size && Utils::is_id_char(data[off]))
				off++;

			if(0 == off - beg)
			{
				Log::error("'_' requires name of parameter", file_name, line_num);
				break;
			}

			output.emplace_back(t_Token::mpr, strings_size(), file_name, line_num);
			strings_add(std::string(data + beg, off - beg));
			break;
		}
		case '@':
		{
			int const beg = off + 1;
			off++;
			while(off < size && Utils::is_id_char(data[off]))
				off++;

			if(0 == off - beg)
			{
				Log::error("stray macro symbol `@`", file_name, line_num);
				break;
			}

			if(5 == off - beg
			&& 0 == strncmp(data + beg, "macro", 5))
			{
				in_macro = true;
				line = line_num;
				output.emplace_back(t_Token::mst, 0,  file_name, line_num);
				break;
			}
			else if(3 == off - beg
			     && 0 == strncmp(data + beg, "end", 3))
			{
				in_macro = false;
				output.emplace_back(t_Token::men, 0,  file_name, line_num);
				break;
			}

			output.emplace_back(t_Token::mus, strings_size(), file_name, line_num);
			strings_add(std::string(data + beg, off - beg));

			break;
		}
	case '#':
	{
		if(off == size - 1
		|| Utils::is_whitespace(data[off + 1])
		|| '}' == data[off + 1])
		{
			if(in_expression)
			{
				output.emplace_back(t_Token::exo, '#', file_name, line_num);
				off++;
				break;
			}
			else
				goto process_stray_character;
		}

		//figure out the length
		off++;
		int const beg = off;

		while(off < size && Utils::is_id_char(data[off]))
			off++;

		output.emplace_back(t_Token::udf, strings_size(), file_name, line_num);
		strings_add(std::string(data + beg, off - beg));
		break;
	}
	case '$':
	{
		if(off == size - 1
		|| Utils::is_whitespace(data[off + 1])
		|| '}' == data[off + 1])
		{
			if(in_expression)
			{
				output.emplace_back(t_Token::exo, '$', file_name, line_num);
				off++;
				break;
			}
			else
				goto process_stray_character;
		}

		//figure out the length
		off++;
		int const beg = off;

		while(off < size && Utils::is_id_char(data[off]))
			off++;

		output.emplace_back(t_Token::uvr, strings_size(), file_name, line_num);
		strings_add(std::string(data + beg, off - beg));
		break;
	}
	process_string:
		#pragma GCC diagnostic ignored "-Wpedantic"
		//skips R 
		case 'a'...'z': case 'A'...'Q': case 'S'...'Z':
		#pragma GCC diagnostic pop
		{
			int dot_count = 0;
			//figure out the length
			int const beg = off;

			while(off < size)
			{
				if(Utils::is_id_char(data[off]))
				{
					off++;
					continue;
				}	
				if('.' == data[off])
				{
					if(0 < dot_count)
					{
						Log::error("more than 1 dot in identifier", file_name, line_num);
						off++;
						
						goto next_char;
					}
					dot_count++;
					off++;
					continue;
				}

				break;
			}

			//3 length strings are probably instructions
			if(off - beg == 3)
			{
				int instr_id = -1;

				constexpr static char const* const to_compare[] = {
					"add", "and", "ann", "cal", "cmp", "crd",
					"cwr", "dvu", "dvs", "fls", "hlt", "int",
					"irt", "jmp", "mls", "mlu", "mov", "mrd", 
					"mwr", "neg", "nop", "not", "orr", "pop", 
					"prd", "prf", "psh", "pwr", "ret", "rng", 
					"shl", "shr", "srd", "sub", "swr", "tst",
					"xor", "xrd", "xwr",

					"jaa", "jbe", "jbz", "jcc", "jae", "jaz",
					"jge", "jgz", "jgg", "jle", "jlz", "jll",
					"jnc", "jbb", "jno", "jns", "jnz", "jne",
					"joo", "jss", "jzz", "jee", 

					"maa", "mbe", "mbz", "mcc", "mae", "maz",
					"mge", "mgz", "mgg", "mle", "mlz", "mll",
					"mnc", "mbb", "mno", "mns", "mnz", "mne",
					"moo", "mss", "mzz", "mee", 

					"saa", "sbe", "sbz", "scc", "sae", "saz",
					"sge", "sgz", "sgg", "sle", "slz", "sll",
					"snc", "sbb", "sno", "sns", "snz", "sne",
					"soo", "sss", "szz", "see"
					};
				for(unsigned i = 0; i < sizeof(to_compare)/sizeof(to_compare[0]); i++)
				{
					if(0 == strncmp(data + beg, to_compare[i], 3))
					{
						instr_id = i;
						break;
					}
				}	
				
				//no instruction
				if(-1 == instr_id)
					goto insert_string;

				output.emplace_back(t_Token::ins, instr_id, file_name, line_num);
				break;
			}
			//3 length strings are probably external registers
			if(off - beg == 2)
			{
				int external_id  = -1;
				if(0 == strncmp(data + beg, "IP", 2)) external_id = 0;
				if(0 == strncmp(data + beg, "UI", 2)) external_id = 1;
				if(0 == strncmp(data + beg, "SP", 2)) external_id = 2;
				if(0 == strncmp(data + beg, "FL", 2)) external_id = 3;
					
				if(-1 == external_id)
					goto insert_string;

				output.emplace_back(t_Token::ext, external_id, file_name, line_num);
				break;
			}

	insert_string:
			if('.' == data[off - 1])
			{
				Log::error("label name cannot end in a dot", file_name, line_num);
				break;
			}
			output.emplace_back(t_Token::ulb, strings_size(), file_name, line_num);
			strings_add(std::string(data + beg, off - beg));
			break;
		}
	process_stray_character:
		default:
			std::string err = "stray character s";
			err.back() = data[off];
			Log::error(err, file_name, line_num);
			off++;
			break;
		}
	next_char:
		continue;
	}

	if(in_string)
		Log::error("string not terminated, started at: ", file_name, line);
	if(in_comment)
		Log::error("comment not terminated, started at: ", file_name, line);
	if(in_macro)
		Log::error("macro not terminated, started at: ", file_name, line);
	while(0 != arg_lists_started.size())
	{
		Log::error("arg list not terminated, started at: ", file_name, arg_lists_started.back());
		arg_lists_started.pop_back();
	}
	while(0 != expressions_started.size())
	{
		Log::error("expression not terminated, started at: ", file_name, expressions_started.back());
		expressions_started.pop_back();
	}

	File::destroy_error_handled(file);
	return;
}

