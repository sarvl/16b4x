#pragma once
#include <cstdint>
#include "privilege.h"

namespace interrupt{
	void raise(uint8_t const interrupt, uint16_t const info0 = 0, uint16_t const info1 = 0);

	void load_iht();
	uint16_t get_iht_entry(int const id);

	void raise_pgf(uint16_t const addr);
	void raise_prv(uint16_t const missing_priv_l0, uint16_t const missing_priv_l1);
	void raise_prv(privilege::type const priv);
	void raise_pev(uint16_t const addr, uint16_t const type);
	void raise_ins();
	void raise_ina();
	void raise_tmr();
	void raise_div();
}
