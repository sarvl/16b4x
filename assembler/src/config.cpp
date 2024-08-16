#include "./config.h"

namespace Config{
	namespace Format{
		id format = Format::txt_1; 

		Format::id index_to_fmt(int const index)
		{
			static constexpr Format::id transl[Format::count] = {
				txt_1, txt_1, txt_1,
				txt_2, txt_2, txt_2,
				txt_4, txt_4, txt_4,
				txt_c, txt_c, txt_c,

				bin_1, bin_1, bin_1,
				bin_2, bin_2, bin_2,
				bin_4, bin_4, bin_4,
				bin_c, bin_c, bin_c,

				vhdl_1, vhdl_1, vhdl_1,
				vhdl_2, vhdl_2, vhdl_2,
				vhdl_4, vhdl_4, vhdl_4,
				vhdl_5, vhdl_5, vhdl_5
			};

			return transl[index];
		}
	};

	namespace Debug{
		uint32_t to_print = 0;
		bool     abort    = false;
	};

	std::vector<std::string_view> filenames;
	const char* file_output = "out.bin";
};

