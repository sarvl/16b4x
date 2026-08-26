#include "./process.h"

#include <iostream>

#include "top/gen_utils/log.h"
#include "top/utils.h"
#include "top/warning.h"

#include "top/instruction_info.h"

using namespace std::literals;

void         strings_add(std::string const& str);
std::string& strings_get(int const ind);
std::string& strings_back();
int          strings_size();

static int get_value(
	std::vector<t_Token>  const& tokens,
	std::vector<t_Label>  const& labels,
	int                        & tid,
	//if addr >= 0 then it is used as offset for labels, used by control flow
	int                   const  addr = -1
	)
{
	if(t_Token::num == tokens[tid].type)
	{
		tid++;
		return tokens[tid - 1].val;
	}

	if(t_Token::ulb == tokens[tid].type)
	{
		std::string_view const cur_label = strings_get(tokens[tid].val);
		//find the string mapping 
		for(auto const& label : labels)
		{
			if(cur_label == label.name)
			{
				tid++;
				if(addr >= 0)
					return label.value - addr - 1;
				else
					return label.value;
			}
		}
		
		Log::error(
			"label \""s + std::string(cur_label) + "\" could not be found", 
			tokens[tid].file_name, tokens[tid].line_num);

		tid++;
		return 0;
	}

	if(t_Token::slb == tokens[tid].type)
	{
		if(-1 == tokens[tid].val)
		{
			Log::error(".< must point to some label", tokens[tid].file_name, tokens[tid].line_num);
			return 0;
		}
		if(static_cast<int>(labels.size()) == tokens[tid].val)
		{
			Log::error(".> must point to some label", tokens[tid].file_name, tokens[tid].line_num);
			return 0;
		}

		int const value = labels[tokens[tid].val].value;
		tid++;
		if(addr >= 0)
			return value - addr - 1;
		else
			return value;
	}

	if(t_Token::exs == tokens[tid].type)
		return evaluate_expr(tokens, labels, tid, addr);
	
	Log::internal_error("invalid token in get_value");
	return 0;
}

