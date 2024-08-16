#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

//Log::level is stored in log.cpp as it comes as separate utility
namespace Config{
	//new format can be added by changing config.h config.cpp save.cpp>save_instructions
	namespace Format{
		enum id : int{
			txt_1  =  0,
			txt_2  =  1,
			txt_4  =  2,
			txt_c  =  9,

			bin_1  = 10,
			bin_2  = 11,
			bin_4  = 12,
			bin_c  = 19,

			vhdl_1 = 20,
			vhdl_2 = 21,
			vhdl_4 = 22,
			vhdl_5 = 23
		};
		constexpr int count = 12 * 3;
		
		constexpr std::string_view name_list[count] = {
			"00", "text_1_instruction"  , "txt_1" ,
			"01", "text_2_instruction"  , "txt_2" ,
			"02", "text_4_instruction"  , "txt_4" ,
			"09", "text_continous"      , "txt_c" ,

			"10", "binary_1_instruction", "bin_1" ,
			"11", "binary_2_instruction", "bin_2" ,
			"12", "binary_4_instruction", "bin_4" ,
			"19", "binary_continous"    , "bin_c" ,

			"20", "vhdl_1_instruction"  , "vhdl_1",
			"21", "vhdl_2_instruction"  , "vhdl_2",
			"22", "vhdl_4_instruction"  , "vhdl_4",
			"23", "vhdl_5_instruction"  , "vhdl_5"
		};

		extern Format::id format;
		
		Format::id index_to_fmt(int const index);
	};

	namespace Debug{
		enum Print : uint32_t{
			token      = 0x0000'0001,
			verify     = 0x0000'0002,
			preprocess = 0x0000'0004
		};


		extern uint32_t to_print;
		extern bool     abort   ;
	};

	extern std::vector<std::string_view> filenames;
	extern const char* file_output;

};

