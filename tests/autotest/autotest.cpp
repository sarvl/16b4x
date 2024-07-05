#include <cstdio>

#include <string>

#include <cstdlib> //system
#include <cstring> //memcmp


#include "../../utility/src/file.h"
#include "../../utility/src/log.h"

int main()
{
	File::t_File file_list;
	File::t_File file_undertest;
	File::t_File file_correct;

	if(-1 == File::create_error_handled(&file_list, "./tests/test_groups.txt"))
		return -1;

	std::string commands[] = {
		"./cosi ./tests/bin/g00t00.bin\n",
		"rm -f dump.txt\n"
	};
	std::string filename = "./tests/out/g00t00.out";

	//assumes that files are correct
	int test_group = 0;
	while(test_group * 10 < file_list.size)
	{
		char const* const cur_data = file_list.data + test_group * 10;
		printf("\033[1;38;5;55m%.7s\033[0m\n", cur_data);

		commands[0][20] = test_group / 10 + '0';
		commands[0][21] = test_group % 10 + '0';
		filename[13]    = test_group / 10 + '0';
		filename[14]    = test_group % 10 + '0';


		int test_num = 0;

		int const test_limit = (*(cur_data + 7) - '0') * 10
		                     + (*(cur_data + 8) - '0')
		                     ;
		while(test_num < test_limit)
		{
			commands[0][23] = test_num / 10 + '0';
			commands[0][24] = test_num % 10 + '0';
			filename[16]    = test_num / 10 + '0';
			filename[17]    = test_num % 10 + '0';

			system(commands[0].c_str());

			if(-1 == File::create_error_handled(&file_undertest, "./dump.txt"))
			{
				Log::info("aborting further processing");
				system(commands[1].c_str());
				goto exit;
			}

			if(-1 == File::create_error_handled(&file_correct, filename))
			{
				File::destroy_error_handled(file_undertest);
				Log::info("aborting further processing");
				system(commands[1].c_str());
				goto exit;
			}


			if(file_undertest.size != file_correct.size
			|| 0 != memcmp(file_undertest.data, file_correct.data, file_correct.size))
				printf("\033[1;38;5;1m[X] g%02dt%02d\n", test_group, test_num);
			else
				printf("\033[1;38;5;46m[V] g%02dt%02d\n", test_group, test_num);
				

			if((-1 == File::destroy_error_handled(file_correct))
			//try to run both, destroy anything necessary
			 | (-1 == File::destroy_error_handled(file_undertest)))
			{
				Log::info("aborting further processing");
				system(commands[1].c_str());
				goto exit;
			}

			system(commands[1].c_str());
			test_num++;
			
		}

		test_group++;
	}

exit:
	File::destroy_error_handled(file_list);
}
