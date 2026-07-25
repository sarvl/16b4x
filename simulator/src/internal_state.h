#pragma once

#include <cstdint>

namespace internal{
	extern uint16_t pid;
	extern uint16_t ptaddr;

	extern uint32_t privilege_levels;

	extern bool     interrupts_enabled;
	extern bool     flush_tlb;
	extern bool     iht_reload;
	extern uint64_t interrupts_enabled_00to7F;
	extern uint64_t interrupts_enabled_F0toFF;

	extern uint16_t interrupt_id;
	extern uint16_t interrupt_saved_ip;
	extern uint16_t interrupt_saved_cr0002;
	extern uint16_t interrupt_saved_cr0003;
	extern uint16_t interrupt_saved_ui;
	extern uint16_t interrupt_saved_fl;

	extern uint16_t interrupt_info0;
	extern uint16_t interrupt_info1;

	extern uint16_t substrate_info[16];
	extern uint16_t paf[16];
	extern uint16_t uaf[16];

	void reset();
}
