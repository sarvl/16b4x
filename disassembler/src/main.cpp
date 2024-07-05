#include <iostream>
#include <cstdio>
#include <string_view>

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

int main(int const argc, char const* const argv[])
{
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

		unsigned const opcode_short = (instruction >> 11) & 0x1F;
		unsigned const opcode_rr    = (instruction      ) & 0x1F;
		unsigned const opcode_long  = (instruction >>  8) & 0xFF; 
		unsigned const rf           = (instruction >>  5) & 0x07;
		unsigned const rs           = (instruction >>  8) & 0x07;
		unsigned const ccc          = (instruction >>  1) & 0x07;
		unsigned const socz         = (instruction >>  4) & 0x0F;

		
		unsigned const imm8_long_t  = 0
			| ((instruction & 0x0001) <<  7)
			| ((instruction & 0x00FE) >>  1)
			;
		unsigned const imm5_long_t  = 0
			| ((instruction & 0x0001) <<  4)
			| ((instruction & 0x001E) >>  1)
			;
		unsigned const imm8_t       = 0
			| ((instruction & 0x0001) <<  7)
			| ((instruction & 0x0700) >>  4)
			| ((instruction & 0x001E) >>  1)
			;
		unsigned const imm8_j_t      = 0
			| ((instruction & 0x0001) <<  7)
			| ((instruction & 0x07F0) >>  4)
			;
		unsigned const imm11_t      = 0
			| ((instruction & 0x0001) << 10)
			| ((instruction & 0x07FE) >>  1)
			;

		signed const imm8_long = static_cast<signed>(imm8_long_t << 24) >> 24;
		signed const imm5_long = static_cast<signed>(imm5_long_t << 27) >> 27;
		signed const imm8      = static_cast<signed>(imm8_t      << 24) >> 24;
		signed const imm8_j    = static_cast<signed>(imm8_j_t    << 24) >> 24;
		signed const imm11     = static_cast<signed>(imm11_t     << 21) >> 21;

		char ccc_str[5];
		int id = 0;
		bool exists = false;
		if(ccc & 0b0100) {ccc_str[id] = 'L'; id++; exists = true;}
		if(ccc & 0b0010) {ccc_str[id] = 'E'; id++; exists = true;}
		if(ccc & 0b0001) {ccc_str[id] = 'G'; id++; exists = true;}
		if(exists) {ccc_str[id] = ' '; id++;}
		ccc_str[id] = '\0';

		char socz_str[6];
		exists = false;
		socz_str[0] = ' ';
		id = 1;
		if(socz & 0b1000) {socz_str[id] = 'S'; id++; exists = true;}
		if(socz & 0b0100) {socz_str[id] = 'O'; id++; exists = true;}
		if(socz & 0b0010) {socz_str[id] = 'C'; id++; exists = true;}
		if(socz & 0b0001) {socz_str[id] = 'Z'; id++; exists = true;}
		if(exists) {socz_str[id] = ' '; id++;}
		else       {id = 0;};
		socz_str[id] = '\0';


		constexpr static char xrd_str[8][3] = {
			"IP", "CF", "LR", "SP", "OF", "FL", "F1", "F2"};
		constexpr static char xwr_str[8][3] = {
			"IP", "UI", "LR", "SP", "OF", "FL", "F1", "F2"};

		char const* const xrd_xr = xrd_str[rs];
		char const* const xwr_xr = xwr_str[rf];
        
		//determine opcode location
		switch(opcode_short)
		{
		case 0x00:
			switch(opcode_rr)
			{
			case 0x00: printf("INVALID INSTRUCTION\n");       break;
			case 0x01: printf("INVALID INSTRUCTION\n");       break;
			case 0x02: printf("INVALID INSTRUCTION\n");       break;
			case 0x03: printf("INVALID INSTRUCTION\n");       break;
			case 0x04: printf("INVALID INSTRUCTION\n");       break;
			case 0x05: printf("INVALID INSTRUCTION\n");       break;
			case 0x06: printf("INVALID INSTRUCTION\n");       break;
			case 0x07: printf("INVALID INSTRUCTION\n");       break;
			case 0x08: printf("prd R%d, R%d\n", rf, rs);      break;
			case 0x09: printf("pwr R%d, R%d\n", rs, rf);      break;
			case 0x0A: printf("xrd R%d, %.2s\n", rf, xrd_xr); break;
			case 0x0B: printf("xwr %.2s, R%d\n", xwr_xr, rs); break;
			case 0x0C: printf("mrd R%d, R%d\n", rf, rs);      break;
			case 0x0D: printf("mwr R%d, R%d\n", rs, rf);      break;
			case 0x0E: printf("mro R%d, R%d\n", rf, rs);      break;
			case 0x0F: printf("mwo R%d, R%d\n", rs, rf);      break;
			case 0x10: printf("mul R%d, R%d\n", rf, rs);      break;
			case 0x11: printf("cmp R%d, R%d\n", rf, rs);      break;
			case 0x12: printf("div R%d, R%d\n", rf, rs);      break;
			case 0x13: printf("tst R%d, R%d\n", rf, rs);      break;
			case 0x14: printf("mov R%d, R%d\n", rf, rs);      break;
			case 0x15: printf("INVALID INSTRUCTION\n");       break;
			case 0x16: printf("INVALID INSTRUCTION\n");       break;
			case 0x17: printf("INVALID INSTRUCTION\n");       break;
			case 0x18: printf("add R%d, R%d\n", rf, rs);      break;
			case 0x19: printf("sub R%d, R%d\n", rf, rs);      break;
			case 0x1A: printf("not R%d, R%d\n", rf, rs);      break;
			case 0x1B: printf("and R%d, R%d\n", rf, rs);      break;
			case 0x1C: printf("orr R%d, R%d\n", rf, rs);      break;
			case 0x1D: printf("xor R%d, R%d\n", rf, rs);      break;
			case 0x1E: printf("shl R%d, R%d\n", rf, rs);      break;
			case 0x1F: printf("shr R%d, R%d\n", rf, rs);      break;
			}
			break;
		case 0x01:
			switch(opcode_long & 0b111)
			{
			case 0b000: printf("int %d\n", imm8_long);          break;
			case 0b001: printf("irt\n");                        break;
			case 0b010: printf("hlt\n");                        break;
			case 0b011: printf("hcf\n");                        break;
			case 0b100: printf("INVALID INSTRUCTION\n");        break;
			case 0b101: printf("INVALID INSTRUCTION\n");        break;
			case 0b110: printf("pxr R%d, %d\n", rf, imm5_long); break;
			case 0b111: printf("pxw %d, R%d\n", imm5_long, rf); break;
			}
			break;
		case 0x06:
		case 0x07:
			printf("PRG\n"); 
			break;
		case 0x15:
			switch(opcode_long & 0b111)
			{
			case 0b000: printf("pop R%d\n", rf);         break;
			case 0b001: printf("psh R%d\n", rf);         break;
			case 0b010: printf("cal R%d\n", rf);         break;
			case 0b011: printf("jmp R%d\n", rf);         break;
			case 0b100: printf("ret%.5s\n", socz_str);   break;
			case 0b101: printf("INVALID INSTRUCTION\n"); break;
			case 0b110: printf("INVALID INSTRUCTION\n"); break;
			case 0b111: printf("nop\n");                 break;
			}
			break;
		default:
			switch(opcode_short)
			{
			/*not encodable*/
			/*not encodable*/
			case 0x02: printf("cal %d\n", imm11);                      break;
			case 0x03: printf("jmp %d\n", imm11);                      break;
			case 0x04: printf("jcu %.4s%d\n", ccc_str, imm8_j);        break;
			case 0x05: printf("jcs %.4s%d\n", ccc_str, imm8_j);        break;
			case 0x08: printf("prd R%d, %d\n", rf, imm8);              break;
			case 0x09: printf("pwr %d, R%d\n", imm8, rf);              break;
			case 0x0A: printf("INVALID INSTRUCTION\n");                break;
			case 0x0B: printf("xwr %.2s, %d\n", xwr_xr, imm8);         break;
			case 0x0C: printf("mrd R%d, %d\n", rf, imm8);              break;
			case 0x0D: printf("mwr %d, R%d\n", imm8, rf);              break;
			case 0x0E: printf("mro R%d, %d\n", rf, imm8);              break;
			case 0x0F: printf("mwo %d, R%d\n", imm8, rf);              break;
			case 0x10: printf("mul R%d, %d\n", rf, imm8);              break;
			case 0x11: printf("cmp R%d, %d\n", rf, imm8);              break;
			case 0x12: printf("div R%d, %d\n", rf, imm8);              break;
			case 0x13: printf("tst R%d, %d\n", rf, imm8);              break;
			case 0x14: printf("mov R%d, %d\n", rf, imm8);              break;
			/*not encodable*/
			case 0x16: printf("mcu %.4sR%d, R%d\n", ccc_str, rf, rs);  break;
			case 0x17: printf("mcs %.4sR%d, R%d\n", ccc_str, rf, rs);  break;
			case 0x18: printf("add R%d, %d\n", rf, imm8);              break;
			case 0x19: printf("sub R%d, %d\n", rf, imm8);              break;
			case 0x1A: printf("not R%d, %d\n", rf, imm8);              break;
			case 0x1B: printf("and R%d, %d\n", rf, imm8);              break;
			case 0x1C: printf("orr R%d, %d\n", rf, imm8);              break;
			case 0x1D: printf("xor R%d, %d\n", rf, imm8);              break;
			case 0x1E: printf("shl R%d, %d\n", rf, imm8);              break;
			case 0x1F: printf("shr R%d, %d\n", rf, imm8);              break;
			}
			break;
		}


		line_num++;
		off += 5;
		continue;
	}


	if(-1 == File::destroy_error_handled(file))
		return -5;

	return 0;
}
