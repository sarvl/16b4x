#pragma once 

#include <cstdint>


namespace privilege{
	enum type : uint32_t {
		//l0 is bottom half
		//l1 is upper half
		
		processor_control       = 0x0000'0001,
		io_access               = 0x0000'0002,
		control_register_access = 0x0000'0004,
		protected_memory_access = 0x0000'0008,
		virtual_memory_control  = 0x0000'0010,
		interrupt_control       = 0x0000'0020
	};

	bool check(privilege::type const priv);
};