void code_gen(
	std::vector<uint16_t>      & result,
	std::vector<t_Token>  const& tokens,
	std::vector<t_Label>  const& labels
	)
{
	int tid = 0;
	int address = 0; //so that it starts at 0
	int const size = static_cast<int>(tokens.size());
	while(true)
	{
		//skip until instruction or directive
		while(tid < size
		   && tokens[tid].type != t_Token::ins
		   && tokens[tid].type != t_Token::dir)
			tid++;
		if(tid >= size)
			break;

		 //only few are allowed
		if(t_Token::dir == tokens[tid].type) switch(static_cast<t_Directive::Type>(tokens[tid].val))
		{
		case t_Directive::org:
			address = tokens[tid + 1].val;
			tid += 2;
			continue;
		case t_Directive::adr:
		{
			//already verified
			int const to_get_to = tokens[tid + 1].val;
			tid += 2;

			while(address < to_get_to)
			{
				result.emplace_back(0);
				address++;
			}
			continue;
		}
		case t_Directive::alg:
		{
			//already verified
			int const alignment = tokens[tid + 1].val;

			while(address % alignment != 0)
			{
				result.emplace_back(0x0006 /*NOP*/);
				address++;
			}
			tid++;
			continue;
		}
		case t_Directive::cas:
		{
			tid++;
			int const cur_line = tokens[tid].line_num;	
			//if true, skip to next line
			if(get_value(tokens, labels, tid))
			{
				while(cur_line == tokens[tid].line_num)
					tid++;
				continue;
			}
			
			//either str or %tostring 
			if(t_Token::str == tokens[tid].type)
			{
				Log::error(strings_get(tokens[tid].val));
				tid++;
				continue;
			}

			//the only allowed is %tostring
			if(t_Token::dir == tokens[tid].type)
			{
				auto const file_name = tokens[tid].file_name;
				auto const line_num  = tokens[tid].line_num;
				tid++;
				switch(tokens[tid].type)
				{
				case t_Token::str:
					Log::error(strings_get(tokens[tid].val), file_name, line_num);
					tid++;
					break;
				case t_Token::ins:
					Log::error(to_string(static_cast<t_Instruction_Id::Type>(tokens[tid].val)), file_name, line_num);
					tid++;
					break;
				case t_Token::num: case t_Token::ulb: case t_Token::exs:
					Log::error(std::to_string(get_value(tokens, labels, tid)), file_name, line_num);
					break;
				case t_Token::dir:
					Log::error(to_string(static_cast<t_Directive::Type>(tokens[tid].val)), file_name, line_num);
					tid++;
					break;
				default:
					Log::error("tostring does not support this argument yet", file_name, line_num);
					while(line_num == tokens[tid].line_num)
						tid++;
					break;
				}
			}
			break;
		}
		case t_Directive::ddw:
		{
			auto const file_name = tokens[tid].file_name;
			auto const line_num  = tokens[tid].line_num;
			tid++;
			int const val = get_value(tokens, labels, tid);
			if(val > 0xFFFF)
				Utils::warning("[immnofit] value "s + std::to_string(val) + " does not fit into 16bits", file_name, line_num, Warning::immnofit, "split into multiple values");
			result.emplace_back(val);
			address++;
			continue;
		}
		default:
			Log::internal_error("unhandled directive in code gen", tokens[tid].file_name, tokens[tid].line_num);
			tid++;
			continue;
		}

		auto const   ins    = static_cast<t_Instruction_Id::Type>(tokens[tid].val & 0xFF);
		unsigned int ins_base = 0;
		unsigned int rf       = 0;
		unsigned int rs       = 0;
		unsigned int imm      = 0;
		unsigned int res      = 0;

		std::string_view const file_name = tokens[tid].file_name;
		int              const line_num  = tokens[tid].line_num;

		using enum t_Instruction_Id::Type;
		switch(ins)
		{
		//ins reg reg/imm8
		case iadd: case iand: case iann: case icmp: case imlu:
		case imls: case imov: case imrd: case iorr: case iprd: 
		case ishl: case ishr: case isrd: case isub: case itst: 
		case ixor: case ixwr:
			if(t_Token::reg == tokens[tid + 2].type)
			{
		[[fallthrough]];
		//ins reg reg 
		case idvu: case idvs: case imaa: case imbe: case imbz: 
		case imcc: case imae: case imaz: case imge: case imgz:
		case imgg: case imle: case imlz: case imll: case imnc:
		case imbb: case imno: case imns: case imnz: case imne:
		case imoo: case imss: case imzz: case imee: case ixrd:
				ins_base = instr_reg[static_cast<int>(ins)];
				rf       = tokens[tid + 1].val;

				tid     += 2;
				rs       = tokens[tid].val;
			//technically, they are not the same format but ins_base takes care of that 
				goto encode_so_rr;
			}
			else
			{
				ins_base = instr_imm[static_cast<int>(ins)];
				rf       = tokens[tid + 1].val;

				tid      += 2;
				imm      = get_value(tokens, labels, tid);
				goto encode_so_ri;
			}
		//ins reg/imm11
		case ical:
			if(t_Token::reg == tokens[tid + 1].type)
			{
				ins_base = instr_reg[static_cast<int>(ins)];

				tid     += 1;
				rf       = tokens[tid].val;
				goto encode_lo_r;
			}
			else
			{
		[[fallthrough]];
		//ins imm11
		case ijmp:
				ins_base = instr_imm[static_cast<int>(ins)];

				tid     += 1;
				imm      = get_value(tokens, labels, tid, address);
				goto encode_so_li;
			}

		//ins reg reg/imm5
		case icrd:
			if(t_Token::reg == tokens[tid + 2].type)
			{
				ins_base = instr_reg[static_cast<int>(ins)];

				tid     += 1;
				rf       = tokens[tid    ].val;
				rs       = tokens[tid + 1].val;
				goto encode_so_rr;
			}
			else
			{
				ins_base = instr_imm[static_cast<int>(ins)];

				rf       = tokens[tid + 1].val;
				tid     += 2;
				imm      = get_value(tokens, labels, tid);
				goto encode_mo_ri;
			}
		//ins reg/imm5 reg
		case icwr:
			if(t_Token::reg == tokens[tid + 1].type)
			{
				ins_base = instr_reg[static_cast<int>(ins)];

				tid     += 1;
				rf       = tokens[tid + 1].val;
				rs       = tokens[tid    ].val;
				goto encode_so_rr;
			}
			else
			{
				ins_base = instr_imm[static_cast<int>(ins)];

				tid     += 1;
				imm      = get_value(tokens, labels, tid);
				rf       = tokens[tid].val;
				goto encode_mo_ri;
			}
		//ins reg/imm8 reg
		case imwr: case iswr: case ipwr:
			if(t_Token::reg == tokens[tid + 1].type)
			{
				ins_base = instr_reg[static_cast<int>(ins)];

				tid     += 1;
				rf       = tokens[tid + 1].val;
				rs       = tokens[tid    ].val;
				goto encode_so_rr;
			}
			else
			{
				ins_base = instr_imm[static_cast<int>(ins)];

				tid     += 1;
				imm      = get_value(tokens, labels, tid);
				rf       = tokens[tid].val;
				goto encode_so_ri;
			}
		//ins
		case ihlt: case iirt: case inop: case iret:
			tid++;
			ins_base = instr_imm[static_cast<int>(ins)];
			goto encode_lo_r;
		//ins reg
		case isaa: case isbe: case isbz: case iscc: case isae:
		case isaz: case isge: case isgz: case isgg: case isle:
		case islz: case isll: case isnc: case isbb: case isno:
		case isns: case isnz: case isne: case isoo: case isss:
		case iszz: case isee: case ineg: case inot: case ipop:
		case ipsh: case irng: case iprf:
			ins_base = instr_reg[static_cast<int>(ins)];

			tid     += 1;
			rf       = tokens[tid].val;

			//technically, they are not the same format but ins_base takes care of that 
			goto encode_lo_r;

		//ins imm8 with label
		case ijaa: case ijbe: case ijbz: case ijcc: case ijae: 
		case ijaz: case ijge: case ijgz: case ijgg: case ijle: 
		case ijlz: case ijll: case ijnc: case ijbb: case ijno: 
		case ijns: case ijnz: case ijne: case ijoo: case ijss:
		case ijzz: case ijee: case ilop:
			ins_base = instr_imm[static_cast<int>(ins)];

			tid     += 1;
			imm      = get_value(tokens, labels, tid, address);
			//technically, they are not the same format but ins_base takes care of that 
			goto encode_mo_i;
		//ins imm8
		case iint:
			ins_base = instr_imm[static_cast<int>(ins)];

			tid     += 1;
			imm      = get_value(tokens, labels, tid);
			//technically, they are not the same format but ins_base takes care of that 
			goto encode_mo_i;

		}

	#define WARN_IMM_TOO_BIG(BIT_COUNT)\
	{\
		if((static_cast<int32_t>(imm) >  (1 << (BIT_COUNT)) - 1)\
		|| (static_cast<int32_t>(imm) < -(1 << ((BIT_COUNT) - 1))))\
		{\
			Utils::warning(\
				"[immnofit] immediate does not fit in "#BIT_COUNT" bits",\
				file_name, line_num,\
				Warning::immnofit,\
				"prepend XWR UI or toggle switch so SASM can do it automatically");\
		}\
	}\

	encode_lo_r:
		res = ( ins_base                  )
		    | ((rf     & 0b0000'0111) << 5) 
		    ;
		
		goto insert;
	encode_mo_i:
		WARN_IMM_TOO_BIG(8)
		res = ( ins_base                  )
		    | ((imm    & 0b1111'1111) << 0)
		    ;

		goto insert;
	encode_mo_ri:
		WARN_IMM_TOO_BIG(5)

		res = ( ins_base                  )
		    | ((rf     & 0b0000'0111) << 5) 
		    | ((imm    & 0b0001'1111) << 0) 
		    ;

		goto insert;
	encode_so_li:
		WARN_IMM_TOO_BIG(11)

		res = ( ins_base                       )
		    | ((imm    & 0b111'1111'1111) <<  0)
		    ;

		goto insert;
	encode_so_ri:
		WARN_IMM_TOO_BIG(8)

		res = ( ins_base                   )
		    | ((rf     &       0b111) <<  5)
		    | ((imm    & 0b1110'0000) <<  3)
		    | ((imm    & 0b0001'1111) <<  0)
		    ;
		goto insert;
	encode_so_rr:
		res = ( ins_base                  )
		    | ((rs     &      0b111) <<  8)
		    | ((rf     &      0b111) <<  5)
		    ;
		goto insert;

	insert:
		address++;
		result.emplace_back(res);
		
		//no need to increment anything
		//start of loop does it already and nothing points to ins/dir
		continue;
	}


	return;
}
