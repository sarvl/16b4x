#include "instruction.h"

#include "state.h"
#include "interrupt.h"
#include "helper.h"
#include "memory.h"
#include "privilege.h"
#include "io.h"
#include "internal_state.h"

#include <cstdio> //printf
#include <cstdlib> //exit, rand

bool check_cond(uint8_t const cond)
{
	switch(cond)
	{
	case 0b0000: return (    fl_c) and (not fl_z);
	case 0b0001: return (not fl_c) or  (    fl_z);
	case 0b0010: return (    fl_c);
	case 0b0011: return                (    fl_s == fl_o);
	case 0b0100: return (not fl_z) and (    fl_s == fl_o);
	case 0b0101: return (    fl_z) or  (    fl_s != fl_o);
	case 0b0110: return                (    fl_s != fl_o);
	case 0b0111: return (not fl_c);
	case 0b1000: return (not fl_o);
	case 0b1001: return (not fl_s);
	case 0b1010: return (not fl_z);
	case 0b1011: return (    fl_o);
	case 0b1100: return (    fl_s);
	case 0b1101: return (    fl_z);
	case 0b1110: interrupt::raise_ina(); return false;
	case 0b1111: interrupt::raise_ina(); return false;
	default:     return false;
	}
}


void instruction_execute()
{
	if(not ui_modified)
		ui = 0;
		
	//1. fetch from memory
	//2. decode
	//3. execute
	//4. update IP and UI accordingly
	
	uint16_t instruction = mem::read_execute(ip);
	
	unsigned const rd           = (instruction >>  5) & 0x07;
	unsigned const rs           = (instruction >>  8) & 0x07;

	unsigned const imm8_long_t  = 0
		| ((instruction & 0x00FF) >> 0)
		;
	unsigned const imm5_long_t  = 0
		| ((instruction & 0x001F) >> 0)
		;
	unsigned const imm8_t    = 0
		| ((instruction & 0x0700) >> 3)
		| ((instruction & 0x001F) >> 0)
		;
	unsigned const imm11_t      = 0
		| ((instruction & 0x07FF) >> 0)
		;

	/*only imm11 and imm8_j sign extends its immediate*/
	uint16_t imm11  = static_cast<uint16_t>((static_cast<  signed>(imm11_t     << 21) >> 21) & 0xFFFF);
	//lop also uses this format
	uint16_t imm8_j = static_cast<uint16_t>((static_cast<  signed>(imm8_long_t << 24) >> 24) & 0xFFFF);

	uint16_t imm8_l = imm8_long_t;
	uint16_t imm8   = imm8_t;
	uint16_t imm5   = imm5_long_t;

	if(ui_modified)
	{
		imm11  = (ui << 8) | (imm11  & 0xFF);
		imm8_l = (ui << 8) | (imm8_l & 0xFF);
		imm8_j = (ui << 8) | (imm8_j & 0xFF);
		imm8   = (ui << 8) | (imm8   & 0xFF);
		//imm5 is never extended
	}
	
	ui_modified = false;

	switch((instruction >> 11) & 0b11111)
	{
	case 0b00000: switch(instruction & 0b11111)
	{
		case 0b00000:   /*invalid*/
			interrupt::raise_ins();
			ip++;
			return;
		case 0b00001:   /*scc v0*/
			regs[rd] = check_cond(0b0000 | ((instruction >> 8) & 0b111));
			ip++;
			return;

		case 0b00010:  /* dvu */
			interrupt::raise_ins();
			ip++;
			return;

		case 0b00011: /* dvs */
			interrupt::raise_ins();
			ip++;
			return;

		case 0b00100:   switch((instruction >> 8) & 0b111)
		{
			case 0b000: /* irt */
				if(not privilege::check(privilege::interrupt_control))
				{
					interrupt::raise(privilege::interrupt_control);
					ip++;
					return;
				}
				printf("returning from interrupt @ %04X to %04X\n", ip, cr::read_bypass(cr::interrupt_saved_ip));

				internal::interrupts_enabled = true;
				cr::write_bypass(cr::privilege_l_0, cr::read_bypass(cr::interrupt_saved_cr0002));
				cr::write_bypass(cr::privilege_l_1, cr::read_bypass(cr::interrupt_saved_cr0003));

				ip = cr::read_bypass(cr::interrupt_saved_ip);
				ui = cr::read_bypass(cr::interrupt_saved_ui);
				register_fl_write(cr::read_bypass(cr::interrupt_saved_fl));
				return;

			case 0b001: /* hlt */
				if(not privilege::check(privilege::processor_control))
				{
					interrupt::raise(privilege::processor_control);
					ip++;
					return;
				}
				halt = true;
				return;
			case 0b010: /* invalid */
			case 0b011: /* invalid */
				interrupt::raise_ins();
				ip++;
				return;

			case 0b100: /* nop reserved */
			case 0b101: /* nop reserved */
				if(not privilege::check(privilege::processor_control))
				{
					interrupt::raise_prv(privilege::processor_control);
					ip++;
					return;
				}
				ip++;
				return;

			case 0b110: /* invalid */
			case 0b111: /* invalid */
				interrupt::raise_ins();
				ip++;
				return;

		}  /*0b00100 end*/ return;	
		case 0b00101:  /* invalid */
			interrupt::raise_ins();
			ip++;
			return;

		case 0b00110:   switch((instruction >> 8) & 0b111)
		{
			case 0b000:  /*nop*/
				ip++;
				return;
			case 0b001: /* rnd */
				regs[rd] = rand();
				ip++;
				return;
			case 0b010: /* psh R */
				sp--;
				mem::write(sp, regs[rd]);
				ip++;

				return;
			case 0b011: /* pop R */
				regs[rd] = mem::read(sp);
				sp++;
				ip++;

				return;
			case 0b100: /* cal R */
				sp--;
				mem::write(sp, ip + 1);
				ip = regs[rd];

				return;
			case 0b101: /* ret */
				ip = mem::read(sp);
				sp++;

				return;
			case 0b110: /* not R */
				regs[rd] = ~regs[rd];
				ip++;

				fl_s = regs[rd] >> 15;
				fl_z = regs[rd] == 0;
				return;
			case 0b111: /* neg R */
				regs[rd] = -regs[rd];
				ip++;

				fl_s = regs[rd] >> 15;
				fl_z = regs[rd] == 0;
				return;

		}  /*0b00110 end*/ break;
		case 0b00111:   switch((instruction >> 8) & 0b111)
		{
			case 0b000: /* prf */
			case 0b001: /* nop reserved */
			case 0b010: /* nop reserved */
			case 0b011: /* nop reserved */
			case 0b100: /* nop reserved */
				ip++;
				return;
			case 0b101: /* invalid */
			case 0b110: /* invalid */
			case 0b111: /* invalid */
				interrupt::raise_ins();
				ip++;
				return;
		}  /*0b00111 end*/  break;
		case 0b01000:   /* prd R R */
			regs[rd] = io::read(regs[rs]);
			ip++;
			return;
		case 0b01001:   /* pwr R R */
			io::write(regs[rs], regs[rd]);
			ip++;
			return;
		case 0b01010:  /* xrd */
			switch(rs)
			{
			case 0: regs[rd] = ip + 1;             ip++; return;
			case 1: regs[rd] = lc;                 ip++; return;

			case 3: regs[rd] = sp;                 ip++; return;
			case 4: regs[rd] = register_fl_read(); ip++; return;
			case 5: regs[rd] = ar;                 ip++; return;

			default: interrupt::raise_ina();              ip++; return;
			}
		case 0b01011:  /* xwr */
			switch(rd)
			{
			case 0: ip = regs[rs];                           return;
			case 1: lc = regs[rs];                     ip++; return;
			case 2: ui = regs[rs]; ui_modified = true; ip++; return;
			case 3: sp = regs[rs];                     ip++; return;
			case 4: register_fl_write(regs[rs]);       ip++; return;
			case 5: ar = regs[rs];                     ip++; return;

			default: interrupt::raise_ina();           ip++; return;
			}
		case 0b01100:   /* mrd R R */
			regs[rd] = mem::read(regs[rs]);
			ip++;
			return;
		case 0b01101:   /* mrd R R */
			mem::write(regs[rs], regs[rd]);
			ip++;
			return;
		case 0b01110:  
		case 0b01111:
			interrupt::raise_ins();
			ip++;
			return;
		case 0b10000:   /* mlu R R */
			regs[rd] = regs[rd] * regs[rs];
			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;

		case 0b10001: /* cmp R R */
		{
			uint32_t const res = static_cast<uint32_t>(regs[rd]) - static_cast<uint32_t>(regs[rs]);

			fl_s = (res >> 15) & 0b1;
			fl_o = (((regs[rd] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
			fl_z = (res & 0xFFFF) == 0x0000;
			fl_c = (res >> 16) & 0b1;

			ip++;
			return;
		}
		case 0b10010:   /* tst R R */
		{
			uint16_t const res = regs[rd] & regs[rs];

			fl_s = (res >> 15) & 0b1;
			fl_z = (res & 0xFFFF) == 0x0000;

			ip++;
			return;
		}
		case 0b10011:  /* invalid,  mls requires Arithmetic extension */
			interrupt::raise_ins();
			ip++;
			return;
		case 0b10100: /* mov R R */
			regs[rd] = regs[rs];
			ip++;
			return;
		case 0b10101: /* crd R R */
			regs[rd] = cr::read(static_cast<cr::type>(regs[rs]));
			ip++;
			return;
		case 0b10110: /* cwr R R */
			cr::write(static_cast<cr::type>(regs[rs]), regs[rd]);
			ip++;
			return;
		case 0b10111:   /* scc v1 R */
			regs[rd] = check_cond(0b1000 | ((instruction >> 8) & 0b111));
			ip++;
			return;

		case 0b11000:   /* add R R */
		{
			uint32_t const res = static_cast<uint32_t>(regs[rd]) + static_cast<uint32_t>(regs[rs]);

			fl_s = (res >> 15) & 0b1;
			fl_o = (((regs[rd] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
			fl_z = (res & 0xFFFF) == 0x0000;
			fl_c = (res >> 16) & 0b1;

			regs[rd] = res;
			ip++;
			return;
		}
			
		case 0b11001:   /* sub R R */
		{
			uint32_t const res = static_cast<uint32_t>(regs[rd]) - static_cast<uint32_t>(regs[rs]);

			fl_s = (res >> 15) & 0b1;
			fl_o = (((regs[rd] ^ regs[rs]) & ~(regs[rs] ^ res)) >> 15) & 0b1;
			fl_z = (res & 0xFFFF) == 0x0000;
			fl_c = (res >> 16) & 0b1;

			regs[rd] = res;
			ip++;
			return;
		}
		case 0b11010:   /* and R R */
			regs[rd] = regs[rd] & regs[rs];

			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;
		case 0b11011:  /* ann R R */
			regs[rd] = regs[rd] & ~regs[rs];

			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;
		case 0b11100:   /* orr R R */
			regs[rd] = regs[rd] | regs[rs];

			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;
		case 0b11101:  /* xor R R */
			regs[rd] = regs[rd] ^ regs[rs];

			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;
		case 0b11110:   /* shl R R */
			if(regs[rs] >= 16)
				regs[rd] = 0;
			else
				regs[rd] = regs[rd] << regs[rs];

			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;
		case 0b11111:   /* shr R R */
			if(regs[rs] >= 16)
				regs[rd] = 0;
			else
				regs[rd] = regs[rd] >> regs[rs];

			fl_s = regs[rd] >> 15;
			fl_z = regs[rd] == 0;
			ip++;
			return;
	}   /*0b00000 end*/ break;
	case 0b00001:       /* invalid, SW defined */ 
		interrupt::raise_ins();
		ip++;
		return;

	case 0b00010:       switch((instruction >> 8) & 0b111)
	{
		case 0b000: /* int i */
			printf("issuing interrupt %d\n", imm8_l);

			interrupt_sw    = true;
			interrupt_sw_id = imm8_l;

			ip++;
			return;
		case 0b001: /*invalid*/
			interrupt::raise_ins();	
			ip++;
			return;
		case 0b010: /* crd R I */
			regs[rd] = cr::read(static_cast<cr::type>(imm5));
			ip++;
			return;
		case 0b011: /* cwr I R */
			cr::write(static_cast<cr::type>(imm5), regs[rd]);
			ip++;
			return;
		case 0b100: /* invalid */
		case 0b101: /* invalid */
		case 0b110: /* invalid */
		case 0b111: /* invalid */
			interrupt::raise_ins();	
			ip++;
			return;
	}  /*0b00010 end*/   break;
	case 0b00011:       switch((instruction >> 8) & 0b111)
	{
		case 0b000: /* nop / reserved */
		case 0b001: /* nop / reserved */
		case 0b010: /* nop / reserved */
		case 0b011: /* nop / reserved */
		case 0b100: /* nop / reserved */
			ip++;
			return;
		case 0b101: /* invalid */
		case 0b110: /* invalid */
		case 0b111: /* lop imm */
			lc--;

			if(lc != 0)
				ip += imm8_j + 1;
			else
				ip++;

			return;
	}  /*0b00011 end*/    break; 
	case 0b00100: /* jcc v0 */
	case 0b00101: /* jcc v1 */
		if(check_cond((instruction >> 8) & 0b1111))
			ip += imm8_j + 1;
		else
			ip++;
		return;
	case 0b00110: /* invalid */
	case 0b00111: /* invalid */
		interrupt::raise_ins();	
		ip++;
		return;
	case 0b01000: /* prd R I */
		regs[rd] = io::read(imm8);
		ip++;
		return;
	case 0b01001: /* pwr R I */
		io::write(imm8, regs[rd]);
		ip++;
		return;
	case 0b01010: /* jmp i11 */
		ip += imm11 + 1;
		return;
	case 0b01011: /* xwr xr imm8 */
		switch(rd)
		{
		case 0: ip = imm8;                           return;
		case 1: lc = imm8;                     ip++; return;
		case 2: ui = imm8; ui_modified = true; ip++; return;
		case 3: sp = imm8;                     ip++; return;
		case 4: register_fl_write(imm8);       ip++; return;
		case 5: ar = imm8;                     ip++; return;

		default: interrupt::raise_ina();       ip++; return;
		}
	case 0b01100: /* mrd R I */
		regs[rd] = mem::read(imm8);
		ip++;
		return;
	case 0b01101: /* mwr I R */
		mem::write(imm8, regs[rd]);
		ip++;
		return;
	case 0b01110: /* srd I R*/
		regs[rd] = mem::read(sp + imm8);
		ip++;
		return;
	case 0b01111: /* swr R I */
		mem::write(sp + imm8, regs[rd]);
		ip++;
		return;
	case 0b10000: /* mlu R I8 */
		regs[rd] = regs[rd] * imm8;
		ip++;

		fl_s = regs[rd] >> 15;
		fl_z = regs[rd] == 0;
		return;
	case 0b10001: /*cmp R I */
	{
		uint32_t const res = static_cast<uint32_t>(regs[rd]) - static_cast<uint32_t>(imm8);

		fl_s = (res >> 15) & 0b1;
		fl_o = (((regs[rd] ^ imm8) & ~(imm8 ^ res)) >> 15) & 0b1;
		fl_z = (res & 0xFFFF) == 0x0000;
		fl_c = (res >> 16) & 0b1;

		ip++;
		return;
	}
	case 0b10010: /* tst R I */
	{
		uint16_t const res = regs[rd] & imm8;

		fl_s = (res >> 15) & 0b1;
		fl_z = (res & 0xFFFF) == 0x0000;

		ip++;
		return;
	}
	case 0b10011: /* invalid */
		interrupt::raise_ins();	
		ip++;
		return;
	case 0b10100: /*mov R I */
		regs[rd] = imm8;
		ip++;
		return;
	case 0b10101: /*cal imm11 */
		sp--;
		mem::write(sp, ip + 1);
		ip += imm11 + 1;
		return;
	case 0b10110: /* mcc v0 */
	case 0b10111: /* mcc v1 */

		if(check_cond(((instruction >> 8) & 0b1000) | ((instruction >> 1) & 0b111) )) 
			regs[rd] = regs[rs];

		ip++;
		return;
	case 0b11000: /* add R I */
	{
		uint32_t const res = static_cast<uint32_t>(regs[rd]) + static_cast<uint32_t>(imm8);

		fl_s = (res >> 15) & 0b1;
		fl_o = (((regs[rd] ^ imm8) & ~(imm8 ^ res)) >> 15) & 0b1;
		fl_z = (res & 0xFFFF) == 0x0000;
		fl_c = (res >> 16) & 0b1;

		regs[rd] = res;
		ip++;
		return;
	}
	case 0b11001: /* sub R I */
	{
		uint32_t const res = static_cast<uint32_t>(regs[rd]) - static_cast<uint32_t>(imm8);

		fl_s = (res >> 15) & 0b1;
		fl_o = (((regs[rd] ^ imm8) & ~(imm8 ^ res)) >> 15) & 0b1;
		fl_z = (res & 0xFFFF) == 0x0000;
		fl_c = (res >> 16) & 0b1;

		regs[rd] = res;
		ip++;
		return;
	}
	case 0b11010: /* and R I */
		regs[rd] = regs[rd] & imm8;

		fl_s = (regs[rd] >> 15) & 0b1;
		fl_z = (regs[rd] & 0xFFFF) == 0x0000;

		ip++;
		return;
	case 0b11011: /* invalid */
		interrupt::raise_ins();
		ip++;
		return;

	case 0b11100: /* orr R I */
		regs[rd] = regs[rd] | imm8;

		fl_s = (regs[rd] >> 15) & 0b1;
		fl_z = (regs[rd] & 0xFFFF) == 0x0000;

		ip++;
		return;
	case 0b11101: /* xor R I */
		regs[rd] = regs[rd] ^ imm8;

		fl_s = (regs[rd] >> 15) & 0b1;
		fl_z = (regs[rd] & 0xFFFF) == 0x0000;

		ip++;
		return;
	case 0b11110: /* shl R I */
		if(imm8 >= 16)
			regs[rd] = 0;
		else
			regs[rd] = regs[rd] << imm8;

		fl_s = regs[rd] >> 15;
		fl_z = regs[rd] == 0;
		ip++;
		return;
	case 0b11111: /* shr R I */
		if(imm8 >= 16)
			regs[rd] = 0;
		else
			regs[rd] = regs[rd] >> imm8;

		fl_s = regs[rd] >> 15;
		fl_z = regs[rd] == 0;
		ip++;
		return;
	}

	return;
}
