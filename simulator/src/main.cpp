#include <iostream>

#include <fcntl.h> //open, close
#include <unistd.h> //write

#include <string>
#include <string_view>

#include <unordered_set> //unordered_set
#include <unordered_map> //unordered_set
#include <memory> //unique_ptr

#include <cstdint>
#include <cstdio>
#include <cstring> //strncmp, memcpy

#include "./../../utility/src/log.h"
#include "./../../utility/src/file.h"

using namespace std::literals;

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

int parse_number(int& ind, char const* const data, int const size)
{
	int num = 0;
	int off = ind;
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
	ind = off;
	return num;
}


int extract_number_advance_ptr(int& index, std::string const& line)
{
	//move to next argument
	int ind = index;
	while(ind < static_cast<int>(line.size())
	   && ' ' != line[ind])
		ind++;
	while(ind < static_cast<int>(line.size())
	   && ' ' == line[ind])
		ind++;

	if(ind == static_cast<int>(line.size()))
		return -1;

	uint16_t const num = static_cast<uint16_t>(parse_number(ind, line.c_str(), line.size()));

	if(ind < static_cast<int>(line.size())
	&& ' ' != line[ind])
		return -2;

	index = ind;
	return num;
}


uint16_t memory[1 << 16];
uint16_t regs[8];
uint16_t extregs[8]; //DOES NOT INCLUDE UI
uint16_t ports[256];
uint16_t privilegedextregs[32];

uint16_t& extreg_IP = extregs[0];
uint16_t& extreg_CF = extregs[1];
uint16_t  extreg_UI = 0; //INTENTIONALLY NOT REFERENCE
uint16_t& extreg_LR = extregs[2];
uint16_t& extreg_SP = extregs[3];
uint16_t& extreg_OF = extregs[4];
uint16_t& extreg_FL = extregs[5];
uint16_t& extreg_F1 = extregs[6];
uint16_t& extreg_F2 = extregs[7];

struct t_State{
	uint16_t m_memory[1 << 16];
	uint16_t m_regs[8];
	uint16_t m_extregs[8]; //DOES NOT INCLUDE UI
	uint16_t m_ports[256];
	uint16_t m_privilegedextregs[32];
	uint16_t m_extreg_UI = 0;

	t_State()
	{
		std::memcpy(m_memory           , memory           , sizeof(uint16_t) * 1 << 16);
		std::memcpy(m_regs             , regs             , sizeof(uint16_t) * 8      );
		std::memcpy(m_extregs          , extregs          , sizeof(uint16_t) * 8      );
		std::memcpy(m_ports            , ports            , sizeof(uint16_t) * 256    );
		std::memcpy(m_privilegedextregs, privilegedextregs, sizeof(uint16_t) * 32     );

		m_extreg_UI = extreg_UI;
	}
};


