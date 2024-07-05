#include <cstdio>

#include <string>

#include <cstdlib> //system
#include <cstring> //memcmp

#include "../../utility/src/file.h"


int main()
{
	File::t_File file_list;
	if(-1 == File::create_error_handled(&file_list, "./tests/test_groups.txt"))
		return -1;


	std::string commands[] = {
		"./sasm ./tests/asm/g00t00.asm -o ./tests/bin/g00t00.bin\n",
		"./cosi ./tests/bin/g00t00.bin\n",
		"mv dump.txt ./tests/out/g00t00.out\n"
	};

	system("rm -f ./tests/bin/*\n");
	system("rm -f ./tests/out/*\n");
	
	//assumes that files are correct
	int test_group = 0;
	while(test_group * 10 < file_list.size)
	{
		char const* const cur_data = file_list.data + test_group * 10;
		printf("\033[1;38;5;55m%.7s\033[0m\n", cur_data);

		commands[0][20] = test_group / 10 + '0';
		commands[0][21] = test_group % 10 + '0';
		commands[0][46] = test_group / 10 + '0';
		commands[0][47] = test_group % 10 + '0';
		commands[1][20] = test_group / 10 + '0';
		commands[1][21] = test_group % 10 + '0';
		commands[2][25] = test_group / 10 + '0';
		commands[2][26] = test_group % 10 + '0';


		int test_num = 0;

		int const test_limit = (*(cur_data + 7) - '0') * 10
		                     + (*(cur_data + 8) - '0')
		                     ;
		while(test_num < test_limit)
		{
			commands[0][23] = test_num / 10 + '0';
			commands[0][24] = test_num % 10 + '0';
			commands[0][49] = test_num / 10 + '0';
			commands[0][50] = test_num % 10 + '0';
			commands[1][23] = test_num / 10 + '0';
			commands[1][24] = test_num % 10 + '0';
			commands[2][28] = test_num / 10 + '0';
			commands[2][29] = test_num % 10 + '0';

//			printf(commands[0].c_str());
//			printf(commands[1].c_str());
//			printf(commands[2].c_str());
			system(commands[0].c_str());
			system(commands[1].c_str());
			system(commands[2].c_str());

			test_num++;
		}
		test_group++;
	}

	if(-1 == File::destroy_error_handled(file_list))
		return -2;
}
