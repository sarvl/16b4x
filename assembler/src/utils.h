#pragma once

#include <string_view>

#include "./warning.h"

namespace Utils{
	using namespace std::literals;

	//wrappers for Log::warning respecting current settings
	void warning(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num,
		Warning::id      const wid,
		std::string_view const info = ""sv
		);
	void warning(
		std::string_view const str, 
		Warning::id      const wid,
		std::string_view const info = ""sv
		);

	uint32_t digit_count(uint32_t x);



	constexpr bool is_digit       (char const c) 
		{ return '0' <= c && c <= '9';}

	constexpr bool is_hexd_digit  (char const c)
		{ return is_digit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');}

	constexpr bool is_bit         (char const c)
		{ return '0' == c || '1' == c;}

	constexpr bool is_lowercase   (char const c)
		{ return 'a' <= c && c <= 'z';}
	constexpr bool is_uppercase   (char const c)
		{ return 'A' <= c && c <= 'Z';}
	constexpr bool is_letter      (char const c) 
		{ return is_lowercase(c) || is_uppercase(c); } 

	constexpr bool is_alphanumeric(char const c)
		{ return is_digit(c) || is_letter(c);}

	constexpr bool is_id_char     (char const c)
		{ return is_alphanumeric(c) || '_' == c || '-' == c; }

	constexpr bool is_whitespace  (char const c)
		{ return ' ' == c || '\t' == c || '\n' == c; }
};

