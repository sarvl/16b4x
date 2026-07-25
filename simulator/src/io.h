#pragma once

#include <cstdint>

namespace io{
	
	uint16_t read (uint16_t const pid);
	void     write(uint16_t const pid, uint16_t const data);

	int      reset(char const* const filename);

	namespace catalyst{
		constexpr uint16_t id = 0x00;

		namespace port{
			enum type : uint16_t{
				cmd        = 0x0000,
				drive_addr = 0x000E,
				drive_data = 0x000F,
			};
		};

		namespace cmd{
			enum type : uint16_t{
				invalid    = 0x00,
				info_fetch = 0x01,
				err_id     = 0x02,
				io_init    = 0x03,
			};
		};

		namespace info_fetch{
			enum type : uint16_t{
				invalid  = 0x00,
				version  = 0x01,
				pmemsize = 0x02
			};
		};
	};
	

};
