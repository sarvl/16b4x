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
				result.emplace_back(0x0003);
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

		//0 indicates that particular entry is NEVER to be used
		//takes care of aligning opcode properly
		constexpr static uint16_t instr_imm[] = {
		//	iadd,  iand,  iann,  ical,  icmp,  icrd,  icwr,  idvu,
			0xC000,0xD000,0xD800,0xA800,0x8800,0x1200,0x1300,   0  ,
		//	idvs,  ifls,  ihlt,  iint,  iirt,  ijmp,  imls,  imlu,  
			 0    ,0x1900,0x0101,0x1000,0x0001,0x5000,0x9800,0x8000,
		//	imov,  imrd,  imwr,  ineg,  inop,  inot,  iorr,  ipop,
			0xA000,0x6000,0x6800,   0  ,0x0003,   0  ,0xE000,  0   ,
		//	iprd,  iprf,  ipsh,  ipwr,  iret,  irng,  ishl,  ishr,  
			0x4000,0x1800,   0  ,0x4800,0x0503,   0  ,0xF000,0xF800,
		//	isrd,  isub,  iswr,  itst,  ixor,  ixrd,  ixwr,
			0x7000,0xC800,0x7800,0x9000,0xE800,   0  ,0x5800,

		//	ijaa,  ijbe,  ijbz,  ijcc,  ijae,  ijaz,  ijge,  ijgz,
			0x2000,0x2100,0x2100,0x2200,0x2200,0x2200,0x2300,0x2300, 
		//	ijgg,  ijle,  ijlz,  ijll,  ijnc,  ijbb,  ijno,  ijns,
			0x2400,0x2500,0x2500,0x2600,0x2700,0x2700,0x2800,0x2900, 
		//	ijnz,  ijne,  ijoo,  ijss,  ijzz,  ijee,  
			0x2A00,0x2A00,0x2B00,0x2C00,0x2D00,0x2D00, 

		//	imaa,  imbe,  imbz,  imcc,  imae,  imaz,  imge,  imgz,
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
		//	imgg,  imle,  imlz,  imll,  imnc,  imbb,  imno,  imns,
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
		//	imnz,  imne,  imoo,  imss,  imzz,  imee,  
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,

		//	isaa,  isbe,  isbz,  iscc,  isae,  isaz,  isge,  isgz,
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
		//	isgg,  isle,  islz,  isll,  isnc,  isbb,  isno,  isns,
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
		//	isnz,  isne,  isoo,  isss,  iszz,  isee
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  
		};
		constexpr static uint16_t instr_reg[] = {
		//	iadd,  iand,  iann,  ical,  icmp,  icrd,  icwr,  idvu,
			0x0018,0x001A,0x001B,0x0403,0x0011,0x0015,0x0016,0x0006,
		//	idvs,  ifls,  ihlt,  iint,  iirt,  ijmp,  imls,  imlu,  
			0x0007,0x0104,   0  ,   0  ,   0  ,   0  ,0x0013,0x0010,
		//	imov,  imrd,  imwr,  ineg,  inop,  inot,  iorr,  ipop,
			0x0014,0x000C,0x000D,0x0703,   0  ,0x0603,0x001C,0x0303,
		//	iprd,  iprf,  ipsh,  ipwr,  iret,  irng,  ishl,  ishr,  
			0x0008,0x0004,0x0203,0x0009,   0  ,0x0103,0x001E,0x001F,
		//	isrd,  isub,  iswr,  itst,  ixor,  ixrd,  ixwr,
			0x000E,0x0019,0x000F,0x0012,0x001D,0x000A,0x000B,

		//	ijaa,  ijbe,  ijbz,  ijcc,  ijae,  ijaz,  ijge,  ijgz,
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
		//	ijgg,  ijle,  ijlz,  ijll,  ijnc,  ijbb,  ijno,  ijns,
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,
		//	ijnz,  ijne,  ijoo,  ijss,  ijzz,  ijee,  
			   0  ,   0  ,   0  ,   0  ,   0  ,   0  ,

		//	imaa,  imbe,  imbz,  imcc,  imae,  imaz,  imge,  imgz,
			0xB000,0xB002,0xB002,0xB004,0xB004,0xB004,0xB006,0xB006,
		//	imgg,  imle,  imlz,  imll,  imnc,  imbb,  imno,  imns,
			0xB008,0xB00A,0xB00A,0xB00C,0xB00E,0xB00E,0xB800,0xB802,
		//	imnz,  imne,  imoo,  imss,  imzz,  imee,  
			0xB804,0xB804,0xB806,0xB808,0xB80A,0xB80A,

		//	isaa,  isbe,  isbz,  iscc,  isae,  isaz,  isge,  isgz,
			0x0005,0x0105,0x0105,0x0205,0x0205,0x0205,0x0305,0x0305,
		//	isgg,  isle,  islz,  isll,  isnc,  isbb,  isno,  isns,
			0x0405,0x0505,0x0505,0x0605,0x0705,0x0705,0x0017,0x0117,
		//	isnz,  isne,  isoo,  isss,  iszz,  isee
			0x0217,0x0217,0x0317,0x0417,0x0517,0x0517
		};


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
		case iadd: case iand: case iann: case icmp: case imls: 
		case imlu: case imov: case imrd: case iorr: case iprd: 
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
		//ins reg/imm8
		case iprf: case ifls:
			if(t_Token::reg == tokens[tid + 1].type)
			{
		[[fallthrough]];
		//ins reg
		case isaa: case isbe: case isbz: case iscc: case isae:
		case isaz: case isge: case isgz: case isgg: case isle:
		case islz: case isll: case isnc: case isbb: case isno:
		case isns: case isnz: case isne: case isoo: case isss:
		case iszz: case isee: case ineg: case inot: case ipop:
		case ipsh: case irng:
				ins_base = instr_reg[static_cast<int>(ins)];

				tid     += 1;
				rf       = tokens[tid].val;

			//technically, they are not the same format but ins_base takes care of that 
				goto encode_lo_r;
			}
			else
			{
		[[fallthrough]];
		//ins imm8 with label
		case ijaa: case ijbe: case ijbz: case ijcc: case ijae: 
		case ijaz: case ijge: case ijgz: case ijgg: case ijle: 
		case ijlz: case ijll: case ijnc: case ijbb: case ijno: 
		case ijns: case ijnz: case ijne: case ijoo: case ijss:
		case ijzz: case ijee:
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
		    | ((imm    & 0b0111'1111) << 1)
		    | ((imm    & 0b1000'0000) >> 7)
		    ;

		goto insert;
	encode_mo_ri:
		WARN_IMM_TOO_BIG(5)

		res = ( ins_base                  )
		    | ((rf     & 0b0000'0111) << 5) 
		    | ((imm    & 0b0000'1111) << 1) 
		    | ((imm    & 0b0001'0000) >> 4) 
		    ;

		goto insert;
	encode_so_li:
		WARN_IMM_TOO_BIG(11)

		res = ( ins_base                       )
		    | ((imm    &  0b11'1111'1111) <<  1)
		    | ((imm    & 0b100'0000'0000) >> 10)
		    ;

		goto insert;
	encode_so_ri:
		WARN_IMM_TOO_BIG(8)

		res = ( ins_base                   )
		    | ((rf     &       0b111) <<  5)
		    | ((imm    & 0b1000'0000) >>  7)
		    | ((imm    & 0b0111'0000) <<  4)
		    | ((imm    & 0b0000'1111) <<  1)
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
