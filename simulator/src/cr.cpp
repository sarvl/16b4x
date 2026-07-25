#include "cr.h"
#include "privilege.h"
#include "interrupt.h"

#include "internal_state.h"

#include <cstdio> //printf
#include <cstdlib> //exit

uint16_t cr::read (cr::type const addr)
{
	//interrupt handled by check
	if(not ((addr >> 14 ) & 0b1)
	&& not privilege::check(privilege::control_register_access))
		return 0;

	return cr::read_bypass(addr);
}

void    cr::write(cr::type const addr, uint16_t const data)
{
	//interrupt handled by check
	if(not privilege::check(privilege::control_register_access))
		return;

	cr::write_bypass(addr, data);
	return;
}

void    cr::write_bypass(cr::type const addr, uint16_t const data)
{
	switch(addr)
	{
	case cr::pid:                      internal::pid    = data; break;
	case cr::ptaddr:                   internal::ptaddr = data;  break;
	case cr::privilege_l_0:            internal::privilege_levels = ( (internal::privilege_levels & 0xFFFF'0000 ) | (static_cast<uint32_t>(data) <<  0) ); break;
	case cr::privilege_l_1:            internal::privilege_levels = ( (internal::privilege_levels & 0x0000'FFFF ) | (static_cast<uint32_t>(data) << 16) ); break;
	
	case cr::interrupt_enabled_0:      internal::interrupts_enabled_00to7F = (internal::interrupts_enabled_00to7F & 0xFFFF'FFFF'0000'FFFF) | (static_cast<uint64_t>(data) << 16); break;
	case cr::interrupt_enabled_1:      internal::interrupts_enabled_00to7F = (internal::interrupts_enabled_00to7F & 0xFFFF'0000'FFFF'FFFF) | (static_cast<uint64_t>(data) << 32); break;

	case cr::state:
		internal::interrupts_enabled = !!(data & 0b1);

		break;
	case cr::state_update:
		if(data & 0x0001)
			internal::iht_reload         = true;
		if(data & 0x0002)
			internal::flush_tlb          = true;
		break;
	
	case cr::interrupt_id:             internal::interrupt_id              = data;  break;
	case cr::interrupt_saved_ip:       internal::interrupt_saved_ip        = data;  break;
	case cr::interrupt_saved_cr0002:   internal::interrupt_saved_cr0002    = data;  break;
	case cr::interrupt_saved_cr0003:   internal::interrupt_saved_cr0003    = data;  break;
	case cr::interrupt_saved_ui:       internal::interrupt_saved_ui        = data;  break;
	case cr::interrupt_saved_fl:       internal::interrupt_saved_fl        = data;  break;
	case cr::interrupt_info0:          internal::interrupt_info0           = data;  break;
	case cr::interrupt_info1:          internal::interrupt_info1           = data;  break;

	//case cr::isa_id:                 meaningless
	//case cr::microarch_id:           meaningless
	//case cr::chipset_id:             meaningless

	case cr::substrate_info_0:
	case cr::substrate_info_1: 
	case cr::substrate_info_2: 
	case cr::substrate_info_3: 
	case cr::substrate_info_4: 
	case cr::substrate_info_5: 
	case cr::substrate_info_6: 
	case cr::substrate_info_7: 
	case cr::substrate_info_8: 
	case cr::substrate_info_9:
	case cr::substrate_info_A:
	case cr::substrate_info_B:
	case cr::substrate_info_C:
	case cr::substrate_info_D:
	case cr::substrate_info_E:
	case cr::substrate_info_F:         internal::substrate_info[addr & 0xF] = data; break;


/* meaningless
	case cr::paf_0:                        
	case cr::paf_1:                        
	case cr::paf_2:                        
	case cr::paf_3:                        
	case cr::paf_4:                        
	case cr::paf_5:                        
	case cr::paf_6:                        
	case cr::paf_7:                        
	case cr::paf_8:                        
	case cr::paf_9:                        
	case cr::paf_A:                        
	case cr::paf_B:                        
	case cr::paf_C:                        
	case cr::paf_D:                        
	case cr::paf_E:                        
	case cr::paf_F:                    return 0x0000;

	case cr::uaf_0:                    
	case cr::uaf_1:                    
	case cr::uaf_2:                    
	case cr::uaf_3:                    
	case cr::uaf_4:                    
	case cr::uaf_5:                    
	case cr::uaf_6:                    
	case cr::uaf_7:                    
	case cr::uaf_8:                    
	case cr::uaf_9:                    
	case cr::uaf_A:                    
	case cr::uaf_B:                    
	case cr::uaf_C:                    
	case cr::uaf_D:                    
	case cr::uaf_E:                    
	case cr::uaf_F:                    return 0x0000;
*/

	default:
		interrupt::raise_ina();
		return;
	}
}

uint16_t cr::read_bypass(cr::type const addr)
{
	switch(addr)
	{
	case cr::pid:                      return  internal::pid;
	case cr::ptaddr:                   return  internal::ptaddr; 
	case cr::privilege_l_0:            return (internal::privilege_levels >>  0) & 0xFFFF;
	case cr::privilege_l_1:            return (internal::privilege_levels >> 16) & 0xFFFF;
	
	case cr::interrupt_enabled_0:      return (internal::interrupts_enabled_00to7F >> 32) & 0xFFFF;
	case cr::interrupt_enabled_1:      return (internal::interrupts_enabled_00to7F >> 48) & 0xFFFF;
	//case cr::state_update: no effect
	case cr::interrupt_id:             return  internal::interrupt_id;
	case cr::interrupt_saved_ip:       return  internal::interrupt_saved_ip;
	case cr::interrupt_saved_cr0002:   return  internal::interrupt_saved_cr0002;
	case cr::interrupt_saved_cr0003:   return  internal::interrupt_saved_cr0003;
	case cr::interrupt_saved_ui:       return  internal::interrupt_saved_ui;
	case cr::interrupt_saved_fl:       return  internal::interrupt_saved_fl;
	case cr::interrupt_info0:          return  internal::interrupt_info0;
	case cr::interrupt_info1:          return  internal::interrupt_info1;

	case cr::isa_id:                   return  0x00'00;
	case cr::microarch_id:             return  0xFF'00;
	case cr::chipset_id:               return  0x00'00;

	case cr::substrate_info_0:
	case cr::substrate_info_1: 
	case cr::substrate_info_2: 
	case cr::substrate_info_3: 
	case cr::substrate_info_4: 
	case cr::substrate_info_5: 
	case cr::substrate_info_6: 
	case cr::substrate_info_7: 
	case cr::substrate_info_8: 
	case cr::substrate_info_9:
	case cr::substrate_info_A:
	case cr::substrate_info_B:
	case cr::substrate_info_C:
	case cr::substrate_info_D:
	case cr::substrate_info_E:
	case cr::substrate_info_F:         return internal::substrate_info[addr & 0xF];

	case cr::paf_0:                        
	case cr::paf_1:                        
	case cr::paf_2:                        
	case cr::paf_3:                        
	case cr::paf_4:                        
	case cr::paf_5:                        
	case cr::paf_6:                        
	case cr::paf_7:                        
	case cr::paf_8:                        
	case cr::paf_9:                        
	case cr::paf_A:                        
	case cr::paf_B:                        
	case cr::paf_C:                        
	case cr::paf_D:                        
	case cr::paf_E:                        
	case cr::paf_F:                    return 0x0000;

	case cr::uaf_0:                    
	case cr::uaf_1:                    
	case cr::uaf_2:                    
	case cr::uaf_3:                    
	case cr::uaf_4:                    
	case cr::uaf_5:                    
	case cr::uaf_6:                    
	case cr::uaf_7:                    
	case cr::uaf_8:                    
	case cr::uaf_9:                    
	case cr::uaf_A:                    
	case cr::uaf_B:                    
	case cr::uaf_C:                    
	case cr::uaf_D:                    
	case cr::uaf_E:                    
	case cr::uaf_F:                    return 0x0000;


	default:
		interrupt::raise_ina();
		return 0x0000;
	}
}
