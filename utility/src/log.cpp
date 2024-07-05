#include "log.h"
#include <iostream>


namespace Log{
	Level log_level = Level::info;
	bool is_error   = false;
	bool is_warning = false;


	void error(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		)
	{
		std::cout << "\033[0;31mERROR:\033[0m " << str << "\n"
				  << "\033[0;36m\tfile:\033[0m  " << file_name << "\n"
				  << "\033[0;36m\tline:\033[0m  " << line_num << "\n";
		is_error = true;
		return;
	}

	void error(
		std::string_view const str
		)
	{
		std::cout << "\033[0;31mERROR:\033[0m " << str << "\n";
		is_error = true;
		return;
	}
	void perror(
		std::string_view const str
		)
	{
		int const terrno = errno;
		std::cout << "\033[0;31mERROR:\033[0m " << str << "\n";
		//alignment
		errno = terrno;
		::perror("     ");
		is_error = true;
		return;
	}

	void warning(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		)
	{
		if(log_level >= Level::warning)
			std::cout << "\033[0;35mWARNING:\033[0m " << str << "\n"
					  << "\033[0;36m\tfile:\033[0m  " << file_name << "\n"
					  << "\033[0;36m\tline:\033[0m  " << line_num << "\n";

		is_warning = true;
		return;
	}
	void warning(
		std::string_view const str 
		)
	{
		if(log_level >= Level::warning)
			std::cout << "\033[0;35mWARNING:\033[0m " << str << "\n";

		is_warning = true;
		return;
	}

	void info(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		)
	{
		if(log_level >= Level::info)
			std::cout << "\033[0;33mINFO:\033[0m "    << str << "\n"
					  << "\033[0;36m\tfile:\033[0m  " << file_name << "\n"
					  << "\033[0;36m\tline:\033[0m  " << line_num << "\n";

		return;
	}
	void info(
		std::string_view const str
		)
	{
		if(log_level >= Level::info)
			std::cout << "\033[0;33mINFO:\033[0m " << str << "\n";

		return;
	}

};


