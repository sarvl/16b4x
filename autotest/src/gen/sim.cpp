#include <cstdio>

#include <cstdlib> //system
#include <cstring> //memcmp

#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../../../utility/src/log.h"

#include "../global.h"

int gen_sim()
{
	//none of these are cleared, they live until the process dies with no consequences
	int const         fd   = open("./tests/cpu/test_groups.txt", O_RDONLY);
	int const         size = lseek(fd, 0, SEEK_END);
	char const* const data = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	//remove existing ones
	system("rm -f tests/cpu/bin/*");
	system("rm -f tests/cpu/out/*");

	//assumes that files are correct
	int off = 0;
	while(off < size)
	{
		std::string command0 = "./sasm ./tests/cpu/asm/g00t00.sasm -o prog.bin";
		std::string command1 = "./cosi prog.bin ";
		std::string command2 = "mv -f prog.bin ./tests/cpu/bin/g00t00.bin";
		std::string command3 = "mv -f dump.txt ./tests/cpu/out/g00t00.out";
		
		//read all args
		while('\n' != data[off])
		{
			command1 += data[off];
			off++;
		}
		off++;

		int const beg = off;
		off += 12;

		if(print_group) printf("\033[1;38;5;55m%.5s\033[0m\n", data + beg + 4);

		command0[24] = data[beg + 1];
		command0[25] = data[beg + 2];
		command2[32] = data[beg + 1];
		command2[33] = data[beg + 2];
		command3[32] = data[beg + 1];
		command3[33] = data[beg + 2];

		int test_num = 0;
		int const test_limit = (data[beg +  9] - '0') * 10
		                     + (data[beg + 10] - '0');
							 
		count_total += test_limit;

		while(test_num < test_limit)
		{
			command0[27] = test_num / 10 + '0';
			command0[28] = test_num % 10 + '0';
			command2[35] = test_num / 10 + '0';
			command2[36] = test_num % 10 + '0';
			command3[35] = test_num / 10 + '0';
			command3[36] = test_num % 10 + '0';

			system(command0.c_str());
			system(command1.c_str());
			system(command2.c_str());
			system(command3.c_str());

			test_num++;
		}
	}

	return 0;
}
