#include <cstdio>

#include <cstdlib> //system
#include <cstring> //memcmp, strcmp

#include <chrono>

#include "global.h"

bool print_help           = false;
bool print_summary        = false;
bool print_summary_pretty = false;
bool print_group          = false;
bool print_failed         = false;
bool print_passed         = false;

int  count_passed = 0;
int  count_total  = 0;

int main(int argc, char* argv[])
{
	bool generate = false;
	{
		char* ptr  = argv[0];
		char* find = argv[0];
		while(*find != '\0')
		{
			if(*find == '/')
				ptr = find + 1;

			find++;
		}

		generate = !strcmp("fgen", ptr);
	}

	for(int ii = 1; ii < argc; ii++)
	{
		char const* const arg = argv[ii];

		if(arg[0] != '-')
			continue;
		
		int const len = strlen(arg);
		for(int jj = 1; jj < len; jj++) switch(arg[jj])
		{
		case 'h': print_help           = true; break;
		case 's': print_summary        = true; break;
		case 'S': print_summary_pretty = true; break;
		case 'g': print_group          = true; break;
		case 'f': print_failed         = true; break;
		case 'p': print_passed         = true; break;
		case 'a': 
			print_summary_pretty = true;
			print_failed         = true;
			break;
		case 'A': 
			print_summary_pretty = true;
			print_group          = true;
			print_failed         = true;
			print_passed         = true;
			break;
		}
	}

	if(print_help)
	{
		if(generate)
			printf(
			"fgen - fast generator\n"
			"usage: fgen target [args]\n"
			"\t target is one of {sim, sasm}\n"
			"\n"
			"by default, nothing is printed\n"
			"\n"
			"\t-h - print this help\n"
			"\t-a - equivalent to -S\n"
			"\t-A - print everything\n"
			"\t-s - print summary\n"
			"\t-S - print summary pretty, overwrites -s\n"
			"\t-g - print group under test\n"
			);
		else
			printf(
			"fost - forensic tester\n"
			"usage: fost target [args]\n"
			"\t target is one of {sim, sasm}\n"
			"\n"
			"by default, nothing is printed, only return code is set\n"
			"\n"
			"\t-h - print this help\n"
			"\t-a - equivalent to -fS\n"
			"\t-A - print everything\n"
			"\t-s - print summary\n"
			"\t-S - print summary pretty, overwrites -s\n"
			"\t-g - print group under test\n"
			"\t-f - print failing tests\n"
			"\t-p - print passing tests\n"
			);
		return 0;
	}

	if(generate)
	{
		auto const time0 = std::chrono::high_resolution_clock::now();

			 if(argc < 2) 
		{
			printf("invalid target, see -h\n"); 
			return 10;
		}
		else if(0 == strcmp(argv[1], "sim" )) gen_sim();
		else if(0 == strcmp(argv[1], "sasm")) gen_sasm();
		else 
		{
			printf("invalid target, see -h\n"); 
			return 10;
		}

		auto const time1 = std::chrono::high_resolution_clock::now();

		if(print_summary_pretty)
		{
			printf("gen %s complete\n", argv[1]);
			printf("\033[0;36mtotal  tests : %d\033[0m\n",   count_total);
			printf("\033[0;33mtime taken   : %ldms\033[0m\n", std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count());
		}
		else if(print_summary)
		{
			printf("\033[0;36mgenerated %d\n\033[0m", count_total);
		}

		return 0;
	}
	else
	{
		auto const time0 = std::chrono::high_resolution_clock::now();

		int retcode = 1;
			 if(argc < 2) 
		{
			printf("invalid target, see -h\n"); 
			return 10;
		}
		else if(0 == strcmp(argv[1], "sim" )) retcode = test_sim();
		else if(0 == strcmp(argv[1], "sasm")) retcode = test_sasm();
		else 
		{
			printf("invalid target, see -h\n"); 
			return 10;
		}

		auto const time1 = std::chrono::high_resolution_clock::now();

		if(print_summary_pretty)
		{
			int const count_failed = count_total - count_passed;

			printf("test %s complete\n", argv[1]);
			printf("\033[0;36mtotal  tests : %d\033[0m\n",   count_total);

			if(count_failed > 0)	
				printf("\033[0;31mfailed tests : %d\033[0m\n", count_failed);
			else
				printf("\033[0;32mfailed tests : %d\033[0m\n", count_failed);

			printf("\033[0;33mtime taken   : %ldms\033[0m\n", std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count());
		}
		else if(print_summary)
		{
			printf("\033[0;36mtotal %d/%d\n\033[0m", count_passed, count_total);
		}

		return retcode;
	}
}
