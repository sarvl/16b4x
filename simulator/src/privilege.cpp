#include "privilege.h"
#include "cr.h"


#include <cstdio>
bool privilege::check(privilege::type const priv)
{
	uint16_t const l1 = (priv >> 16) & 0xFFFF;
	uint16_t const l0 = (priv      ) & 0xFFFF;

	uint16_t const crl1 = cr::read_bypass(cr::privilege_l_1);
	uint16_t const crl0 = cr::read_bypass(cr::privilege_l_0);

	return l0 == (l0 & crl0)
	    && l1 == (l1 & crl1);
}
