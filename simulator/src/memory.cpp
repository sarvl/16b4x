#include "memory.h"
#include "privilege.h"
#include "interrupt.h"
#include "cr.h"

#include <cstdio>

uint16_t memory[1 << 16];
mem::t_pte tlb[32];

enum class t_access{
	read, write, execute
};

uint16_t mem::read_pm (uint32_t const addr)
{
	return memory[addr];
}
void     mem::write_pm(uint32_t const addr, uint16_t const data)
{
	memory[addr] = data;
	return;
}


uint32_t tlb_translate(uint16_t const vaddr, t_access const access_type)
{
	uint16_t const virt_id = vaddr >> 11;

	auto& pte = tlb[virt_id]; 
	uint16_t const ptadr = cr::read_bypass(cr::ptaddr) + virt_id;

	//in case of TLB miss, do PT "walk"
	//in case of miss after PT walk, definitely no PT entry
	if(0                 == pte.present
	|| cr::read(cr::pid) != pte.pid    )
	{
		uint16_t const newptentry = mem::read_pm(ptadr);

		pte.present = (newptentry >> 15) & 0b1;
		pte.protect = (newptentry >> 14) & 0b1;
		pte.dirty   = (newptentry >> 13) & 0b1;
		pte.read    = (newptentry >> 12) & 0b1;
		pte.write   = (newptentry >> 11) & 0b1;
		pte.execute = (newptentry >> 10) & 0b1;
		pte.frame   = (newptentry >>  0) & 0x3FF;
		pte.pid     = cr::read_bypass(cr::pid);

		//check again
		if(0 == pte.present)
		{
			interrupt::raise_pgf(vaddr);
			return 0;
		}
	}

	if(1 ==  pte.protect
	&& not privilege::check(privilege::protected_memory_access))
	{
		interrupt::raise_prv(privilege::protected_memory_access);
		return 0;
	}

	if(t_access::read    == access_type && 0 == pte.read   )
	{
		interrupt::raise_pev(vaddr, 0x0001);
		return 0;
	}

	if(t_access::write   == access_type && 0 == pte.write  )
	{
		interrupt::raise_pev(vaddr, 0x0002);
		return 0;
	}

	if(t_access::execute == access_type && 0 == pte.execute)
	{
		interrupt::raise_pev(vaddr, 0x0004);
		return 0;
	}


	uint32_t const paddr = (static_cast<uint32_t>(pte.frame) << 11)
	                     | (vaddr & 0x07FF);

	if(t_access::write == access_type
	&& 0               == pte.dirty)
	{
		pte.dirty = 1;

		uint16_t data = mem::read_pm(ptadr);
		data |= 0b00100000'00000000;
		mem::write_pm(ptadr, data);
	}

	return paddr;
}

uint32_t tlb_translate_no_check(uint16_t const vaddr)
{
	uint16_t const virt_id = vaddr >> 11;

	auto& pte = tlb[virt_id]; 
	uint16_t const ptadr = cr::read_bypass(cr::ptaddr) + virt_id;

	//in case of TLB miss, do PT "walk"
	//in case of miss after PT walk, definitely no PT entry
	if(0                 == pte.present
	|| cr::read(cr::pid) != pte.pid    )
	{

		uint16_t const newptentry = mem::read_pm(ptadr);

		pte.present = (newptentry >> 15) & 0b1;
		pte.protect = (newptentry >> 14) & 0b1;
		pte.dirty   = (newptentry >> 13) & 0b1;
		pte.read    = (newptentry >> 12) & 0b1;
		pte.write   = (newptentry >> 11) & 0b1;
		pte.execute = (newptentry >> 10) & 0b1;
		pte.frame   = (newptentry >>  0) & 0x3FF;
		pte.pid     = cr::read_bypass(cr::pid);
	}

	uint32_t const paddr = (static_cast<uint32_t>(pte.frame) << 11)
	                     | (vaddr & 0x07FF);

	return paddr;
}


uint16_t mem::read        (uint16_t const addr)
{
	return memory[tlb_translate(addr, t_access::read)];

}

uint16_t mem::read_bypass (uint16_t const addr)
{
	return memory[tlb_translate_no_check(addr)];
}

uint16_t mem::read_execute(uint16_t const addr)
{
	return memory[tlb_translate(addr, t_access::execute)];

}

void     mem::write       (uint16_t const vaddr, uint16_t const tdata)
{
	uint32_t const addr = tlb_translate(vaddr, t_access::write);

//	if(not interrupt::is_int)
		memory[addr] = tdata;	
	
	return;
}

void     mem::write_bypass(uint16_t const vaddr, uint16_t const tdata)
{
	uint32_t const addr = tlb_translate_no_check(vaddr);
	memory[addr] = tdata;	
	
	return;
}


void     mem::reset()
{
	for(int i = 0; i < 65536; i++)
		memory[i] = 0;

	
	for(int i = 0; i < 32; i++)
	{
		tlb[i].present = true;
		tlb[i].protect = false;
		tlb[i].dirty   = true;
		tlb[i].read    = true;
		tlb[i].write   = true;
		tlb[i].execute = true;

		tlb[i].frame   = i;

		tlb[i].pid     = 0;
	}
	return;
}

extern uint16_t ip;
void     mem::flush_tlb()
{
	printf("TLB FLUSHED BY %04X @ %04X\n", mem::read_bypass(ip), ip);

	for(int i = 0; i < 32; i++)
	{
		tlb[i].present = false;
		tlb[i].protect = false;
		tlb[i].dirty   = false;
		tlb[i].read    = false;
		tlb[i].write   = false;
		tlb[i].execute = false;

		tlb[i].frame   = i;

		tlb[i].pid     = 0;
	}
	return;
}
