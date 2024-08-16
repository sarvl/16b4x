#include <iostream>
#include <cstdio>
#include <cstdint>
#include <string_view>
#include <cstring>

#include "./../../utility/src/log.h"
#include "./../../utility/src/file.h"

constexpr int hexd_to_val(char const hexd)
{
	constexpr signed char translate[128] = {
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	return translate[hexd & 0x7F];
}

extern void disassemble(uint16_t const instr);

int main(int const argc, char const* const argv[])
{
	//special interactive mode
	if(1 < argc && 0 == strcmp("stdin", argv[1]))
	{
		while(true)
		{
			char instr[8];
			//4 characters, newline and '\0'
			if((instr[0] = getchar()) == EOF
			||  instr[0] == '\n')
				return 0;

			if((instr[1] = getchar()) == EOF
			||  instr[1] == '\n'
			|| (instr[2] = getchar()) == EOF
			||  instr[2] == '\n'
			|| (instr[3] = getchar()) == EOF
			||  instr[3] == '\n')
			{
				Log::error("line does not contain full instruction, aborting");
				return -1;	
			}

			if((instr[4] = getchar()) != EOF
			&&  instr[4] != '\n')
			{
				Log::error("line contains too many characters, aborting");
				return -2;
			}

			int const parts[4] = {
				hexd_to_val(instr[0]),
				hexd_to_val(instr[1]),
				hexd_to_val(instr[2]),
				hexd_to_val(instr[3])};

			if(-1 == parts[0]
			|| -1 == parts[1]
			|| -1 == parts[2]
			|| -1 == parts[3])
			{
				Log::error("line contains character that is not hexadecimal digit, aborting");
				return -3;
			}

			unsigned const instruction = 0
				| (parts[0] << 12)
				| (parts[1] <<  8)
				| (parts[2] <<  4)
				| (parts[3] <<  0)
				;

			disassemble(instruction);
		}
	}


	std::string_view const file_name = (1 == argc ? "out.bin" : argv[1]);

	File::t_File file;
	if(-1 == File::create_error_handled(&file, file_name))
		return -1;

	char const* const data = file.data;
	int  const        size = file.size;

	int line_num = 1;
	int off = 0;
	while(off < size)
	{
		if(off + 5 > size
		|| data[off + 0] == '\n'
		|| data[off + 1] == '\n'
		|| data[off + 2] == '\n'
		|| data[off + 3] == '\n')
		{
			Log::error(
				"line does not contain full instruction, aborting",
				file_name,
				line_num);
			
			//not handled because main error is above
			//if something fails, will be handled by OS on cleanup
			(void)File::destroy(file);
			return -2;
		}
		if(data[off + 4] != '\n')
		{
			Log::error(
				"line contains too many characters, aborting",
				file_name,
				line_num);

			(void)File::destroy(file);
			return -3;
		}

		int const parts[4] = {
			hexd_to_val(data[off + 0]),
			hexd_to_val(data[off + 1]),
			hexd_to_val(data[off + 2]),
			hexd_to_val(data[off + 3])};

		if(-1 == parts[0]
		|| -1 == parts[1]
		|| -1 == parts[2]
		|| -1 == parts[3])
		{
			Log::error(
				"line contains character that is not hexadecimal digit, aborting",
				file_name,
				line_num);
			
			//not handled because main error is above
			//if something fails, will be handled by OS on cleanup
			(void)File::destroy(file);
			return -4;
		}

		unsigned const instruction = 0
			| (parts[0] << 12)
			| (parts[1] <<  8)
			| (parts[2] <<  4)
			| (parts[3] <<  0)
			;

		disassemble(instruction);

		line_num++;
		off += 5;
		continue;
	}


	if(-1 == File::destroy_error_handled(file))
		return -5;

	return 0;
}
