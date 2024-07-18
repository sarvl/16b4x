#include "./token.h"

#include <fcntl.h> //open, close
#include <sys/mman.h> //mmap, munmap
#include <unistd.h> //lseek

#include <cstring> //strncmp

#include "./../../utility/src/log.h"
#include "./../../utility/src/file.h"

using namespace std::literals;

t_Token::t_Token(){}
t_Token::t_Token(Type n_type, int const n_val, std::string_view const n_fn, int const n_ln)
	: type(n_type), val(n_val), file_name(n_fn), line_num(n_ln) {}

std::ostream&  operator<<(std::ostream& os, t_Token const& token)
{
	using enum t_Token::Type;
	switch(token.type)
	{
		case tex:
			os << "[tex " << strings[token.val] << "]";
			break;
		case num:
			os << "[num " << token.val << "]";
			break;
		case reg:
			os << "[reg " << token.val << "]";
			break;
		case dir:
			os << "[dir " << token.val << "]";
			break;
		case bcl:
			os << "[bcl]";
			break;
		case bcr:
			os << "[bcr]";
			break;
		case col:
			os << "[col]";
			break;
		case str:
			os << "[str " << strings[token.val] << "]";
			break;
		case ins:
			os << "[" << static_cast<t_Instruction_Id>(token.val) << "]";
			break;
		case ccc:
			os << "[ccc " << token.val << "]";
			break;
		case fla:
			os << "[fla " << token.val << "]";
			break;
		case ext:
			os << "[ext " << token.val << "]";
			break;
	}

	return os;
}