#define F_SIGN ((extreg_FL >> 3) & 0b1)
#define F_OVER ((extreg_FL >> 2) & 0b1)
#define F_CARY ((extreg_FL >> 1) & 0b1)
#define F_ZERO ((extreg_FL >> 0) & 0b1)

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

		memory[line_num - 1] = instruction;

		line_num++;
		off += 5;
	}
	if(-1 == File::destroy_error_handled(file))
		return -5;


	std::unordered_map<uint16_t, std::string> symbols;
	//read symbols from symbols.txt if such file exists
	{
		File::t_File symbol_file;
		if(File::Error::no_error == File::create(&symbol_file, "symbols.txt"))
		{
			char const* const data = symbol_file.data;
			int  const        size = symbol_file.size;
			int ind = 0;
			int line = 0;
			while(ind < size)
			{
				int const beg = ind;
				//find space
				while(ind < size
				   && ' '  != data[ind]
				   && '\n' != data[ind])
				   ind++;
				int const end = ind;
				//advance till not space
				//find space
				while(ind < size
				   && ' ' == data[ind])
				   ind++;

				if(ind  == size
				|| '\n' == data[ind])
				{
					Log::error("symbol does not have number specified", "symbols.txt", line);

					File::destroy(symbol_file);
					return -5;
				}

				//parse the number
				int const num = parse_number(ind, data, size);
				//skip whitespace
				while(ind < size
				   && ' ' == data[ind])
					ind++;

				if(ind != size
				&& '\n' != data[ind])
				{
					Log::error("invalid number provided", "symbols.txt", line);

					File::destroy(symbol_file);
					return -6;
				}
				//is it possible to do it faster?
				symbols.emplace(static_cast<uint16_t>(num), std::string(data + beg, end - beg));
				line++;
				//skip newline
				ind++;
			}
		}

		File::destroy(symbol_file);
	}

	int const debug = 0;
	int const print = 0;

	bool ui_changed = false;
	bool running = false;
	bool const exit_on_error = true;

	std::unordered_set<uint16_t> breakpoints;
	std::unordered_map<int, std::unique_ptr<t_State>> savedstates;
	while(true)
	{
		if(!ui_changed)
			extreg_UI = 0;
		else
			ui_changed = false;

		if(breakpoints.contains(extreg_IP))
			running = false;
		
		if(debug && !running) 
		{
			while(true)
			{
				std::string line;
				std::getline(std::cin, line);

				if(0 == line.size())
					break;
				
				enum class t_Command{
					help, 
					cont, 
					dump_mem,  dump_reg,  dump_ext, 
					print_mem, print_reg, print_ext,
					set_mem,   set_reg,   set_ext,
					breakpoint_add, breakpoint_delete,
					instruction,
					save_state,
					restore_state,
					clear,
					quit,
					total_count,
					invalid
				};
				//bogus value to indicate not found
				t_Command command = t_Command::invalid;
				constexpr int command_count = static_cast<int>(t_Command::total_count);

				//full
				     if(0 == strncmp(line.c_str(), "help",              4)) command = t_Command::help;
				else if(0 == strncmp(line.c_str(), "continue",          8)) command = t_Command::cont;
				else if(0 == strncmp(line.c_str(), "dumpmem",           7)) command = t_Command::dump_mem;
				else if(0 == strncmp(line.c_str(), "dumpreg",           7)) command = t_Command::dump_reg;
				else if(0 == strncmp(line.c_str(), "dumpext",           7)) command = t_Command::dump_ext;
				else if(0 == strncmp(line.c_str(), "printmem",          8)) command = t_Command::print_mem;
				else if(0 == strncmp(line.c_str(), "printreg",          8)) command = t_Command::print_reg;
				else if(0 == strncmp(line.c_str(), "printext",          8)) command = t_Command::print_ext;
				else if(0 == strncmp(line.c_str(), "setmem",            6)) command = t_Command::set_mem;
				else if(0 == strncmp(line.c_str(), "setreg",            6)) command = t_Command::set_reg;
				else if(0 == strncmp(line.c_str(), "setext",            6)) command = t_Command::set_ext;
				else if(0 == strncmp(line.c_str(), "breakpointadd",    13)) command = t_Command::breakpoint_add;
				else if(0 == strncmp(line.c_str(), "breakpointdelete", 16)) command = t_Command::breakpoint_delete;
				else if(0 == strncmp(line.c_str(), "instruction",      11)) command = t_Command::instruction;
				else if(0 == strncmp(line.c_str(), "savestate",         9)) command = t_Command::save_state;
				else if(0 == strncmp(line.c_str(), "restorestate",     12)) command = t_Command::restore_state;
				else if(0 == strncmp(line.c_str(), "clear",             5)) command = t_Command::clear;
				else if(0 == strncmp(line.c_str(), "quit",              4)) command = t_Command::quit;

				if(t_Command::invalid == command
				&& (2 == line.size() || ' ' == line[2]))
				{
					//2 letter mnemonic
					constexpr static char const* const mnemonic_2_letter[command_count] = {
						"hl", "co", "dm", "dr", "dx", "pm", "pr", 
						"px", "sm", "sr", "sx", "ba", "bd", "pi",
						"ss", "rs", "cl", "qt"
					};
					for(int i = 0; i < command_count; i++)
						if(0 == strncmp(line.c_str(), mnemonic_2_letter[i], 2)) {command = static_cast<t_Command>(i); break;}
				}

				if(t_Command::invalid == command
				&& (1 == line.size() || ' ' == line[1]))
				{
					//1 letter shortcut for some
					constexpr static char mnemonic_1_letter[command_count] = {
						'h' , 'c' , 'm' , 'r' , 'x' , 'p' , '\0', 
						'\0', 's' , '\0', '\0', 'b' , 'd' , '\0', 
						'\0', '\0', 'l' , 'q'
					};
					for(int i = 0; i < command_count; i++)
						if(mnemonic_1_letter[i] == line[0]) {command = static_cast<t_Command>(i); break;}
				}

				switch(command)
				{
				case t_Command::help:
					printf("help             hl h\n"
					       "    prints this help\n"
					       "continue         co h\n"
					       "    continues program until breakpoint or completion\n"
					       "dumpmem          dm m\n"
					       "    dumps entire memory into dump.txt\n"
					       "dumpreg          dr r\n"
					       "    dumps contents of registers to stdout\n"
					       "dumpext          dx x\n"
					       "    dumps contents of external registers to stdout\n"
					       "printmem         pm p\n"
					       "    prints contents of memory address following commnand to stdout\n"
					       "printreg         pr\n"
					       "    prints contents of register following commnand to stdout\n"
					       "printext         px\n"
					       "    prints contents of external register following commnand to stdout\n"
					       "setmem           sm s\n"
					       "    sets contents of memory address following commnand to value following address\n"
					       "setreg           sr\n"
					       "    sets contents of register following commnand to value following register id\n"
					       "setext           sx\n"
					       "    sets contents of external register following commnand to value following external register id\n"
					       "breakpointadd    bp b\n"
					       "    sets breakpoint at specified address\n"
					       "breakpointdelete bd d\n"
					       "    removes breakpoint from specified address\n"
					       "instruction      in\n"
					       "    prints current IP and instruction\n"
					       "savestate        ss\n"
					       "    saves current state of the simulator\n"
					       "restorestate     rs\n"
					       "    restores saved state of the debugger\n"
					       "clear            cl l\n"
					       "    clears the screen\n"
					       "quit             qt q\n"
					       "    quits the program\n");
					break;
				case t_Command::cont:
					running = true;
					goto next_instruction;
				case t_Command::dump_mem:
				{
					int const fd = open("dump.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);							
					char to_write[6] = "    \n";
					for(int i = 0; i < (1 << 16); i++)
					{
						snprintf(to_write, 5, "%04X", memory[i]);
						to_write[4] = '\n';
						write(fd, to_write, 5);
					}

					close(fd);
					printf("done\n");
					break;
				}
				case t_Command::dump_reg:
					for(int i = 0; i < 8; i++)
						printf("R%d %5d 0x%04X\n", i, regs[i], regs[i]);
					break;
				case t_Command::dump_ext:
					printf("IP %5d 0x%04X\n", extreg_IP, extreg_IP);
					printf("CF %5d 0x%04X\n", extreg_CF, extreg_CF);
					printf("UI %5d 0x%04X\n", extreg_UI, extreg_UI);
					printf("LR %5d 0x%04X\n", extreg_LR, extreg_LR);
					printf("SP %5d 0x%04X\n", extreg_SP, extreg_SP);
					printf("OF %5d 0x%04X\n", extreg_OF, extreg_OF);
					printf("FL %5d 0x%04X\n", extreg_FL, extreg_FL);
					printf("F1 %5d 0x%04X\n", extreg_F1, extreg_F1);
					printf("F2 %5d 0x%04X\n", extreg_F2, extreg_F2);
					break;
				case t_Command::print_mem:
				{
					int ind = 0;

					int const addr = extract_number_advance_ptr(ind, line);

					if(-1 == addr) {Log::error("printmem requires address as an argument"); break;}
					if(-2 == addr) {Log::error("invalid number provided");                  break;}

					printf("M[0x%04X] = %5d 0x%04X\n", addr, memory[addr], memory[addr]);
					break;
				}
				case t_Command::print_reg:
				{
					int ind = 0;

					int const reg = extract_number_advance_ptr(ind, line);

					if(-1 == reg) {Log::error("printreg requires number as an argument"); break;}
					if(-2 == reg) {Log::error("invalid number provided");                 break;}
					if(reg > 7  ) {Log::error("too big number provided");                 break;}

					printf("R%d = %5d 0x%04X\n", reg, regs[reg], regs[reg]);
					break;
				}
				case t_Command::print_ext:
				{
					//move to next argument
					int ind = 0;
					while(ind < static_cast<int>(line.size())
					   && ' ' != line[ind])
						ind++;
					while(ind < static_cast<int>(line.size())
					   && ' ' == line[ind])
						ind++;

					if(ind + 2 > size)
					{
						Log::error("printext requires external register as argument");
						break;
					}
					     if(0 == strncmp(line.c_str() + ind, "IP", 2)
					     || 0 == strncmp(line.c_str() + ind, "ip", 2)) printf("IP %5d 0x%04X\n", extreg_IP, extreg_IP);
					else if(0 == strncmp(line.c_str() + ind, "CF", 2)
					     || 0 == strncmp(line.c_str() + ind, "cf", 2)) printf("CF %5d 0x%04X\n", extreg_CF, extreg_CF);
					else if(0 == strncmp(line.c_str() + ind, "UI", 2)
					     || 0 == strncmp(line.c_str() + ind, "ui", 2)) printf("UI %5d 0x%04X\n", extreg_UI, extreg_UI);
					else if(0 == strncmp(line.c_str() + ind, "LR", 2)
					     || 0 == strncmp(line.c_str() + ind, "lr", 2)) printf("LR %5d 0x%04X\n", extreg_LR, extreg_LR);
					else if(0 == strncmp(line.c_str() + ind, "SP", 2)
					     || 0 == strncmp(line.c_str() + ind, "sp", 2)) printf("SP %5d 0x%04X\n", extreg_SP, extreg_SP);
					else if(0 == strncmp(line.c_str() + ind, "OF", 2)
					     || 0 == strncmp(line.c_str() + ind, "of", 2)) printf("OF %5d 0x%04X\n", extreg_OF, extreg_OF);
					else if(0 == strncmp(line.c_str() + ind, "FL", 2)
					     || 0 == strncmp(line.c_str() + ind, "fl", 2)) printf("FL %5d 0x%04X\n", extreg_FL, extreg_FL);
					else if(0 == strncmp(line.c_str() + ind, "F1", 2)
					     || 0 == strncmp(line.c_str() + ind, "f1", 2)) printf("F1 %5d 0x%04X\n", extreg_F1, extreg_F1);
					else if(0 == strncmp(line.c_str() + ind, "F2", 2)
					     || 0 == strncmp(line.c_str() + ind, "f2", 2)) printf("F2 %5d 0x%04X\n", extreg_F2, extreg_F2);
					else Log::error("invalid register specified");

					break;
				}
				case t_Command::set_mem:
				{
					int ind = 0;

					int const addr = extract_number_advance_ptr(ind, line);

					if(-1 == addr) {Log::error("setmem requires address as an argument"); break;}
					if(-2 == addr) {Log::error("invalid number provided");                break;}

					int const val = extract_number_advance_ptr(ind, line);

					if(-1 == val) {Log::error("setmem requires value as an argument"); break;}
					if(-2 == val) {Log::error("invalid number provided");              break;}

					memory[addr] = val;
					break;
				}
				case t_Command::set_reg:
				{
					int ind = 0;

					int const reg = extract_number_advance_ptr(ind, line);

					if(-1 == reg) {Log::error("setreg requires address as an argument"); break;}
					if(-2 == reg) {Log::error("invalid number provided");                break;}
					if(reg > 7  ) {Log::error("too big number provided");                break;}

					int const val = extract_number_advance_ptr(ind, line);

					if(-1 == val) {Log::error("setreg requires value as an argument"); break;}
					if(-2 == val) {Log::error("invalid number provided");              break;}

					regs[reg] = val;
					break;
				}
				case t_Command::set_ext:
				{
					//move to next argument
					int ind = 0;
					while(ind < static_cast<int>(line.size())
					   && ' ' != line[ind])
						ind++;
					while(ind < static_cast<int>(line.size())
					   && ' ' == line[ind])
						ind++;

					if(ind + 2 > size)
					{
						Log::error("setext requires external register as argument");
						break;
					}

					uint16_t* dest = nullptr;
					     if(0 == strncmp(line.c_str() + ind, "IP", 2)
					     || 0 == strncmp(line.c_str() + ind, "ip", 2)) dest = &extreg_IP;
					else if(0 == strncmp(line.c_str() + ind, "CF", 2)
					     || 0 == strncmp(line.c_str() + ind, "cf", 2)) dest = &extreg_CF;
					else if(0 == strncmp(line.c_str() + ind, "UI", 2)
					     || 0 == strncmp(line.c_str() + ind, "ui", 2)) dest = &extreg_UI;
					else if(0 == strncmp(line.c_str() + ind, "LR", 2)
					     || 0 == strncmp(line.c_str() + ind, "lr", 2)) dest = &extreg_LR;
					else if(0 == strncmp(line.c_str() + ind, "SP", 2)
					     || 0 == strncmp(line.c_str() + ind, "sp", 2)) dest = &extreg_SP;
					else if(0 == strncmp(line.c_str() + ind, "OF", 2)
					     || 0 == strncmp(line.c_str() + ind, "of", 2)) dest = &extreg_OF;
					else if(0 == strncmp(line.c_str() + ind, "FL", 2)
					     || 0 == strncmp(line.c_str() + ind, "fl", 2)) dest = &extreg_FL;
					else if(0 == strncmp(line.c_str() + ind, "F1", 2)
					     || 0 == strncmp(line.c_str() + ind, "f1", 2)) dest = &extreg_F1;
					else if(0 == strncmp(line.c_str() + ind, "F2", 2)
					     || 0 == strncmp(line.c_str() + ind, "f2", 2)) dest = &extreg_F2;
					else {Log::error("invalid register specified"); break;}

					int const val = extract_number_advance_ptr(ind, line);

					if(-1 == val) {Log::error("setext requires value as an argument"); break;}
					if(-2 == val) {Log::error("invalid number provided");              break;}

					*dest = val;
					break;
				}
				case t_Command::breakpoint_add:
				{
					int ind = 0;

					int const addr = extract_number_advance_ptr(ind, line);

					if(-1 == addr) {Log::error("breakpointadd requires address as an argument"); break;}
					if(-2 == addr) {Log::error("invalid number provided");                       break;}
					
					if(breakpoints.contains(addr))
						Log::warning("breakpoint already exists");
					else
						breakpoints.insert(addr);
					break;
				}
				case t_Command::breakpoint_delete:
				{
					int ind = 0;

					int const addr = extract_number_advance_ptr(ind, line);

					if(-1 == addr) {Log::error("breakpointdelete requires address as an argument"); break;}
					if(-2 == addr) {Log::error("invalid number provided");                          break;}
				
					if(0 == breakpoints.erase(addr))
						Log::warning("no breakpoint at specified address");
					break;
				}
				case t_Command::instruction:
					printf("0x%04X 0x%04X\n", extreg_IP, memory[extreg_IP]);
					break;
				case t_Command::clear:
					system("clear\n");
					break;
				case t_Command::quit:
					goto finish;
					break;
				case t_Command::save_state:
				{
					int const next_id = savedstates.size();
					
					savedstates[next_id] = std::make_unique<t_State>();
					printf("saved state as %d\n", next_id);

					break;
				}
				case t_Command::restore_state:
				{
					int ind = 0;
					int const id = extract_number_advance_ptr(ind, line);

					if(-1 == id) {Log::error("restorestate requires id as an argument"); break;}
					if(-2 == id) {Log::error("invalid number provided");                 break;}

					
					t_State const& state = *savedstates[id];

					std::memcpy(memory           , state.m_memory           , sizeof(uint16_t) * (1 << 16));
					std::memcpy(regs             , state.m_regs             , sizeof(uint16_t) * (8      ));
					std::memcpy(extregs          , state.m_extregs          , sizeof(uint16_t) * (8      ));
					std::memcpy(ports            , state.m_ports            , sizeof(uint16_t) * (256    ));
					std::memcpy(privilegedextregs, state.m_privilegedextregs, sizeof(uint16_t) * (32     ));
					extreg_UI = state.m_extreg_UI;	
					
					extreg_IP--;
					goto next_instruction;
				}
				case t_Command::total_count:
				case t_Command::invalid:
					Log::warning("Invalid command");
					break;
				}
			} 
		}

	next_instruction:


		uint16_t const instruction = memory[extreg_IP];
		extreg_IP++;

		if(debug) {printf("0x%04X - 0x%04X - ", extreg_IP - 1, instruction);}
		
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

		int16_t imm8_long =                    (static_cast<signed>(imm8_long_t << 24) >> 24);
		int16_t imm5_long =                    (static_cast<signed>(imm5_long_t << 27) >> 27);
		int16_t imm8      = (extreg_UI << 8) | (static_cast<signed>(imm8_t      << 24) >> 24);
		int16_t imm8_j    = (extreg_UI << 8) | (static_cast<signed>(imm8_j_t    << 24) >> 24);
		int16_t imm11     = (extreg_UI << 8) | (static_cast<signed>(imm11_t     << 21) >> 21);

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


		int res = 0;
		bool set_flags = false;
		int overflow = 0;
 
		//determine opcode location
		switch(opcode_short)
		{
		case 0x00:
			//cast here so they dont cause problems later

			switch(opcode_rr)
			{
			//do not warn about range
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpedantic"
			case 0x00 ... 0x07: 
			#pragma GCC diagnostic pop
				printf("INVALID INSTRUCTION\n");
				
				if(exit_on_error)
					goto finish; 
				break;
			//prd R R
			case 0x08: 
				if(debug)
				{
					printf("prd R%d, R%d\n", rf, rs);
					printf("R%d <-- %5d (= 0x%04X)\n", rf, ports[regs[rs]], ports[regs[rs]]);
				}

				regs[rf] = ports[regs[rs]];

				break;
			//pwr R R
			case 0x09: 
				if(debug)
				{
					printf("pwr R%d, R%d\n", rs, rf);
					printf("P[0x%04X] <-- %5d (= 0x%04X)\n", regs[rf], regs[rs], regs[rs]);
				}

				ports[regs[rs]] = regs[rf];

				break;
			//xrd R E
			case 0x0A: 
				if(debug) 
				{
					printf("xrd R%d, %.2s\n", rf, xrd_xr);
					printf("R%d <-- %d (= 0x%04X)\n", rf, extregs[rs], extregs[rs]);
				}

				regs[rf] = extregs[rs];

				break;
			//xwr E R
			case 0x0B: 
				if(debug)
				{	
					printf("xwr %.2s, R%d\n", xwr_xr, rs); 
					
					//wrx IP see labels
					if(0 == rf)
					{
						auto const it = symbols.find(static_cast<uint16_t>(regs[rs]));
						if(symbols.end() == it)
							printf("IP <-- %d (= 0x%04X)\n", regs[rs], regs[rs]);
						else
							printf("IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
					}
					else
						printf("%.2s <-- %5d (= 0x%04X)\n", xwr_xr, regs[rs], regs[rs]);
				}


				if(rf == 1)
				{
					extreg_UI   = regs[rs];
					ui_changed  = true;
				}
				else
					extregs[rf] = regs[rs];

				break;
			//mrd R R
			case 0x0C: 
				if(debug) 
				{
					printf("mrd R%d, R%d\n", rf, rs); 
					printf("R%d <-- M[0x%04X] (= %5d 0x%04X)\n", rf, regs[rs], memory[regs[rs]], memory[regs[rs]]);
				}

				regs[rf] = memory[regs[rs]];

				break;
			//mwr R R
			case 0x0D: 
				if(debug) 
				{
					printf("mwr R%d, R%d\n", rs, rf);
					printf("M[0x%04X] <-- %5d (= 0x%04X)\n", regs[rs], regs[rf], regs[rf]);
				}

				memory[regs[rf]] = regs[rs];

				break;
			//mro R R 
			case 0x0E: 
				if(debug) 
				{
					printf("mro R%d, R%d\n", rs, rf);
					printf("M[0x%04X] <-- %5d (= 0x%04X)\n", regs[rs] + extreg_OF, regs[rf], regs[rf]);
				}

				regs[rf] = memory[regs[rs] + extreg_OF];

				break;
			//mwo R R 
			case 0x0F: 
				if(debug)
				{	
					printf("mwo R%d, R%d\n", rf, rs);
					printf("R%d <-- M[0x%04X] (= %5d 0x%04X)\n", rf, regs[rs] + extreg_OF, memory[regs[rs] + extreg_OF], memory[regs[rs]] + extreg_OF);
				}

				memory[regs[rf] + extreg_OF] = regs[rs];

				break;
			//mul R R 
			case 0x10: 
				if(debug)
				{
					printf("mul R%d, R%d\n", rs, rf);
					printf("R%d <-- %5d * %5d (= %5d)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] * regs[rs]));
				}

				res = regs[rf] = static_cast<unsigned>(regs[rf]) * regs[rs];

				set_flags = true;
				break;
			//cmp R R 
			case 0x11: 
				if(debug)
				{
					printf("cmp R%d, R%d\n", rf, rs);
					printf("%5d - %5d (= %5d)\n", regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] - regs[rs]));
				}

				res = regs[rf] - regs[rs];

				overflow  = (((regs[rf] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
				set_flags = true;
				break;
			//div R R
			case 0x12: 
				if(debug)
				{
					printf("div R%d, R%d\n", rf, rs);
					printf("R%d <-- %5d / %5d (= %5d)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] / regs[rs]));
				}

				res = regs[rf] = regs[rf] * regs[rs];

				set_flags = true;
				break;
			//tst R R
			case 0x13: 
				if(debug)
				{
					printf("tst R%d, R%d\n", rf, rs);
					printf("R%d <-- 0x%04X & 0x%04X (= 0x%04X)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] & regs[rs]));
				}

				res = regs[rf] * regs[rs];

				set_flags = true;
				break;
			//mov R R
			case 0x14: 
				if(debug)
				{
					printf("mov R%d, R%d\n", rf, rs);
					printf("R%d <-- %5d\n", rf, static_cast<uint16_t>(regs[rs]));
				}

				regs[rf] = regs[rs];
				break;
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpedantic"
			case 0x15 ... 0x17: 
			#pragma GCC diagnostic pop
				printf("INVALID INSTRUCTION\n");

				if(exit_on_error)
					goto finish; 
				break;
			//add R R
			case 0x18: 
				if(debug)
				{
					printf("add R%d, R%d\n", rf, rs);
					printf("R%d <-- %5d + %5d (= %5d)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] + regs[rs]));
				}

				res = regs[rf] + regs[rs];
				//must have the same sign which is not the same as output
				overflow  = ((~(regs[rf] ^ regs[rs]) & (regs[rs] ^ res)) >> 15) & 0b1;
				regs[rf] = res;

				set_flags = true;
				break;
			//sub R R
			case 0x19: 
				if(debug)
				{
					printf("sub R%d, R%d\n", rf, rs);
					printf("R%d <-- %5d - %5d (= %5d)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] - regs[rs]));
				}

				res = regs[rf] - regs[rs];
				overflow  = (((regs[rf] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
				regs[rf] = res;

				set_flags = true;
				break;
			//not R R
			case 0x1A: 
				if(debug)
				{
					printf("not R%d, R%d\n", rf, rs);
					printf("R%d <-- ~0x%04X (= 0x%04X)\n", rf, regs[rs], static_cast<uint16_t>(~regs[rs]));
				}

				res = regs[rf] = ~regs[rs];

				set_flags = true;
				break;
			//and R R
			case 0x1B: 
				if(debug)
				{
					printf("and R%d, R%d\n", rf, rs);
					printf("R%d <-- 0x%04X & 0x%04X (= 0x%04X)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] & regs[rs]));
				}

				res = regs[rf] = regs[rf] & regs[rs];

				set_flags = true;
				break;
			//orr R R
			case 0x1C: 
				if(debug)
				{
					printf("orr R%d, R%d\n", rf, rs);
					printf("R%d <-- 0x%04X | 0x%04X (= 0x%04X)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] | regs[rs]));
				}

				res = regs[rf] = regs[rf] | regs[rs];
				
				set_flags = true;
				break;
			//xor R R
			case 0x1D: 
				if(debug)
				{
					printf("xor R%d, R%d\n", rf, rs);
					printf("R%d <-- 0x%04X ^ 0x%04X (= 0x%04X)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] ^ regs[rs]));
				}

				res = regs[rf] = regs[rf] ^ regs[rs];
				
				set_flags = true;
				break;
			//shl R R
			case 0x1E: 
				if(debug)
				{
					printf("shl R%d, R%d\n", rf, rs);
					printf("R%d <-- 0x%04X << 0x%04X (= 0x%04X)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rs] > 15 ? 0 : regs[rf] << regs[rs]));
				}

				res = regs[rf] = regs[rs] > 15 ? 0 : regs[rf] << regs[rs];

				set_flags = true;

				break;
			//shr R R
			case 0x1F: 
				if(debug)
				{
					printf("shr R%d, R%d\n", rf, rs);
					printf("R%d <-- 0x%04X >> 0x%04X (= 0x%04X)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rs] > 15 ? 0 : regs[rf] >> regs[rs]));
				}

				res = regs[rf] = regs[rs] > 15 ? 0 : regs[rf] >> regs[rs];

				set_flags = true;
			}
			break;
		case 0x01:
			switch(opcode_long & 0b111)
			{
			//int I
			case 0b000: 
				if(debug) {printf("int %3d\n", imm8_long);          }

				printf("INT NOT IMPLEMENTED YET\n");

				if(exit_on_error)
					goto finish; 
				break;
			//irt
			case 0b001: 
				if(debug) {printf("irt\n");                        }

				printf("IRT NOT IMPLEMENTED YET\n");

				if(exit_on_error)
					goto finish; 
				break;
			//hlt
			case 0b010: 
				if(debug) {printf("hlt\n");                        }

				printf("HLT NOT IMPLEMENTED YET\n");

				if(exit_on_error)
					goto finish; 
				break;
			//hcf
			case 0b011: 
				if(debug) {printf("hcf\n");                        }

				goto finish;
			case 0b100:
			case 0b101:
				printf("INVALID INSTRUCTION\n");

				if(exit_on_error)
					goto finish; 
				break;
			//pxr R I
			case 0b110: 
				if(debug) 
				{
					printf("pxr R%d, %2d\n", rf, imm5_long); 
					printf("R%d <-- %2d (= 0x%04X)\n", rf, privilegedextregs[imm5_long], privilegedextregs[imm5_long]);
				}

				regs[rf] = privilegedextregs[imm5_long];

				break;
			//pxw I R
			case 0b111: 
				if(debug)
				{
					printf("pxw %d, R%d\n", imm5_long, rf);
					printf("PX[0x%04X] <-- %2d (= 0x%04X)\n", imm5_long, regs[rf], regs[rf]);
				}

				privilegedextregs[imm5_long] = regs[rf];

				break;
			}
			break;
		case 0x06:
		case 0x07:
			printf("PRG NOT IMPLEMENTED YET\n"); 

			if(exit_on_error)
				goto finish; 
			break;
		case 0x15:
			switch(opcode_long & 0b111)
			{
			//pop R
			case 0b000: 
				if(debug) 
				{
					printf("pop R%d\n", rf);
					printf("R%d <-- M[0x%04X] (= %5d 0x%04X)\n", rf, extreg_SP, regs[rf], regs[rf]);
					printf("SP <-- 0x%04X\n", static_cast<uint16_t>(extreg_SP + 1));
				}

				regs[rf] = memory[extreg_SP];
				extreg_SP++;

				break;
			//psh R
			case 0b001: 
				if(debug)
				{
					printf("psh R%d\n", rf);
					printf("SP <-- 0x%04X\n", static_cast<uint16_t>(extreg_SP - 1));
					printf("M[0x%04X] <-- %5d (= 0x%04X)\n", static_cast<uint16_t>(extreg_SP - 1), regs[rf], regs[rf]);
				}

				extreg_SP--;
				memory[extreg_SP] = regs[rf];

				break;
			//cal R
			case 0b010: 
				if(debug) 
				{
					printf("cal R%d\n", rf);
					printf("LR <-- 0x%04X\n", extreg_IP);
					auto const it = symbols.find(static_cast<uint16_t>(extreg_IP + regs[rf]));
					if(symbols.end() == it)
						printf("IP <-- 0x%04X\n", static_cast<uint16_t>(extreg_IP + regs[rf]));
					else
						printf("IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
				}

				extreg_LR = extreg_IP;
				extreg_IP = extreg_IP + regs[rf];
				break;
			//jmp R
			case 0b011: 
				if(debug)
				{
					printf("jmp R%d\n", rf);

					auto const it = symbols.find(static_cast<uint16_t>(extreg_IP + regs[rf]));
					if(symbols.end() == it)
						printf("IP <-- 0x%04X\n", static_cast<uint16_t>(extreg_IP + regs[rf]));
					else
						printf("IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
				}

				extreg_IP = extreg_IP + regs[rf];
				break;
			//ret
			case 0b100: 
				if(debug)
				{
					printf("ret%.5s\n", socz_str);
					printf("FL <-- 0x%04X\n", socz);
					printf("IP <-- 0x%04X\n", extreg_LR);
				}

				extreg_FL = socz;
				extreg_IP = extreg_LR;
			
				break;
			case 0b101:
			case 0b110:
				printf("INVALID INSTRUCTION\n"); 

				if(exit_on_error)
					goto finish; 
				break;
			case 0b111:
				if(debug) {printf("nop\n");                 }

				break;
			}
			break;
		default:
			switch(opcode_short)
			{
			/*0x00 not encodable*/
			/*0x01 not encodable*/
			//cal I
			case 0x02: 
				if(debug) 
				{
					printf("cal %5d\n", imm11);
					printf("LR <-- 0x%04X\n", extreg_IP);

					auto const it = symbols.find(static_cast<uint16_t>(extreg_IP + imm11));
					if(symbols.end() == it)
						printf("IP <-- 0x%04X\n", static_cast<uint16_t>(extreg_IP + imm11));
					else
						printf("IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
				}

				extreg_LR = extreg_IP;
				extreg_IP = extreg_IP + imm11;
				break;
			//jmp I
			case 0x03: 
				if(debug)
				{
					printf("jmp %5d\n", imm11);
					auto const it = symbols.find(static_cast<uint16_t>(extreg_IP + imm11));
					if(symbols.end() == it)
						printf("IP <-- 0x%04X\n", static_cast<uint16_t>(extreg_IP + imm11));
					else
						printf("IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
				}

				extreg_IP = extreg_IP + imm11;
				break;
			//jcu F I
			case 0x04: 
			{
				if(debug)
				{
					printf("jcu %.4s%5d\n", ccc_str, imm8_j);
					auto const it = symbols.find(static_cast<uint16_t>(extreg_IP + imm8_j));
					if(symbols.end() == it)
						printf("condition met => IP <-- 0x%04X\n", static_cast<uint16_t>(extreg_IP + imm8_j));
					else
						printf("condition met => IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
				}
				uint16_t const  jmp = extreg_IP + imm8_j;
				bool jumped = false;

				switch(ccc)
				{
				//     LEG
				case 0b000:
					break;
				case 0b001:
					if(F_CARY == 0 && F_ZERO == 0) {extreg_IP =  jmp;  jumped = true;}
					break;                         
				case 0b010:                        
					if(F_ZERO != 0)                {extreg_IP =  jmp;  jumped = true;}
					break;                         
				case 0b011:                        
					if(F_CARY == 0)                {extreg_IP =  jmp;  jumped = true;}
					break;                         
				case 0b100:                        
					if(F_CARY != 0)                {extreg_IP =  jmp;  jumped = true;}
					break;                         
				case 0b101:                        
					if(F_ZERO == 0)                {extreg_IP =  jmp;  jumped = true;}
					break;                          
				case 0b110:                         
					if(F_CARY != 0 || F_ZERO != 0) {extreg_IP =  jmp;  jumped = true;}
					break;
				case 0b111:
					extreg_IP = jmp;
					break;
				}

				if(debug && jumped)
					printf("jumped\n");
				break;
			}
			//jcs F I
			case 0x05: 
			{
				if(debug)
				{
					printf("jcs %.4s%5d\n", ccc_str, imm8_j);
					auto const it = symbols.find(static_cast<uint16_t>(extreg_IP + imm8_j));
					if(symbols.end() == it)
						printf("condition met => IP <-- 0x%04X\n", static_cast<uint16_t>(extreg_IP + imm8_j));
					else
						printf("condition met => IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
				}
				uint16_t const  jmp = extreg_IP + imm8_j;
				bool jumped = false;

				switch(ccc)
				{
				//     LEG
				case 0b000:
					break;
				case 0b001:
					if(F_SIGN == F_OVER && F_ZERO == 0) {extreg_IP =  jmp;  jumped = true;}
					break;                              
				case 0b010:                             
					if(F_ZERO != 0)                     {extreg_IP =  jmp;  jumped = true;}
					break;                              
				case 0b011:                             
					if(F_SIGN == F_OVER)                {extreg_IP =  jmp;  jumped = true;}
					break;                              
				case 0b100:                             
					if(F_SIGN != F_OVER)                {extreg_IP =  jmp;  jumped = true;}
					break;                              
				case 0b101:                             
					if(F_ZERO == 0)                     {extreg_IP =  jmp;  jumped = true;}
					break;                               
				case 0b110:                              
					if(F_SIGN != F_OVER || F_ZERO != 0) {extreg_IP =  jmp;  jumped = true;}
					break;
				case 0b111:
					extreg_IP = jmp;
					break;
				}

				if(debug && jumped)
					printf("jumped\n");
				break;
			}
			//prd R I
			case 0x08: 
				if(debug) 
				{
					printf("prd R%d, %5d\n", rf, imm8);
					printf("R%d <-- %d (= 0x%04X)\n", rf, ports[imm8], ports[imm8]);
				}

				regs[rf] = ports[imm8];

				break;
			//pwr I R
			case 0x09: 
				if(debug)
				{
					printf("pwr %5d, R%d\n", imm8, rf);
					printf("P[0x%04X] <-- %d (= 0x%04X)\n", ports[imm8], regs[rf], regs[rf]);
				}

				ports[imm8] = regs[rf];

				break;
			case 0x0A:
				printf("INVALID INSTRUCTION\n");

				if(exit_on_error)
					goto finish; 
				break;
			//xwr E I
			case 0x0B: 
				if(debug)
				{
					printf("xwr %.2s, %5d\n", xwr_xr, imm8);
					//wrx IP see labels
					if(0 == rf)
					{
						auto const it = symbols.find(static_cast<uint16_t>(imm8));
						if(symbols.end() == it)
							printf("IP <-- %d (= 0x%04X)\n", imm8, imm8);
						else
							printf("IP <-- %s (= 0x%04X)\n", it->second.c_str(), it->first);
					}
					else
						printf("%.2s <-- %5d (= 0x%04X)\n", xwr_xr, regs[rs], regs[rs]);
				}

				if(rf == 1)
				{
					extreg_UI   = imm8;
					ui_changed  = true;
				}
				else
					extregs[rf] = imm8;

				break;
			//mrd R I
			case 0x0C: 
				if(debug) 
				{
					printf("mrd R%d, %5d\n", rf, imm8);
					printf("R%d <-- M[0x%04X] (= %5d 0x%04X)\n", rf, imm8, memory[imm8], memory[imm8]);
				}

				regs[rf] = memory[imm8];

				break;
			//mwr I R
			case 0x0D: 
				if(debug) 
				{
					printf("mwr %5d, R%d\n", imm8, rf);
					printf("M[0x%04X] <-- %5d (= 0x%04X)\n", imm8, regs[rf], regs[rf]);
				}

				memory[imm8] = regs[rf];

				break;
			//mro R I
			case 0x0E: 
				if(debug) 
				{
					printf("mro R%d, %5d\n", rf, imm8);
					printf("R%d <-- M[0x%04X] (= %5d 0x%04X)\n", rf, imm8 + extreg_OF, memory[imm8 + extreg_OF], memory[imm8 + extreg_OF]);
				}

				regs[rf] = memory[imm8 + extreg_OF];

				break;
			//mwo I R
			case 0x0F: 
				if(debug) 
				{
					printf("mwo %d, R%5d\n", imm8, rf);
					printf("R%d <-- M[0x%04X] (= %5d 0x%04X)\n", rf, imm8 + extreg_OF, memory[imm8 + extreg_OF], memory[imm8] + extreg_OF);
				}

				memory[imm8 + extreg_OF] = regs[rf];

				break;
			//mul R I 
			case 0x10: 
				if(debug)
				{
					printf("mul R%d, %5d\n", rf, imm8);
					printf("R%d <-- %5d * %5d (= %5d)\n", rf, regs[rf], regs[rs], static_cast<uint16_t>(regs[rf] * regs[rs]));
				}

				res = regs[rf] = static_cast<unsigned>(regs[rf]) * imm8;

				set_flags = true;
				break;
			//cmp R I 
			case 0x11: 
				if(debug) 
				{
					printf("cmp R%d, %5d\n", rf, imm8);
					printf("%5d - %5d (= %5d)\n", regs[rf], imm8, static_cast<uint16_t>(regs[rf] - imm8));
				}

				res = regs[rf] - imm8;

				overflow  = (((regs[rf] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
				set_flags = true;
				break;
			//div R I
			case 0x12: 
				if(debug) 
				{
					printf("div R%d, %5d\n", rf, imm8);
					printf("R%d <-- %5d / %5d (= %5d)\n", rf, regs[rf], imm8, static_cast<uint16_t>(regs[rf] / imm8));
				}

				res = regs[rf] = regs[rf] / imm8;
				
				set_flags = true;
				break;
			//tst R I
			case 0x13: 
				if(debug) 
				{
					printf("tst R%d, %5d\n", rf, imm8);
					printf("0x%04X & 0x%04X (= 0x%04X)\n", regs[rf], imm8, static_cast<uint16_t>(regs[rf] & imm8));
				}

				res = regs[rf] = regs[rf] & imm8;
				
				set_flags = true;
				break;
			//mov R I
			case 0x14: 
				if(debug) 
				{
					printf("mov R%d, %5d\n", rf, imm8);
					printf("R%d <-- %5d \n", rf, imm8);
				}

				regs[rf] = imm8;

				break;
			/*0x15 not encodable*/
			//mcu F R R
			case 0x16: 
			{
				if(debug) 
				{
					printf("mcu %.4sR%d, R%d\n", ccc_str, rf, rs);
					printf("condition met => R%d <-- %d (= 0x%04X)\n", rf, regs[rs], regs[rs]);
				}
				bool moved = true;

				switch(ccc)
				{
				//     LEG
				case 0b000:
					break;
				case 0b001:
					if(F_CARY == 0 && F_ZERO == 0) {regs[rf] = regs[rs]; moved = true;}
					break;           
				case 0b010:          
					if(F_ZERO != 0)                {regs[rf] = regs[rs]; moved = true;}
					break;          
				case 0b011:        
					if(F_CARY == 0)                {regs[rf] = regs[rs]; moved = true;}
					break;        
				case 0b100:      
					if(F_CARY != 0)                {regs[rf] = regs[rs]; moved = true;}
					break;        
				case 0b101:       
					if(F_ZERO == 0)                {regs[rf] = regs[rs]; moved = true;}
					break;        
				case 0b110:       
					if(F_CARY != 0 || F_ZERO != 0) {regs[rf] = regs[rs]; moved = true;}
					break;       
				case 0b111:      
					                               {regs[rf] = regs[rs]; moved = true;}
					break;
				}
				if(debug && moved)
					printf("moved\n");
			}

				break;
			//mcs F R R
			case 0x17: 
			{
				if(debug) 
				{
					printf("mcs %.4sR%d, R%d\n", ccc_str, rf, rs); 
					printf("condition met => R%d <-- %d (= 0x%04X)\n", rf, regs[rs], regs[rs]);
				}
				bool moved = true;

				switch(ccc)
				{
				//     LEG
				case 0b000:
					break;
				case 0b001:
					if(F_SIGN == F_OVER && F_ZERO == 0) {regs[rf] = regs[rs]; moved = true;}
					break;           
				case 0b010:          
					if(F_ZERO != 0)                     {regs[rf] = regs[rs]; moved = true;}
					break;          
				case 0b011:        
					if(F_SIGN == F_OVER)                {regs[rf] = regs[rs]; moved = true;}
					break;        
				case 0b100:      
					if(F_SIGN != F_OVER)                {regs[rf] = regs[rs]; moved = true;}
					break;        
				case 0b101:       
					if(F_ZERO == 0)                     {regs[rf] = regs[rs]; moved = true;}
					break;        
				case 0b110:       
					if(F_SIGN != F_OVER || F_ZERO != 0) {regs[rf] = regs[rs]; moved = true;}
					break;       
				case 0b111:      
					                                    {regs[rf] = regs[rs]; moved = true;}
					break;
				}

				if(debug && moved)
					printf("moved\n");
			}

				break;
			//add R I
			case 0x18: 
				if(debug) 
				{
					printf("add R%d, %5d\n", rf, imm8);
					printf("R%d <-- %5d + %5d (= %5d)\n", rf, regs[rf], imm8, static_cast<uint16_t>(regs[rf] + imm8));
				}

				res = regs[rf] + imm8;
				//must have the same sign which is not the same as output
				overflow  = ((~(regs[rf] ^ regs[rs]) & (regs[rs] ^ res)) >> 15) & 0b1;
				regs[rf] = res;

				set_flags = true;
				break;
			//sub R I
			case 0x19: 
				if(debug) 
				{
					printf("sub R%d, %5d\n", rf, imm8);
					printf("R%d <-- %5d - %5d (= %5d)\n", rf, regs[rf], imm8, static_cast<uint16_t>(regs[rf] - imm8));
				}

				res = regs[rf] - imm8;
				overflow  = (((regs[rf] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
				regs[rf] = res;

				set_flags = true;
				break;
			//not R I
			case 0x1A: 
				if(debug) 
				{
					printf("not R%d, %5d\n", rf, imm8);
					printf("R%d <-- ~0x%04X (= 0x%04X)\n", rf, imm8, static_cast<uint16_t>(~imm8));
				}

				res = regs[rf] = ~imm8;

				set_flags = true;
				break;
			//and R I
			case 0x1B: 
				if(debug) 
				{
					printf("and R%d, %5d\n", rf, imm8);
					printf("R%d <-- 0x%04X & 0x%04X (= 0x%04X)\n", rf, regs[rf], imm8, static_cast<uint16_t>(regs[rf] & imm8));
				}

				res = regs[rf] = regs[rf] & imm8;

				set_flags = true;
				break;
			//orr R I
			case 0x1C: 
				if(debug) 
				{
					printf("orr R%d, %5d\n", rf, imm8);
					printf("R%d <-- 0x%04X | 0x%04X (= 0x%04X)\n", rf, regs[rf], imm8, static_cast<uint16_t>(regs[rf] | imm8));
				}

				res = regs[rf] = regs[rf] | imm8;
				
				set_flags = true;
				break;
			//xor R I
			case 0x1D: 
				if(debug) 
				{
					printf("xor R%d, %5d\n", rf, imm8);
					printf("R%d <-- 0x%04X ^ 0x%04X (= 0x%04X)\n", rf, regs[rf], imm8, static_cast<uint16_t>(regs[rf] ^ imm8));
				}

				res = regs[rf] = regs[rf] ^ imm8;

				set_flags = true;
				break;
			//shl R I
			case 0x1E: 
				if(debug) 
				{
					printf("shl R%d, %5d\n", rf, imm8);
					printf("R%d <-- 0x%04X << 0x%04X (= 0x%04X)\n", rf, regs[rf], imm8, static_cast<uint16_t>(imm8 > 15 ? 0  : regs[rf] << imm8));
				}

				res = regs[rf] = imm8 > 15 ? 0  : regs[rf] << imm8;

				set_flags = true;
				break;
			//shr R I
			case 0x1F: 
				if(debug) 
				{
					printf("shr R%d, %5d\n", rf, imm8);
					printf("R%d <-- 0x%04X >> 0x%04X (= 0x%04X)\n", rf, regs[rf], imm8, static_cast<uint16_t>(imm8 > 15 ? 0  : regs[rf] >> imm8));
				}

				res = regs[rf] = imm8 > 15 ? 0  : regs[rf] >> imm8;

				set_flags = true;
				break;
			break;
			}
		}


		if(set_flags)
		{
			int const sign = (res >> 15) & 0b1;
			int const cary = (res >> 16) & 0b1;
			int const zero = (res ==  0) & 0b1;

			extreg_FL = (sign << 3)
					  | (overflow << 2)
					  | (cary << 1)
					  | (zero << 0);

			//surpress warning about format
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wformat"
			if(debug) printf("flags set to %04b\n", extreg_FL);
			#pragma GCC diagnostic pop
		}
		

		continue;
	}

finish:
	if(print)
	{
		std::cout << "registers:\n";
		for(int i = 0; i < 8; i++)
			std::cout << "R" << i << " " << regs[i] << '\n';
	}
	int const memdump_fd = open("dump.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);							
	char to_write[6] = "    \n";
	for(int i = 0; i < (1 << 16); i++)
	{
		snprintf(to_write, 5, "%04X", memory[i]);
		to_write[4] = '\n';
		write(memdump_fd, to_write, 5);
	}

	close(memdump_fd);

	return 0;
}
