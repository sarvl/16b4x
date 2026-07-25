#pragma once

#include <cstdint>

namespace cr{
	enum type : uint16_t {
		pid                                = 0x0000,
		ptaddr                             = 0x0001,
		privilege_l_0                      = 0x0002,
		privilege_l_1                      = 0x0003,
		interrupt_enabled_0                = 0x0004,
		interrupt_enabled_1                = 0x0005,

		state                              = 0x000E,
		state_update                       = 0x000F,

		interrupt_id                       = 0x0010,
		interrupt_saved_ip                 = 0x0011,
		interrupt_saved_cr0002             = 0x0012,
		interrupt_saved_cr0003             = 0x0013,
		interrupt_saved_ui                 = 0x0014,
		interrupt_saved_fl                 = 0x0015,

		interrupt_info0                    = 0x0018,
		interrupt_info1                    = 0x0019,


		isa_id                             = 0x1000,
		microarch_id                       = 0x1001,
		chipset_id                         = 0x1002,

		substrate_info_0                   = 0x1010,
		substrate_info_1                   = 0x1011,
		substrate_info_2                   = 0x1012,
		substrate_info_3                   = 0x1013,
		substrate_info_4                   = 0x1014,
		substrate_info_5                   = 0x1015,
		substrate_info_6                   = 0x1016,
		substrate_info_7                   = 0x1017,
		substrate_info_8                   = 0x1018,
		substrate_info_9                   = 0x1019,
		substrate_info_A                   = 0x101A,
		substrate_info_B                   = 0x101B,
		substrate_info_C                   = 0x101C,
		substrate_info_D                   = 0x101D,
		substrate_info_E                   = 0x101E,
		substrate_info_F                   = 0x101F,

		substrate_info_ram_size            = substrate_info_0,

		paf_0                              = 0x2000,
		paf_1                              = 0x2001,
		paf_2                              = 0x2002,
		paf_3                              = 0x2003,
		paf_4                              = 0x2004,
		paf_5                              = 0x2005,
		paf_6                              = 0x2006,
		paf_7                              = 0x2007,
		paf_8                              = 0x2008,
		paf_9                              = 0x2009,
		paf_A                              = 0x200A,
		paf_B                              = 0x200B,
		paf_C                              = 0x200C,
		paf_D                              = 0x200D,
		paf_E                              = 0x200E,
		paf_F                              = 0x200F,

		uaf_0                              = 0x5000,
		uaf_1                              = 0x5001,
		uaf_2                              = 0x5002,
		uaf_3                              = 0x5003,
		uaf_4                              = 0x5004,
		uaf_5                              = 0x5005,
		uaf_6                              = 0x5006,
		uaf_7                              = 0x5007,
		uaf_8                              = 0x5008,
		uaf_9                              = 0x5009,
		uaf_A                              = 0x500A,
		uaf_B                              = 0x500B,
		uaf_C                              = 0x500C,
		uaf_D                              = 0x500D,
		uaf_E                              = 0x500E,
		uaf_F                              = 0x500F
	};


	uint16_t read (cr::type const addr);
	void     write(cr::type const addr, uint16_t const data);

	//dangerous, use with extreme care
	void     write_bypass(cr::type const addr, uint16_t const data);
	uint16_t read_bypass (cr::type const addr);
};

