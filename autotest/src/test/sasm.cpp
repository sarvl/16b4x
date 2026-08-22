#include <cstdio>

#include <cstdlib> //system
#include <cstring> //memcmp

#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../../../utility/src/log.h"

#include "../global.h"

int test_sasm()
{
	//none of these are cleared, they live until the process dies with no consequences
	int const         fd   = open("./tests/sasm/test_groups.txt", O_RDONLY);
	int const         size = lseek(fd, 0, SEEK_END);
	char const* const data = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	//assumes that files are correct
	int off = 0;
	while(off < size)
	{
		std::string command = "./sasm ./tests/sasm/code/g00t00.sasm -o fout.out ";
		std::string correct_fout = "./tests/sasm/fout/g00t00.txt";
		std::string correct_sout = "./tests/sasm/sout/g00t00.txt";

		//read all args
		while('\n' != data[off])
		{
			command += data[off];
			off++;
		}
		off++;
		command += " > sout.out\n";

		int const beg = off;
		off += 12;

		if(print_group) printf("\033[1;38;5;55m%.5s\033[0m\n", data + beg + 4);

		command     [26] = data[beg + 1];
		command     [27] = data[beg + 2];
		correct_fout[19] = data[beg + 1];
		correct_fout[20] = data[beg + 2];
		correct_sout[19] = data[beg + 1];
		correct_sout[20] = data[beg + 2];

		int const group_num  = (data[beg +  1] - '0') * 10
		                     + (data[beg +  2] - '0');
		int test_num = 0;
		int const test_limit = (data[beg +  9] - '0') * 10
		                     + (data[beg + 10] - '0');
							 
		//two tests each
		count_total += test_limit * 2;

		while(test_num < test_limit)
		{
			command[29]      = test_num / 10 + '0';
			command[30]      = test_num % 10 + '0';
			correct_fout[22] = test_num / 10 + '0';
			correct_fout[23] = test_num % 10 + '0';
			correct_sout[22] = test_num / 10 + '0';
			correct_sout[23] = test_num % 10 + '0';

			//remove stale
			system("rm -f fout.out sout.out");

			system(command.c_str());

			//but something must exist
			system("touch ./fout.out ./sout.out");

			int f_fout_fd     = open("./fout.out",         O_RDONLY);
			int f_sout_fd     = open("./sout.out",         O_RDONLY);
			int f_corrfout_fd = open(correct_fout.c_str(), O_RDONLY);
			int f_corrsout_fd = open(correct_sout.c_str(), O_RDONLY);

			if(-1 == f_fout_fd
			or -1 == f_sout_fd 
			or -1 == f_corrfout_fd 
			or -1 == f_corrsout_fd)
			{
				char err[] = "File error at file g00t00";
				err[20] = group_num / 10 + '0';
				err[21] = group_num % 10 + '0';
				err[23] = test_num  / 10 + '0';
				err[24] = test_num  % 10 + '0';

				Log::error(err);
				return 2;
			}

			int const f_fout_size     = lseek(f_fout_fd,     0, SEEK_END);
			int const f_sout_size     = lseek(f_sout_fd,     0, SEEK_END);
			int const f_corrfout_size = lseek(f_corrfout_fd, 0, SEEK_END);
			int const f_corrsout_size = lseek(f_corrsout_fd, 0, SEEK_END);

			char* f_fout_data     = (char*)mmap(NULL, f_fout_size,     PROT_READ, MAP_SHARED, f_fout_fd,     0);
			char* f_sout_data     = (char*)mmap(NULL, f_sout_size,     PROT_READ, MAP_SHARED, f_sout_fd,     0);
			char* f_corrfout_data = (char*)mmap(NULL, f_corrfout_size, PROT_READ, MAP_SHARED, f_corrfout_fd, 0);
			char* f_corrsout_data = (char*)mmap(NULL, f_corrsout_size, PROT_READ, MAP_SHARED, f_corrsout_fd, 0);

			close(f_fout_fd    );
			close(f_sout_fd    );
			close(f_corrfout_fd);
			close(f_corrsout_fd);

			if(f_fout_size != f_corrfout_size
			or 0 != memcmp(f_corrfout_data, f_fout_data, f_fout_size))
			{
				if(print_failed) printf("\033[1;38;5;1m[X] g%02dt%02df\n", group_num, test_num);
			}
			else
			{
				if(print_passed) printf("\033[1;38;5;46m[V] g%02dt%02df\n", group_num, test_num);
				count_passed++;
			}

			if(f_sout_size != f_corrsout_size
			or 0 != memcmp(f_corrsout_data, f_sout_data, f_sout_size))
			{
				if(print_failed) printf("\033[1;38;5;1m[X] g%02dt%02ds\n", group_num, test_num);
			}
			else
			{
				if(print_passed) printf("\033[1;38;5;46m[V] g%02dt%02ds\n", group_num, test_num);
				count_passed++;
			}

			test_num++;
		}
	}

	system("rm -f fout.out sout.out");
	//1 on error
	return count_passed != count_total;
}
