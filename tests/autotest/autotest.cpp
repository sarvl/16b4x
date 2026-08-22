#include <cstdio>

#include <string>

#include <cstdlib> //system
#include <cstring> //memcmp

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <chrono>

#include "../../utility/src/log.h"

int main(int argc, char* argv[])
{
	bool print_help           = false;
	bool print_summary        = false;
	bool print_summary_pretty = false;
	bool print_group          = false;
	bool print_failed         = false;
	bool print_passed         = false;

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
		}
	}

	if(print_help)
	{
		printf(
			"fost - forensic tester\n"
			"by default, nothing is printed, only return code is set\n"
			"\n"
			"\t-h - print this help\n"
			"\t-s - print summary\n"
			"\t-S - print summary pretty, overwrites -s\n"
			"\t-g - print group under test\n"
			"\t-f - print failing tests\n"
			"\t-p - print passing tests\n"
			);
		return 0;
	}

	auto const time0 = std::chrono::high_resolution_clock::now();

	//none of these are cleared, they live until the process dies with no consequences
	int const         filelist_fd   = open("./tests/test_groups.txt", O_RDONLY);
	int const         filelist_size = lseek(filelist_fd, 0, SEEK_END);
	char const* const filelist_data = (char*)mmap(NULL, filelist_size, PROT_READ, MAP_SHARED, filelist_fd, 0);

	char command[]  = "./cosi ./tests/bin/g00t00.bin\0";
	char filename[] = "./tests/out/g00t00.out\0";

	int count_passed = 0;
	int count_total  = 0;

	//assumes that files are correct
	int test_group = 0;
	while(test_group * 10 < filelist_size)
	{
		char const* const cur_data = filelist_data + test_group * 10;
		if(print_group) printf("\033[1;38;5;55m%.7s\033[0m\n", cur_data);

		command [20] = cur_data[1];
		command [21] = cur_data[2];
		filename[13] = cur_data[1];
		filename[14] = cur_data[2];

		int const group_num  = (cur_data[1] - '0') * 10
		                     + (cur_data[2] - '0')
		                     ;

		int test_num = 0;
		int const test_limit = (cur_data[7] - '0') * 10
		                     + (cur_data[8] - '0')
		                     ;
							 
		count_total += test_limit;

		while(test_num < test_limit)
		{
			command [23] = test_num / 10 + '0';
			command [24] = test_num % 10 + '0';
			filename[16] = test_num / 10 + '0';
			filename[17] = test_num % 10 + '0';

			system(command);

			int f_dump_fd = open("./dump.txt", O_RDONLY);
			int f_corr_fd = open(filename    , O_RDONLY);

			if(-1 == f_dump_fd
			or -1 == f_corr_fd) 
			{
				char err[] = "File error at file g00t00";
				err[20] = group_num / 10 + '0';
				err[21] = group_num % 10 + '0';
				err[23] = test_num  / 10 + '0';
				err[24] = test_num  % 10 + '0';

				Log::error(err);
				goto fail;
			}

			int const f_dump_size = lseek(f_dump_fd, 0, SEEK_END);
			int const f_corr_size = lseek(f_corr_fd, 0, SEEK_END);

			char* f_dump_data = (char*)mmap(NULL, f_dump_size, PROT_READ, MAP_SHARED, f_dump_fd, 0);
			char* f_corr_data = (char*)mmap(NULL, f_corr_size, PROT_READ, MAP_SHARED, f_corr_fd, 0);

			close(f_dump_fd);
			close(f_corr_fd);

			if(f_corr_size != f_dump_size
			or 0 != memcmp(f_corr_data, f_dump_data, f_dump_size))
			{
				if(print_failed) printf("\033[1;38;5;1m[X] g%02dt%02ds\n", group_num, test_num);
			}
			else
			{
				if(print_passed) printf("\033[1;38;5;46m[V] g%02dt%02ds\n", test_group, test_num);
				count_passed++;
			}

			system("rm -f dump.txt");

			test_num++;
		}

		test_group++;
	}

	//so goto doesnt complain
	{

	auto const time1 = std::chrono::high_resolution_clock::now();

	system("rm -f dump.txt");

	if(print_summary_pretty)
	{
		int const count_failed = count_total - count_passed;

		printf("simulator test complete\n");
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

	//1 on error
	return count_passed != count_total;

	}
fail:
	system("rm -f dump.txt");
	return 2;
}
