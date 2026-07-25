#include <cstdio>
#include <iostream>
#include <fstream>

#include "state.h"
#include "internal_state.h"

#include "instruction.h"
#include "io.h"
#include "interrupt.h"
#include "memory.h"
#include "helper.h"

using namespace std::literals;

uint16_t regs[8];
uint16_t ip;
uint16_t lc;
uint16_t ui;
uint16_t sp;
uint16_t ar;

uint16_t fl_s, fl_o, fl_c, fl_z, fl_d;

bool halt              = false;
bool io_enabled        = false;
bool ui_modified       = false;

bool interrupt_pgf   = false;
bool interrupt_prv   = false;
bool interrupt_pev   = false;
bool interrupt_ins   = false;
bool interrupt_ina   = false;
bool interrupt_tmr   = false;
bool interrupt_div   = false;
bool interrupt_io    = false;
bool interrupt_sw    = false;
int  interrupt_sw_id = 0;

int main(int const argc, char* argv[])
{
	//currently args are very limited, just make sure the file reading works 
	const char* filename = (2 <= argc) ? argv[1] : "out.bin";
	bool debug           = (3 <= argc);

	internal::reset();
	mem::reset();

	if(int err = io::reset(filename); err != 0)
	{
		printf("ERROR: something is wrong with the file, go fix\nerror code: %d\n", err);
		return 1;
	}
	
	int max_instr = 20000000;
	int instr_cnt = 0;

	const int brkpointbase = 0x804C;
	int brkpoint = brkpointbase;

	while(true)
	{
		if(debug)
		{
			printf("current IP   : x%04X\n", ip);
			printf("current instr: x%04X\n", mem::read_bypass(ip));
		}

		instruction_execute();
		instr_cnt++;

		if(internal::flush_tlb)
		{
			mem::flush_tlb();
			internal::flush_tlb = false;
		}
		if(internal::iht_reload)
		{
			interrupt::load_iht();
			internal::iht_reload = false;
		}

		if(halt)
		{
			if(not internal::interrupts_enabled)
				break;	
			
			printf("ERROR: halting not yet implemented");
			exit(69);
		}

		if(instr_cnt > max_instr)
		{
			printf("ERROR: too many instructions\n");
			printf("current IP   : x%04X\n", ip);
			printf("current instr: x%04X\n", mem::read_bypass(ip));
			break;
		}

//		timer_update();

//		io_update();

//		interrupt_handle();
		if(interrupt_sw)
		{
			interrupt_sw = false;

			cr::write_bypass(cr::interrupt_id, interrupt_sw_id);
			cr::write_bypass(cr::interrupt_saved_ip, ip);
			cr::write_bypass(cr::interrupt_saved_ui, ui);
			cr::write_bypass(cr::interrupt_saved_fl, register_fl_read());
			cr::write_bypass(cr::interrupt_saved_cr0002, cr::read_bypass(cr::privilege_l_0));
			cr::write_bypass(cr::interrupt_saved_cr0003, cr::read_bypass(cr::privilege_l_1));
			cr::write_bypass(cr::privilege_l_0, 0xFF);
			cr::write_bypass(cr::privilege_l_1, 0xFF);

			internal::interrupts_enabled = false;
			ip = interrupt::get_iht_entry(interrupt_sw_id);

			printf("switching ip to %04X\n", ip);
		}


		if(brkpoint == ip)
			brkpoint = -1;

		if(debug && brkpoint == -1)
		{
				
			std::string cmd;
			std::getline(std::cin, cmd);


			if("quit"s == cmd)
				break;
			if("reg"s == cmd)
			{
				printf("Registers: \n");
				for(int i = 0; i < 8; i++)
					printf("  R%d: x%04X\n", i, regs[i]);
				printf("Auxiliary Registers: \n");
					printf(" IP: x%04X\n", ip);
					printf(" LC: x%04X\n", lc);
					printf(" UI: x%04X\n", ui);
					printf(" SP: x%04X\n", sp);
					printf(" AR: x%04X\n", ar);
					printf(" FL: x%04X\n", register_fl_read());
			}
			if("dump"s == cmd)
			{
				FILE* f = fopen("dump.txt", "w");
				for(int i = 0; i < 65536; i++)
					fprintf(f, "%04X\n", mem::read_pm(i));
				fclose(f);
			}
			if("brk"s == cmd)
			{
				brkpoint = brkpointbase;
			}
		}
	}


	//final state print, in the future add more options
	
	if(debug)
	{
		printf("Registers: \n");
		for(int i = 0; i < 8; i++)
			printf("  R%d: x%04X\n", i, regs[i]);
		printf("Auxiliary Registers: \n");
			printf(" IP: x%04X\n", ip);
			printf(" LC: x%04X\n", lc);
			printf(" UI: x%04X\n", ui);
			printf(" SP: x%04X\n", sp);
			printf(" AR: x%04X\n", ar);
			printf(" FL: x%04X\n", register_fl_read());
	}

	//memdump 
	FILE* f = fopen("dump.txt", "w");
	for(int i = 0; i < 65536; i++)
		fprintf(f, "%04X\n", mem::read_pm(i));
	fclose(f);

	return 0;
}
