#include <cstdio>

#include <cstdlib> //system
#include <cstring> //memcmp

#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../../../utility/src/log.h"

#include "../global.h"

int gen_sasm()
{
	//none of these are cleared, they live until the process dies with no consequences
	int const         fd   = open("./tests/sasm/test_groups.txt", O_RDONLY);
	int const         size = lseek(fd, 0, SEEK_END);
	char const* const data = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	//remove existing ones
	system("rm -f tests/sasm/fout/*");
	system("rm -f tests/sasm/sout/*");

	//assumes that files are correct
	int off = 0;
	while(off < size)
	{
		std::string command0 = "./sasm ./tests/sasm/code/g00t00.sasm -o fout.out ";
		std::string command1 = "mv -f sout.out ./tests/sasm/sout/g00t00.txt";
		std::string command2 = "mv -f fout.out ./tests/sasm/fout/g00t00.txt";
		
		//read all args
		while('\n' != data[off])
		{
			command0 += data[off];
			off++;
		}
		off++;
		command0 += " > sout.out";

		int const beg = off;
		off += 12;

		if(print_group) printf("\033[1;38;5;55m%.5s\033[0m\n", data + beg + 4);

		command0[26] = data[beg + 1];
		command0[27] = data[beg + 2];
		command1[34] = data[beg + 1];
		command1[35] = data[beg + 2];
		command2[34] = data[beg + 1];
		command2[35] = data[beg + 2];

		int test_num = 0;
		int const test_limit = (data[beg +  9] - '0') * 10
		                     + (data[beg + 10] - '0');
							 
		count_total += test_limit;

		while(test_num < test_limit)
		{
			command0[29] = test_num / 10 + '0';
			command0[30] = test_num % 10 + '0';
			command1[37] = test_num / 10 + '0';
			command1[38] = test_num % 10 + '0';
			command2[37] = test_num / 10 + '0';
			command2[38] = test_num % 10 + '0';

			system(command0.c_str());
			system("touch sout.out fout.out");

			system(command1.c_str());
			system(command2.c_str());

			test_num++;
		}
	}
	return 0;
}
