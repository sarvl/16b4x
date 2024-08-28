#include <cstdio>
#include <cstdint>

void disassemble(uint16_t const instruction)
{
	unsigned const rf           = (instruction >>  5) & 0x07;
	unsigned const rs           = (instruction >>  8) & 0x07;

	unsigned const imm8_long_t  = 0
		| ((instruction & 0x0001) <<  7)
		| ((instruction & 0x00FE) >>  1)
		;
	unsigned const imm5_long_t  = 0
		| ((instruction & 0x0001) <<  4)
		| ((instruction & 0x001E) >>  1)
		;
	unsigned const imm8_t    = 0
		| ((instruction & 0x0001) <<  7)
		| ((instruction & 0x0700) >>  4)
		| ((instruction & 0x001E) >>  1)
		;
	unsigned const imm11_t      = 0
		| ((instruction & 0x0001) << 10)
		| ((instruction & 0x07FE) >>  1)
		;
	  signed const imm11  = static_cast<  signed>(imm11_t     << 21) >> 21;
	unsigned const imm8_l = static_cast<unsigned>(imm8_long_t << 24) >> 24;
	  signed const imm8_j = static_cast<  signed>(imm8_long_t << 24) >> 24;
	  signed const imm8   = static_cast<  signed>(imm8_t      << 24) >> 24;
	  signed const imm5   = static_cast<  signed>(imm5_long_t << 27) >> 27;


	constexpr static char xrd_str[8][3] = {
		"IP", "--", "SP", "FL", "--", "--", "--", "--"};
	constexpr static char xwr_str[8][3] = {
		"IP", "UI", "SP", "FL", "--", "--", "--", "--"};

	char const* const xrd_xr = xrd_str[rs];
	char const* const xwr_xr = xwr_str[rf];

	switch((instruction >> 11) & 0b11111)
	{
	case 0b00000: switch(instruction & 0b11111)
	{
		case 0b00000:   printf("INVALID");                   break;
		case 0b00001:   switch((instruction >> 8) & 0b111)
		{
			case 0b000: printf("irt");                       break;
			case 0b001: printf("hlt");                       break;
			case 0b010: printf("INVALID");                   break;
			case 0b011: printf("INVALID");                   break;
			case 0b100: printf("INVALID");                   break;
			case 0b101: printf("INVALID");                   break;
			case 0b110: printf("INVALID");                   break;
			case 0b111: printf("INVALID");                   break;
		}  /*0b00001 end*/                                   break;
		case 0b00010:   printf("INVALID");                   break;
		case 0b00011:   switch((instruction >> 8) & 0b111)
		{
			case 0b000: printf("nop");                       break;
			case 0b001: printf("rng R%d", rf);               break;
			case 0b010: printf("psh R%d", rf);               break;
			case 0b011: printf("pop R%d", rf);               break;
			case 0b100: printf("cal R%d", rf);               break;
			case 0b101: printf("ret");                       break;
			case 0b110: printf("not R%d", rf);               break;
			case 0b111: printf("neg R%d", rf);               break;
		}  /*0b00011 end*/                                   break;
		case 0b00100:   switch((instruction >> 8) & 0b111)
		{
			case 0b000: printf("prf R%d", rf);               break;
			case 0b001: printf("fls R%d", rf);               break;
			case 0b010: printf("INVALID");                   break;
			case 0b011: printf("INVALID");                   break;
			case 0b100: printf("INVALID");                   break;
			case 0b101: printf("INVALID");                   break;
			case 0b110: printf("INVALID");                   break;
			case 0b111: printf("INVALID");                   break;
		}  /*0b00100 end*/                                   break;
		case 0b00101:   switch((instruction >> 8) &  0b111)
		{
			case 0b000: printf("saa R%d", rf);               break;
			case 0b001: printf("sbe R%d", rf);               break;
			case 0b010: printf("scc R%d", rf);               break;
			case 0b011: printf("sge R%d", rf);               break;
			case 0b100: printf("sgg R%d", rf);               break;
			case 0b101: printf("sle R%d", rf);               break;
			case 0b110: printf("sll R%d", rf);               break;
			case 0b111: printf("snc R%d", rf);               break;
		}  /*0b00101 end*/                                   break;
		case 0b00110:   printf("dvu R%d, R%d", rf, rs);      break;
		case 0b00111:   printf("dvs R%d, R%d", rf, rs);      break;
		case 0b01000:   printf("prd R%d, R%d", rf, rs);      break;
		case 0b01001:   printf("pwr R%d, R%d", rs, rf);      break;
		case 0b01010:   printf("xrd R%d, %.2s", rf, xrd_xr); break;
		case 0b01011:   printf("xwr %.2s, R%d", xwr_xr, rs); break;
		case 0b01100:   printf("mrd R%d, R%d", rf, rs);      break;
		case 0b01101:   printf("mwr R%d, R%d", rs, rf);      break;
		case 0b01110:   printf("srd R%d, R%d", rf, rs);      break;
		case 0b01111:   printf("swr R%d, R%d", rs, rf);      break;
		case 0b10000:   printf("mlu R%d, R%d", rf, rs);      break;
		case 0b10001:   printf("cmp R%d, R%d", rf, rs);      break;
		case 0b10010:   printf("tst R%d, R%d", rf, rs);      break;
		case 0b10011:   printf("mls R%d, R%d", rf, rs);      break;
		case 0b10100:   printf("mov R%d, R%d", rf, rs);      break;
		case 0b10101:   printf("crd R%d, R%d", rf, rs);      break;
		case 0b10110:   printf("cwr R%d, R%d", rs, rf);      break;
		case 0b10111:   switch((instruction >> 8) &  0b111)
		{
			case 0b000: printf("sno R%d", rf);               break;
			case 0b001: printf("sns R%d", rf);               break;
			case 0b010: printf("snz R%d", rf);               break;
			case 0b011: printf("soo R%d", rf);               break;
			case 0b100: printf("sss R%d", rf);               break;
			case 0b101: printf("szz R%d", rf);               break;
			case 0b110: printf("INVALID");                   break;
			case 0b111: printf("INVALID");                   break;
		}  /*0b10111 end*/                                   break;
		case 0b11000:   printf("add R%d, R%d", rf, rs);      break;
		case 0b11001:   printf("sub R%d, R%d", rf, rs);      break;
		case 0b11010:   printf("and R%d, R%d", rf, rs);      break;
		case 0b11011:   printf("ann R%d, R%d", rf, rs);      break;
		case 0b11100:   printf("orr R%d, R%d", rf, rs);      break;
		case 0b11101:   printf("xor R%d, R%d", rf, rs);      break;
		case 0b11110:   printf("shl R%d, R%d", rf, rs);      break;
		case 0b11111:   printf("shr R%d, R%d", rf, rs);      break;
	}   /*0b00000 end*/                                      break;
	case 0b00001:       printf("INVALID");                   break;
	case 0b00010:       switch((instruction >> 8) & 0b111)
	{
		case 0b000: printf("int %d", imm8_l);                break;
		case 0b001: printf("INVALID");                       break;
		case 0b010: printf("crd R%d, %d", rf, imm5);         break;
		case 0b011: printf("cwr %d, R%d", imm5, rf);         break;
		case 0b100: printf("INVALID");                       break;
		case 0b101: printf("INVALID");                       break;
		case 0b110: printf("INVALID");                       break;
		case 0b111: printf("INVALID");                       break;
	}  /*0b00010 end*/                                       break;
	case 0b00011:       switch((instruction >> 8) & 0b111)
	{
		case 0b000: printf("prf %d", imm8_l);                break;
		case 0b001: printf("fls %d", imm8_l);                break;
		case 0b010: printf("INVALID");                       break;
		case 0b011: printf("INVALID");                       break;
		case 0b100: printf("INVALID");                       break;
		case 0b101: printf("INVALID");                       break;
		case 0b110: printf("INVALID");                       break;
		case 0b111: printf("INVALID");                       break;
	}  /*0b00011 end*/                                       break;
	case 0b00100: case 0b00101: switch((instruction >> 8) & 0b1111)
	{
		case 0b0000: printf("jaa %d", imm8_j);               break;
		case 0b0001: printf("jbe %d", imm8_j);               break;
		case 0b0010: printf("jcc %d", imm8_j);               break;
		case 0b0011: printf("jge %d", imm8_j);               break;
		case 0b0100: printf("jgg %d", imm8_j);               break;
		case 0b0101: printf("jle %d", imm8_j);               break;
		case 0b0110: printf("jll %d", imm8_j);               break;
		case 0b0111: printf("jnc %d", imm8_j);               break;
		case 0b1000: printf("jno %d", imm8_j);               break;
		case 0b1001: printf("jns %d", imm8_j);               break;
		case 0b1010: printf("jnz %d", imm8_j);               break;
		case 0b1011: printf("joo %d", imm8_j);               break;
		case 0b1100: printf("jss %d", imm8_j);               break;
		case 0b1101: printf("jzz %d", imm8_j);               break;
		case 0b1110: printf("INVALID");                      break; 
		case 0b1111: printf("INVALID");                      break; 
	}  /*0b0010- end*/                                       break;
	case 0b00110: printf("INVALID");                         break;
	case 0b00111: printf("INVALID");                         break;
	case 0b01000: printf("prd R%d, %d", rf, imm8);           break;
	case 0b01001: printf("pwr %d, R%d", imm8, rf);           break;
	case 0b01010: printf("jmp %d", imm11);                   break;
	case 0b01011: printf("xwr %.2s, %d", xwr_xr, imm8);      break;
	case 0b01100: printf("mrd R%d, %d", rf, imm8);           break;
	case 0b01101: printf("mwr %d, R%d", imm8, rf);           break;
	case 0b01110: printf("srd R%d, %d", rf, imm8);           break;
	case 0b01111: printf("swr %d, R%d", imm8, rf);           break;
	case 0b10000: printf("mlu R%d, %d", rf, imm8);           break;
	case 0b10001: printf("cmp R%d, %d", rf, imm8);           break;
	case 0b10010: printf("tst R%d, %d", rf, imm8);           break;
	case 0b10011: printf("mls R%d, %d", rf, imm8);           break;
	case 0b10100: printf("mov R%d, %d", rf, imm8);           break;
	case 0b10101: printf("cal %d", imm11);                   break;
	case 0b10110: switch((instruction >> 1) & 0b111) 
	{
		case 0b000: printf("maa R%d, R%d", rf, rs);          break;
		case 0b001: printf("mbe R%d, R%d", rf, rs);          break;
		case 0b010: printf("mcc R%d, R%d", rf, rs);          break;
		case 0b011: printf("mge R%d, R%d", rf, rs);          break;
		case 0b100: printf("mgg R%d, R%d", rf, rs);          break;
		case 0b101: printf("mle R%d, R%d", rf, rs);          break;
		case 0b110: printf("mll R%d, R%d", rf, rs);          break;
		case 0b111: printf("mnc R%d, R%d", rf, rs);          break;
	}  /*0b00110 end*/                                       break;
	case 0b10111: switch((instruction >> 1) & 0b111) 
	{
		case 0b000: printf("mno R%d, R%d", rf, rs);          break;
		case 0b001: printf("mns R%d, R%d", rf, rs);          break;
		case 0b010: printf("mnz R%d, R%d", rf, rs);          break;
		case 0b011: printf("moo R%d, R%d", rf, rs);          break;
		case 0b100: printf("mss R%d, R%d", rf, rs);          break;
		case 0b101: printf("mzz R%d, R%d", rf, rs);          break;
		case 0b110: printf("INVALID");                       break;
		case 0b111: printf("INVALID");                       break;
	}  /*0b00111 end*/                                       break;
	case 0b11000: printf("add R%d, %d", rf, imm8);           break;
	case 0b11001: printf("sub R%d, %d", rf, imm8);           break;
	case 0b11010: printf("and R%d, %d", rf, imm8);           break;
	case 0b11011: printf("ann R%d, %d", rf, imm8);           break;
	case 0b11100: printf("orr R%d, %d", rf, imm8);           break;
	case 0b11101: printf("xor R%d, %d", rf, imm8);           break;
	case 0b11110: printf("shl R%d, %d", rf, imm8);           break;
	case 0b11111: printf("shr R%d, %d", rf, imm8);           break;
	}
	printf("\n");

	return;
}
