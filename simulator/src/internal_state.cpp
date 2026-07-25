#include "internal_state.h"

uint16_t internal::pid;
uint16_t internal::ptaddr;

uint32_t internal::privilege_levels;

bool     internal::interrupts_enabled;
bool     internal::flush_tlb;
bool     internal::iht_reload;
uint64_t internal::interrupts_enabled_00to7F;
uint64_t internal::interrupts_enabled_F0toFF;

uint16_t internal::interrupt_id;
uint16_t internal::interrupt_saved_ip;
uint16_t internal::interrupt_saved_cr0002;
uint16_t internal::interrupt_saved_cr0003;
uint16_t internal::interrupt_saved_ui;
uint16_t internal::interrupt_saved_fl;

uint16_t internal::interrupt_info0;
uint16_t internal::interrupt_info1;

uint16_t internal::substrate_info[16];
uint16_t internal::paf[16];
uint16_t internal::uaf[16];

void internal::reset()
{
	using namespace internal;
	//note: this procedure *might* be used at any time
	//therefore even though some stuff is global and then by default 0, it is set here anyway 

	pid    = 0x0000;
	ptaddr = 0x0000;
	
	privilege_levels = 0xFFFF'FFFF;

	interrupts_enabled = false;

	flush_tlb          = false;
	iht_reload         = false;

	interrupts_enabled_00to7F = 0;
	interrupts_enabled_F0toFF = 0;

	//interrupt id left uninitialized 
	
	for(int i = 0; i < 16; i++)
	{
		substrate_info[i] = 0;
		paf[i] = 0;
		uaf[i] = 0;
	}
}
