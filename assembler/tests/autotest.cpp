#include <cstdio>

#include <string>

#include <cstdlib> //system
#include <cstring> //memcmp


#include "../../utility/src/file.h"
#include "../../utility/src/log.h"

int main(int const argc, char const* const* const argv)
{
	bool print_detailed   = true;
	bool print_everything = true;
	if(2 == argc)
	{
		if('t' == argv[1][0])
			print_detailed   = false;
		if('e' == argv[1][0])
		{
			print_detailed   = false;
			print_everything = false;
		}
	}

	int total_passed = 0;
	int total_tests  = 0;

	File::t_File file_list;
	File::t_File file_undertest;
	File::t_File file_correct;

	if(-1 == File::create_error_handled(&file_list, "./tests/test_groups.txt"))
		return -1;

	char const* const data = file_list.data;

	//assumes that files are correct
	int off = 0;
	while(off < file_list.size)
	{
		int passed = 0;
		std::string command = "./sasm tests/code/g00t00.sasm -o fout.out ";
		std::string correct_fout = "tests/fout/g00t00.txt";
		std::string correct_sout = "tests/sout/g00t00.txt";
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

		int const test_group = (data[beg + 1] - '0') * 10
		                     + (data[beg + 2] - '0');
		
		if(print_everything)
			printf("\033[1;38;5;55m%.5s\033[0m\n", data + beg + 4);

		int const limit = (data[beg +  9] - '0') * 10
		                + (data[beg + 10] - '0');

		command[19]      = data[beg + 1];
		command[20]      = data[beg + 2];
		correct_fout[12] = data[beg + 1];
		correct_fout[13] = data[beg + 2];
		correct_sout[12] = data[beg + 1];
		correct_sout[13] = data[beg + 2];

		int test_num = 0;

		total_tests += limit;

		while(test_num < limit)
		{
			command[22]      = test_num / 10 + '0';
			command[23]      = test_num % 10 + '0';
			correct_fout[15] = test_num / 10 + '0';
			correct_fout[16] = test_num % 10 + '0';
			correct_sout[15] = test_num / 10 + '0';
			correct_sout[16] = test_num % 10 + '0';

			//make sure no stale file
			system("rm -f fout.out sout.out");
			system(command.c_str());

			system("touch ./fout.out ./sout.out");

			if(File::Error::create_empty == File::create(&file_undertest, "./fout.out"))
				file_undertest.size = 0;
			if(File::Error::create_empty == File::create(&file_correct, correct_fout))
				file_correct.size = 0;

			if(file_undertest.size != file_correct.size
			|| 0 != memcmp(file_undertest.data, file_correct.data, file_correct.size))
			{
				if(print_everything)
					printf("\033[1;38;5;1m[X] g%02dt%02df\n", test_group, test_num);
			}
			else
			{
				if(print_everything && print_detailed)
					printf("\033[1;38;5;46m[V] g%02dt%02df\n", test_group, test_num);
				passed++;
			}	

			if(0 != file_correct.size)   File::destroy(file_correct);
			if(0 != file_undertest.size) File::destroy(file_undertest);

			if(File::Error::create_empty == File::create(&file_undertest, "./sout.out"))
				file_undertest.size = 0;
			if(File::Error::create_empty == File::create(&file_correct, correct_sout))
				file_correct.size = 0;

			if(file_undertest.size != file_correct.size
			|| 0 != memcmp(file_undertest.data, file_correct.data, file_correct.size))
			{
				if(print_everything)
					printf("\033[1;38;5;1m[X] g%02dt%02ds\n", test_group, test_num);
			}
			else
			{
				if(print_everything && print_detailed)
					printf("\033[1;38;5;46m[V] g%02dt%02ds\n", test_group, test_num);
				passed++;
			}	
				
			if(0 != file_correct.size)   File::destroy(file_correct);
			if(0 != file_undertest.size) File::destroy(file_undertest);

			test_num++;

		}
		if(print_everything)
		{
			if(passed == limit * 2)
				printf("\033[1;38;5;46m%d/%d\033[0m\n", passed, limit * 2);
			else
				printf("\033[0;33m%d/%d\033[0m\n", passed, limit * 2);
		}

		total_passed += passed;
	}
	//make sure no stale file
	File::destroy_error_handled(file_list);

	printf("\033[0;36mtotal %d/%d\n\033[0m", total_passed, total_tests * 2);

	//clear after itself
	system("rm -f fout.out sout.out symbols.txt");
}
