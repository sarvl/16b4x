#pragma once

#include <cstdint>

namespace mem{

	uint16_t read         (uint16_t const addr);
	uint16_t read_execute (uint16_t const addr);
	void     write        (uint16_t const addr, uint16_t const data);

	//avoid whenever possible
	uint16_t read_pm (uint32_t const addr);
	void     write_pm(uint32_t const addr, uint16_t const data);

	void     reset();
	void     flush_tlb();

	//the only usecase of these is debugging
	uint16_t read_bypass  (uint16_t const addr);
	void     write_bypass (uint16_t const addr, uint16_t const data);

	struct t_pte{
		unsigned present   :  1;
		unsigned protect   :  1;
		unsigned dirty     :  1;
		unsigned read      :  1;
		unsigned write     :  1;
		unsigned execute   :  1;
		unsigned frame     : 10;

		uint16_t pid;
	};
};
