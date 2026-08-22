#include <cstdio>

#include <cstdlib> //system
#include <cstring> //memcmp

#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../../../utility/src/log.h"

#include "../global.h"

int test_sim()
{
	//none of these are cleared, they live until the process dies with no consequences
	int const         fd   = open("./tests/cpu/test_groups.txt", O_RDONLY);
	int const         size = lseek(fd, 0, SEEK_END);
	char const* const data = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	//assumes that files are correct
	int off = 0;
	while(off < size)
	{
		std::string command = "./cosi ./tests/cpu/bin/g00t00.bin ";
		std::string correct = "./tests/cpu/out/g00t00.out";

		//read all args
		while('\n' != data[off])
		{
			command += data[off];
			off++;
		}
		off++;

		int const beg = off;
		off += 12;

		if(print_group) printf("\033[1;38;5;55m%.5s\033[0m\n", data + beg + 4);

		command[24] = data[beg + 1];
		command[25] = data[beg + 2];
		correct[17] = data[beg + 1];
		correct[18] = data[beg + 2];

		int const group_num  = (data[beg +  1] - '0') * 10
		                     + (data[beg +  2] - '0');
		int test_num = 0;
		int const test_limit = (data[beg +  9] - '0') * 10
		                     + (data[beg + 10] - '0');
							 
		count_total += test_limit;

		while(test_num < test_limit)
		{
			command[27] = test_num / 10 + '0';
			command[28] = test_num % 10 + '0';
			correct[20] = test_num / 10 + '0';
			correct[21] = test_num % 10 + '0';

			//remove stale
			system("rm -f dump.txt");

			system(command.c_str());

			//but something must exist
			system("touch dump.txt");

			int f_dump_fd = open("./dump.txt"   , O_RDONLY);
			int f_corr_fd = open(correct.c_str(), O_RDONLY);

			if(-1 == f_dump_fd
			or -1 == f_corr_fd) 
			{
				char err[] = "File error at file g00t00";
				err[20] = group_num / 10 + '0';
				err[21] = group_num % 10 + '0';
				err[23] = test_num  / 10 + '0';
				err[24] = test_num  % 10 + '0';

				system("rm -f dump.txt");
				Log::error(err);
				return 2;
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
				if(print_failed) printf("\033[1;38;5;1m[X] g%02dt%02d\n", group_num, test_num);
			}
			else
			{
				if(print_passed) printf("\033[1;38;5;46m[V] g%02dt%02d\n", group_num, test_num);
				count_passed++;
			}


			test_num++;
		}
	}

	system("rm -f dump.txt");
	//1 on error
	return count_passed != count_total;
}
