#include "./save.h" 

#include <string>

#include <cstdio> //snprintf

#include <cstring> //strncpy

#include <fcntl.h> //open
#include <unistd.h> //write

#include "top/gen_utils/log.h"
#include "top/utils.h" //digit_count
#include "top/config.h"

/*
 * properly handles all formats
 * does error handling
 */
int save_instructions(std::vector<uint16_t>& instructions)
{
	/*
	 *	snprintf is not used for format because it is slower (slightly but measurably)
	 *	unless that format is particularly inconvientient as it is with vhdl output
	 */
	//open file for writing
	int const output_fd = open(Config::file_output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(-1 == output_fd)
	{
		Log::perror("output file could not be opened");
		return -7;
	}	
	int count = 0;
	int size  = 0;
	char* data = nullptr;

	constexpr static char transl[] = "0123456789ABCDEF";

	switch(Config::Format::format)
	{
	case Config::Format::txt_1:
	{
		count = instructions.size();
		size  = count * 4 + count;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr = instructions[i];
			data[5 * i + 0] = transl[(instr >> 12) & 0xF];
			data[5 * i + 1] = transl[(instr >>  8) & 0xF];
			data[5 * i + 2] = transl[(instr >>  4) & 0xF];
			data[5 * i + 3] = transl[(instr >>  0) & 0xF];
			data[5 * i + 4] = '\n';
		}
		break;
	}
	case Config::Format::txt_2:
	{
		 //padding
		while(0b1 & instructions.size())
			instructions.emplace_back(0);

		count = instructions.size() / 2;
		size  = count * 8 + count;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr0 = instructions[2 * i + 0];
			uint16_t const instr1 = instructions[2 * i + 1];
			data[9 * i + 0] = transl[(instr0 >> 12) & 0xF];
			data[9 * i + 1] = transl[(instr0 >>  8) & 0xF];
			data[9 * i + 2] = transl[(instr0 >>  4) & 0xF];
			data[9 * i + 3] = transl[(instr0 >>  0) & 0xF];
			data[9 * i + 4] = transl[(instr1 >> 12) & 0xF];
			data[9 * i + 5] = transl[(instr1 >>  8) & 0xF];
			data[9 * i + 6] = transl[(instr1 >>  4) & 0xF];
			data[9 * i + 7] = transl[(instr1 >>  0) & 0xF];
			data[9 * i + 8] = '\n';
		}
		break;
	}
	case Config::Format::txt_4:
	{
		//padding
		while(0b11 & instructions.size())
			instructions.emplace_back(0);

		count = instructions.size() / 4;
		size  = count * 16 + count;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr0 = instructions[4 * i + 0];
			uint16_t const instr1 = instructions[4 * i + 1];
			uint16_t const instr2 = instructions[4 * i + 2];
			uint16_t const instr3 = instructions[4 * i + 3];
			data[17 * i +  0] = transl[(instr0 >> 12) & 0xF];
			data[17 * i +  1] = transl[(instr0 >>  8) & 0xF];
			data[17 * i +  2] = transl[(instr0 >>  4) & 0xF];
			data[17 * i +  3] = transl[(instr0 >>  0) & 0xF];
			data[17 * i +  4] = transl[(instr1 >> 12) & 0xF];
			data[17 * i +  5] = transl[(instr1 >>  8) & 0xF];
			data[17 * i +  6] = transl[(instr1 >>  4) & 0xF];
			data[17 * i +  7] = transl[(instr1 >>  0) & 0xF];
			data[17 * i +  8] = transl[(instr2 >> 12) & 0xF];
			data[17 * i +  9] = transl[(instr2 >>  8) & 0xF];
			data[17 * i + 10] = transl[(instr2 >>  4) & 0xF];
			data[17 * i + 11] = transl[(instr2 >>  0) & 0xF];
			data[17 * i + 12] = transl[(instr3 >> 12) & 0xF];
			data[17 * i + 13] = transl[(instr3 >>  8) & 0xF];
			data[17 * i + 14] = transl[(instr3 >>  4) & 0xF];
			data[17 * i + 15] = transl[(instr3 >>  0) & 0xF];
			data[17 * i + 16] = '\n';
		}
		break;
	}
	case Config::Format::txt_c:
	{
		count = instructions.size();
		size  = count * 4;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr = instructions[i];
			data[4 * i + 0] = transl[(instr >> 12) & 0xF];
			data[4 * i + 1] = transl[(instr >>  8) & 0xF];
			data[4 * i + 2] = transl[(instr >>  4) & 0xF];
			data[4 * i + 3] = transl[(instr >>  0) & 0xF];
		}
		break;
	}

	case Config::Format::bin_1:
	{
		count = instructions.size();
		size  = count * 2 + count;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr = instructions[i];
			data[3 * i + 0] = (instr >>  8) & 0xFF;
			data[3 * i + 1] = (instr >>  0) & 0xFF;
			data[3 * i + 2] = '\n';
		}
		break;
	}
	case Config::Format::bin_2:
	{
		 //padding
		while(0b1 & instructions.size())
			instructions.emplace_back(0);

		count = instructions.size() / 2;
		size  = count * 4 + count;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr0 = instructions[2 * i + 0];
			uint16_t const instr1 = instructions[2 * i + 1];
			data[5 * i + 0] = (instr0 >>  8) & 0xFF;
			data[5 * i + 1] = (instr0 >>  0) & 0xFF;
			data[5 * i + 2] = (instr1 >>  8) & 0xFF;
			data[5 * i + 3] = (instr1 >>  0) & 0xFF;
			data[5 * i + 4] = '\n';
		}
		break;
	}
	case Config::Format::bin_4:
	{
		//padding
		while(0b11 & instructions.size())
			instructions.emplace_back(0);

		count = instructions.size() / 4;
		size  = count * 8 + count;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr0 = instructions[4 * i + 0];
			uint16_t const instr1 = instructions[4 * i + 1];
			uint16_t const instr2 = instructions[4 * i + 2];
			uint16_t const instr3 = instructions[4 * i + 3];
			data[9 * i + 0] = (instr0 >> 8) & 0xFF;
			data[9 * i + 1] = (instr0 >> 0) & 0xFF;
			data[9 * i + 2] = (instr1 >> 8) & 0xFF;
			data[9 * i + 3] = (instr1 >> 0) & 0xFF;
			data[9 * i + 4] = (instr2 >> 8) & 0xFF;
			data[9 * i + 5] = (instr2 >> 0) & 0xFF;
			data[9 * i + 6] = (instr3 >> 8) & 0xFF;
			data[9 * i + 7] = (instr3 >> 0) & 0xFF;
			data[9 * i + 8] = '\n';
		}
		break;
	}
	case Config::Format::bin_c:
	{
		count = instructions.size();
		size  = count * 2;
		data  = new char[size];

		for(int i = 0; i < count; i++)
		{
			uint16_t const instr = instructions[i];
			data[2 * i + 0] = (instr >> 8) & 0xFF;
			data[2 * i + 1] = (instr >> 0) & 0xFF;
		}
		break;
	}

	case Config::Format::vhdl_1:
	{
		count = instructions.size();

		int const digits = Utils::digit_count(count);
		//                           =>x" hexd ",  \n
		int const per_line = digits + 4 + 4 + 2 + 1;
		size  = count * per_line
		//      OTHERS => x"0000"
		      + 17;

		data = new char[size];
		char* ptr = data;
		for(int i = 0; i < count; i++)
		{
			if(0 == instructions[i])
			{
				size = size - per_line; 
				continue;
			}
			
			//            null, will be overwritten anyway
			snprintf(ptr, per_line + 1,
			         "%.*d=>x\"%04X\",\n", digits, i, instructions[i]); 
			ptr += per_line;
		}
		strncpy(ptr, "OTHERS => x\"0000", 17);
		*(ptr  + 16) = '"'; //overwrite null
		break;
	}
	case Config::Format::vhdl_2:
	{
		//padding
		while(0b1 & instructions.size())
			instructions.emplace_back(0);

		count = instructions.size();

		int const digits = Utils::digit_count(count);
		//                               =>x" hexd ",  \n
		int const per_line = 2 * (digits + 4 + 4 + 2) + 1;
		size  = count/2 * per_line
		//      OTHERS => x"0000"
		      + 17;

		data = new char[size];
		char* ptr = data;
		for(int i = 0; i < count; i += 2)
		{
			if(0 == instructions[i + 0]
			&& 0 == instructions[i + 1])
			{
				size = size - per_line;
				continue;
			}
			
			//            null, will be overwritten anyway
			snprintf(ptr, per_line + 1,
			         "%.*d=>x\"%04X\",%.*d=>x\"%04X\",\n", digits, i, instructions[i], digits, i+1, instructions[i + 1]); 
			ptr += per_line;
		}
		strncpy(ptr, "OTHERS => x\"0000", 17);
		*(ptr  + 16) = '"'; //overwrite null
		break;
	}
	case Config::Format::vhdl_4:
	{
		//padding
		while(0b11 & instructions.size())
			instructions.emplace_back(0);

		count = instructions.size();

		int const digits = Utils::digit_count(count);
		//                               =>x" hexd ",  \n
		int const per_line = 4 * (digits + 4 + 4 + 2) + 1;
		size  = count/4 * per_line
		//      OTHERS => x"0000"
		      + 17;

		data = new char[size];
		char* ptr = data;
		for(int i = 0; i < count; i += 4)
		{
			if(0 == instructions[i + 0]
			&& 0 == instructions[i + 1]
			&& 0 == instructions[i + 2]
			&& 0 == instructions[i + 3])
			{
				size = size - per_line;
				continue;
			}
			
			//            null, will be overwritten anyway
			snprintf(ptr, per_line + 1,
			         "%.*d=>x\"%04X\",%.*d=>x\"%04X\",%.*d=>x\"%04X\",%.*d=>x\"%04X\",\n", 
			         digits, i+0, instructions[i+0], digits, i+1, instructions[i+1],
			         digits, i+2, instructions[i+2], digits, i+3, instructions[i+3]); 
			ptr += per_line;
		}
		strncpy(ptr, "OTHERS => x\"0000", 17);
		*(ptr  + 16) = '"'; //overwrite null
		break;
	}
	case Config::Format::vhdl_5:
	{
		//padding
		while(instructions.size() % 5 != 0)
			instructions.emplace_back(0);

		count = instructions.size();

		int const digits = Utils::digit_count(count);
		//                               =>x" hexd ",  \n
		int const per_line = 5 * (digits + 4 + 4 + 2) + 1;
		size  = count/5 * per_line
		//      OTHERS => x"0000"
		      + 17;

		data = new char[size];
		char* ptr = data;
		for(int i = 0; i < count; i += 5)
		{
			if(0 == instructions[i + 0]
			&& 0 == instructions[i + 1]
			&& 0 == instructions[i + 2]
			&& 0 == instructions[i + 3]
			&& 0 == instructions[i + 4])
			{
				size = size - per_line;
				continue;
			}
			
			//            null, will be overwritten anyway
			snprintf(ptr, per_line + 1,
			         "%.*d=>x\"%04X\",%.*d=>x\"%04X\",%.*d=>x\"%04X\",%.*d=>x\"%04X\",%.*d=>x\"%04X\",\n", 
			         digits, i+0, instructions[i+0], digits, i+1, instructions[i+1],
			         digits, i+2, instructions[i+2], digits, i+3, instructions[i+3], 
			         digits, i+4, instructions[i+4]);
			ptr += per_line;
		}
		strncpy(ptr, "OTHERS => x\"0000", 17);
		*(ptr  + 16) = '"'; //overwrite null
		break;
	}
	//not possible
	default:
		return -1;
	}

	if(-1 == write(output_fd, data, size))
	{
		Log::perror("write failed");
		close(output_fd);
		delete[] data;
		return -8;
	}
	delete[] data;

	close(output_fd);
	return 0;
}
/*
 * does error handling
 */
int save_symbols     (std::vector<t_Label>  const& labels)
{
	int const symbols_fd = open("symbols.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(-1 == symbols_fd)
	{
		Log::perror("symbols file could not be opened");
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
			"%*s %d\n", static_cast<int>(label.name.size()), &label.name[0], label.value);

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
