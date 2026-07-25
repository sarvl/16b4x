#include "interrupt.h"

#include "state.h"
#include "cr.h"
#include "helper.h"
#include "memory.h"

#include <cstdio>

uint16_t iht[256];

extern uint16_t ip;
void interrupt::load_iht()
{
	printf("IHT FLUSHED BY %04X @ %04X\n", mem::read_bypass(ip), ip);
	for(int ii = 0; ii < 256; ii++)
		iht[ii] = mem::read(0x100 + ii);

	return;
}

uint16_t interrupt::get_iht_entry(int const id)
{
	return iht[id];
}


void interrupt::raise(uint8_t const interrupt, uint16_t const info0, uint16_t const info1)
{
	cr::write_bypass(cr::interrupt_id, interrupt);

	if(interrupt_pgf_id == interrupt) 
		cr::write_bypass(cr::interrupt_saved_ip, ip);
	else
		cr::write_bypass(cr::interrupt_saved_ip, ip + 1);

	cr::write_bypass(cr::interrupt_saved_cr0002, cr::read_bypass(cr::privilege_l_0));
	cr::write_bypass(cr::interrupt_saved_cr0003, cr::read_bypass(cr::privilege_l_1));
	cr::write_bypass(cr::interrupt_saved_ui    , ui);
	cr::write_bypass(cr::interrupt_saved_fl    , register_fl_read());

	cr::write_bypass(cr::interrupt_info0       , info0);
	cr::write_bypass(cr::interrupt_info1       , info1);
	return;
}

void interrupt::raise_pgf(uint16_t const addr)
{
	interrupt_pgf = true;
	interrupt::raise(interrupt_pgf_id, addr);
	return;
}

void interrupt::raise_prv(uint16_t const missing_priv_l0, uint16_t const missing_priv_l1)
{
	interrupt_prv = true;
	interrupt::raise(interrupt_prv_id, missing_priv_l0, missing_priv_l1);
	return;
}
void interrupt::raise_prv(privilege::type const priv)
{
	uint32_t const tmp = static_cast<uint32_t>(priv);
	interrupt::raise_prv((tmp >>  0) & 0xFFFF, (tmp >> 16) & 0xFFFF);
	return;
}

void interrupt::raise_pev(uint16_t const addr, uint16_t const type)
{
	interrupt_pev = true;
	interrupt::raise(interrupt_pev_id, addr, type);
	return;
}

void interrupt::raise_ins()
{
	//check this to be sure
	interrupt_ins = true;
	interrupt::raise(interrupt_ins_id, mem::read(ip));
	return;
}

void interrupt::raise_ina()
{
	interrupt_ina = true;
	interrupt::raise(interrupt_ina_id, mem::read(ip));
	return;
}

void interrupt::raise_tmr()
{
	interrupt_tmr = true;
	interrupt::raise(interrupt_tmr);
	return;
}

void interrupt::raise_div()
{
	interrupt_div = true;
	interrupt::raise(interrupt_div);
	return;
}

//void interrupt::handle()
//{
//
//
//}