std::ostream&  operator<<(std::ostream& os, t_Instruction_Id const instr)
{
	constexpr static char const* const transl[] = {
		"add", "and", "cal", "cmp", "div", "hcf",
		"hlt", "int", "irt", "jmp", "jsc", "juc",
		"mcs", "mcu", "mov", "mrd", "mro", "mul",
		"mwo", "mwr", "not", "orr", "pop", "prd",
		"psh", "pwr", "pxr", "pxw", "ret", "shl",
		"shr", "sub", "tst", "xor", "xrd", "xwr",
		"nop"
		};
	os << transl[static_cast<int>(instr)];
	return os;
}


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
	bool in_comment = false;
	int off = 0;
	bool negative = false;
	while(off < size)
	{
		if(in_comment)
		{
			if(off + 1 <= size
			&& '*' == data[off]
			&& '/' == data[off + 1])
			{
				
				off+= 2;
				in_comment = false;
				continue;
			}
		
			off++;
			continue;
		}

		switch(data[off])
		{
		case '/':
			if(off + 1 == size)
				goto process_stray_character;
			if('*' == data[off + 1])
			{
				in_comment = true;
				break;
			}
			if('/' != data[off + 1])
				goto process_stray_character;

		[[fallthrough]];
		case '#':
		case ';':
			while(off   < size
			   && '\n' != data[off])
			   off++;
			break;

		case '{':
			output.emplace_back(t_Token::bcl, 0, file_name, line_num);
			off++;
			break;
		case '}':
			output.emplace_back(t_Token::bcr, 0, file_name, line_num);
			off++;
			break;
		case ':':
			output.emplace_back(t_Token::col, 0, file_name, line_num);
			off++;
			break;

		case '\n':
			line_num++;
		[[fallthrough]];
		case ' ':
		case ',':
		case '\t':
			off++;
			break;

		case 'R':
			//check whether the last char 
			if(off == size - 1)
				goto process_string;

			//check whether next is a number
			if(data[off + 1] < '0'
			|| data[off + 1] > '9')
				goto process_string;

			//check whether some separator after that 
			if(off == size - 2
			|| ' '  == data[off + 2]
			|| ','  == data[off + 2]
			|| '\t' == data[off + 2]
			|| '\n' == data[off + 2])
			{
				output.emplace_back(t_Token::reg, data[off + 1] - '0', file_name, line_num);
				off += 2;
				break;
			}
			goto process_string;

		case '-':
			negative = true;
			if(off == size - 1
			|| data[off + 1] < '0'
			|| data[off + 1] > '9')
				goto process_stray_character;

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
					else if(data[off] >= 'a' && data[off] <= 'z')
						num = (num << 4) + (data[off] - 'a' + 10);
					else if(data[off] >= 'A' && data[off] <= 'Z')
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
					if(data[off] >= '0' && data[off] <= '1')
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
				 if(data[off] >= '0' && data[off] <= '9')
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
			output.emplace_back(t_Token::dir, 0, file_name, line_num);
			off++;
			break;

	process_string:
		#pragma GCC diagnostic ignored "-Wpedantic"
		//skips R 
		case 'a'...'z': case 'A'...'Q': case 'S'...'Z':
		#pragma GCC diagnostic pop
		{
			//figure out the length
			int const beg = off;

			while(off < size && (
			//number
				   (data[off] >= '0' && data[off] <= '9')
			//lowercase
				|| (data[off] >= 'a' && data[off] <= 'z')
			//uppercase
				|| (data[off] >= 'A' && data[off] <= 'Z')
			//few additional
				|| '-' == data[off] || '_' == data[off]
				))
				off++;


			//3 length strings are probably instructions
			if(off - beg == 3)
			{
				int instr_id = -1;

				constexpr static char const* const to_compare[] = {
					"add", "and", "cal", "cmp", "div", "hcf",
					"hlt", "int", "irt", "jcs", "jcu", "jmp",
					"mcs", "mcu", "mov", "mrd", "mro", "mul",
					"mwo", "mwr", "not", "orr", "pop", "prd",
					"psh", "pwr", "pxr", "pxw", "ret", "shl",
					"shr", "sub", "tst", "xor", "xrd", "xwr",
					"nop"
					};
				for(int i = 0; i < 37; i++)
				{
					if(0 == strncmp(data + beg, to_compare[i], 3))
					{
						instr_id = i;
						break;
					}
				}	
				
				//no instruction
				if(-1 == instr_id)
				{
					output.emplace_back(t_Token::str, strings.size(), file_name, line_num);
					strings.emplace_back(data + beg, off - beg);
					break;
				}

				output.emplace_back(t_Token::ins, instr_id, file_name, line_num);
				break;
			}
			//3 length strings are probably external registers
			if(off - beg == 2)
			{
				int external_id  = -1;
				if(0 == strncmp(data + beg, "IP", 2)) external_id = 0;
				if(0 == strncmp(data + beg, "CF", 2)) external_id = 1;
				if(0 == strncmp(data + beg, "UI", 2)) external_id = 1;
				if(0 == strncmp(data + beg, "LR", 2)) external_id = 2;
				if(0 == strncmp(data + beg, "SP", 2)) external_id = 3;
				if(0 == strncmp(data + beg, "OF", 2)) external_id = 4;
				if(0 == strncmp(data + beg, "FL", 2)) external_id = 5;
				if(0 == strncmp(data + beg, "F1", 2)) external_id = 6;
				if(0 == strncmp(data + beg, "F2", 2)) external_id = 7;
					
				if(-1 == external_id)
				{
					output.emplace_back(t_Token::str, strings.size(), file_name, line_num);
					strings.emplace_back(data + beg, off - beg);
					break;
				}

				output.emplace_back(t_Token::ext, external_id, file_name, line_num);
				break;
			}

			output.emplace_back(t_Token::str, strings.size(), file_name, line_num);
			strings.emplace_back(data + beg, off - beg);
			break;
		}
	process_stray_character:
		default:
			std::string err = "stray character s";
			err.back() = data[off];
			Log::warning(err, file_name, line_num);
			off++;
			break;
		}
	}

	File::destroy_error_handled(file);
	return;
}

