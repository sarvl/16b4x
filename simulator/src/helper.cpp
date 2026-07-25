#include "helper.h"

#include "state.h"

uint16_t register_fl_read()
{
	return (fl_d << 4)
	     | (fl_s << 3)
	     | (fl_o << 2)
	     | (fl_c << 1)
	     | (fl_z << 0)
	     ;
}

void     register_fl_write(uint16_t const val)
{
	fl_d = (val >> 4) & 0b1;
	fl_s = (val >> 3) & 0b1;
	fl_o = (val >> 2) & 0b1;
	fl_c = (val >> 1) & 0b1;
	fl_z = (val >> 0) & 0b1;

	return;
}

void    modify_flags(uint32_t const res, uint32_t const A, uint32_t const B)
{
	fl_s = (res >> 15) & 0b1;
	fl_o = (((A ^ B) & ~(B ^ res)) >> 15) & 0b1;
	fl_z = (res & 0xFFFF) == 0x0000;
	fl_c = (res >> 16) & 0b1;
	return;
}
