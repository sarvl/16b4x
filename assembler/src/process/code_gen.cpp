#include "./process.h"

#include <iostream>

#include "top/gen_utils/log.h"
#include "top/utils.h"
#include "top/warning.h"

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
				result.emplace_back(0xAF00);
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


		auto const   ins    = static_cast<t_Instruction_Id::Type>(tokens[tid].val);
		unsigned int opcode = instruction_to_opcode(ins);
		unsigned int rf     = 0;
		unsigned int rs     = 0;
		unsigned int imm    = 0;
		unsigned int ccc    = 0;
		unsigned int socz   = 0;
		unsigned int res    = 0;

		std::string_view const file_name = tokens[tid].file_name;
		int              const line_num  = tokens[tid].line_num;

		using enum t_Instruction_Id::Type;
		//within instructions, stuff may get complicted 
		switch(ins)
		{
		//ins reg imm
		//ins reg reg 
		case iadd: case iand: case icmp: case idiv:
		case imov: case imrd: case imro: case imul:
		case inot: case iorr: case iprd: case ishl: 
		case ishr: case isub: case itst: case ixor: 
			rf = tokens[tid + 1].val;

			tid += 2;
			if(tokens[tid].type == t_Token::reg)
			{
				rs = tokens[tid].val;
				goto short_opcode_reg_reg;
			}

			imm = get_value(tokens, labels, tid);

			if(idiv == ins
			&& 0 == imm)
				Utils::warning("[divbyzero] divide by zero",
							   file_name, line_num,
							   Warning::divbyzero);
			goto short_opcode_reg_imm;
		//ins imm reg 
		//ins reg reg 
		case imwo: case imwr: case ipwr:
			rf = tokens[tid + 2].val;

			tid += 1;
			if(tokens[tid].type == t_Token::reg)
			{
				rs = tokens[tid].val;
				tid++;
				goto short_opcode_reg_reg;
			}

			imm = get_value(tokens, labels, tid);
			//skip the register too
			tid++;
			goto short_opcode_reg_imm;

		//ins ext imm
		//ins ext reg
		case ixwr:
			rf = tokens[tid + 1].val;

			if(tokens[tid + 2].type == t_Token::reg)
			{
				rs = tokens[tid + 2].val;
				tid += 2;
				goto short_opcode_reg_reg;
			}

			tid += 2;
			imm = get_value(tokens, labels, tid);
			goto short_opcode_reg_imm;
		//ins reg ext
		case ixrd:
			rf = tokens[tid + 1].val;
			rs = tokens[tid + 2].val;
			tid += 2;
			goto short_opcode_reg_reg;
		//ins imm
		case iint:
			tid += 1;
			imm = get_value(tokens, labels, tid);
			goto long_opcode_immediate;
		//ins imm
		//ins reg
		case ical: case ijmp: 
			if(tokens[tid + 1].type == t_Token::reg)
			{
				if(ical == ins)
					opcode = 0b00010101010;
				else
					opcode = 0b00010101011;

				rf = tokens[tid + 1].val;
				tid++;
				goto long_opcode_reg_imm;
			}

			tid++;

			imm = get_value(tokens, labels, tid, address);
			goto short_opcode_long_immediate;
		//ins ccc imm
		case ijcu: case ijcs:
			ccc = tokens[tid + 1].val;
			if((0b111 & ccc) == 0b111)
				Utils::warning("[alwaystrue] jump condition always true",
							   file_name, line_num,
				               Warning::alwaystrue,
				               "^ JMP allows for longer jumps and may perform better");

			tid += 2;
			imm = get_value(tokens, labels, tid, address);
			goto short_opcode_jmp_ccc;
		//ins 
		case ihcf: case ihlt: case iirt: case inop:
			tid++;
			goto long_opcode_immediate;
		//ins
		//ins socz
		case iret:
			//check whether next token on the same line
			if(tid + 1 < size
			&& tokens[tid + 1].line_num == tokens[tid + 0].line_num)
				socz = tokens[tid + 1].val;
			else
				socz = 0;	
			
			tid++;
			goto long_opcode_ret_flags;

		//ins ccc reg reg
		case imcs: case imcu:
			ccc = tokens[tid + 1].val;

			if((0b111 & ccc) == 0b111)
				Utils::warning("[alwaystrue] conditional move condition always true",
							   file_name, line_num,
				               Warning::alwaystrue,
				               "^ MOV may perform better");

			rf  = tokens[tid + 2].val;
			rs  = tokens[tid + 3].val;
			tid += 4;
			goto short_opcode_mov_ccc;
		//ins reg
		case ipop: case ipsh:
			rf = tokens[tid + 1].val;
			tid += 2;
			goto long_opcode_reg_imm;
		//ins reg imm
		case ipxr:
			rf = tokens[tid + 1].val;
			tid += 2;
			imm = get_value(tokens, labels, tid);
			goto long_opcode_reg_imm;
		//ins imm reg
		case ipxw:
			rf = tokens[tid + 2].val;
			tid += 1;
			imm = get_value(tokens, labels, tid);
			goto long_opcode_reg_imm;
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

	long_opcode_immediate:
		WARN_IMM_TOO_BIG(8)

		res = ((opcode & 0b1111'1111) << 8)
		    | ((imm    & 0b0111'1111) << 1)
		    | ((imm    & 0b1000'0000) >> 7)
		    ;

		goto insert;
	long_opcode_reg_imm:
		WARN_IMM_TOO_BIG(5)

		res = ((opcode & 0b1111'1111) << 8)
		    | ((rf     & 0b0000'0111) << 5) 
		    | ((imm    & 0b0000'1111) << 1) 
		    | ((imm    & 0b0001'0000) >> 4) 
		    ;

		goto insert;
	long_opcode_ret_flags:
		
		res = ((opcode & 0b1111'1111) << 8)
		    | ((socz   &      0b1111) << 4)
		    ;
		
		goto insert;
	short_opcode_long_immediate:
		WARN_IMM_TOO_BIG(11)

		res = ((opcode &        0b1'1111) << 11)
		    | ((imm    &  0b11'1111'1111) <<  1)
		    | ((imm    & 0b100'0000'0000) >> 10)
		    ;

		goto insert;
	short_opcode_jmp_ccc:
		WARN_IMM_TOO_BIG(8)


		res = ((opcode &    0b1'1111) << 11)
		    | ((imm    &  0b111'1111) <<  4)
		    | ((imm    & 0b1000'0000) >>  7)
		    | ((ccc    &       0b111) <<  1)
		    ;
		goto insert;
	short_opcode_mov_ccc:
		res = ((opcode &   0b1'1111) << 11)
		    | ((rs     &      0b111) <<  8)
		    | ((rf     &      0b111) <<  5)
		    | ((ccc    &      0b111) <<  1)
		    ;
		goto insert;
	short_opcode_reg_imm:
		WARN_IMM_TOO_BIG(8)

		res = ((opcode &    0b1'1111) << 11)
		    | ((rf     &       0b111) <<  5)
		    | ((imm    & 0b1000'0000) >>  7)
		    | ((imm    & 0b0111'0000) <<  4)
		    | ((imm    & 0b0000'1111) <<  1)
		    ;
		goto insert;
	short_opcode_reg_reg:
		res = ((opcode &   0b1'1111) <<  0)
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
