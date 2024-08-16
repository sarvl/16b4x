#include "./process.h"

#include <iostream>

#include "top/types/types.h"
#include "top/gen_utils/log.h"
#include "top/utils.h"

using namespace std::literals;

void         strings_add(std::string const& str);
std::string& strings_get(int const ind);
std::string& strings_back();
int          strings_size();

static int ccc_to_int(std::string_view const cc)
{
	int res = 0;
	for(char const c : cc)
	switch(c)
	{
	case 'L': case 'l':
		res |= 0b0100;
		break;
	case 'Z': case 'z': case 'E': case 'e':
		res |= 0b0010;
		break;
	case 'G': case 'g':
		res |= 0b0001;
		break;

	default:
		return -1;
	}

	return res;
}

static int socz_to_int(std::string_view const socz)
{
	int res = 0;
	for(char const c : socz)
	switch(c)
	{
	case 'S': case 's': case 'N': case 'n':
		res |= 0b1000;
		break;
	case 'O': case 'o':
		res |= 0b0100;
		break;
	case 'C': case 'c':
		res |= 0b0010;
		break;
	case 'Z': case 'z': case 'E': case 'e':
		res |= 0b0001;
		break;
	default:
		return -1;
	}

	return res;
}

//does not need to check for start/end
//this part has been verified by tokenizer
//checks whether tokens consist of 
//	num, udf, uvr, exo
// or dir-tof x 
// or dir-sam x x
// or dir-isd udf
// or dir-isa uvr
static bool verify_expression(
	std::vector<t_Token>      & output,
	std::vector<t_Token> const& tokens,
	int                       & tid,
	bool                 const  constant_expression
	)
{
	//starts with exs
	output.emplace_back(tokens[tid]);
	tid++;
	using enum t_Token::Type;
	while(true) switch(tokens[tid].type)
	{
	case num: case udf: case uvr: case mpr:
		output.emplace_back(tokens[tid]);
		tid++;
		continue;
	case exo:
		if('$' == tokens[tid].val && constant_expression)
		{
			Log::error("'$' not allowed in constant expression");
			while(exe != tokens[tid].type)
				tid++;
			tid++;
			return false;
		}
		else
		{
			output.emplace_back(tokens[tid]);
			tid++;
			continue;
		}
	//embedded expression
	case exs:
		if(false == verify_expression(output, tokens, tid, constant_expression))
			return false;
		continue;
	case mus:
		//thanks to tokenizer pattern is correct and ')' is before '}'
		//not implemented yet 
		tid++;
		continue;
	case slb:
		if(constant_expression)
			goto invalid_token;
		if('>' == tokens[tid].val || '<' == tokens[tid].val)
		{
			output.emplace_back(tokens[tid]);
			tid++;
			continue;
		}
		Log::error("., .. can only be used as label definitions", tokens[tid].file_name, tokens[tid].line_num);	
		tid++;
		break;
	case ulb:
		if(constant_expression)
			goto invalid_token;
		output.emplace_back(tokens[tid]);
		tid++;
		continue;
	case dir:
		switch(static_cast<t_Directive::Type>(tokens[tid].val))
		{
		case t_Directive::tof:
			output.emplace_back(tokens[tid]);
			tid++;
			if(exe == tokens[tid].type)
			{
				Log::error(
					"%typeof expects argument, EXE found instead",
					tokens[tid].file_name,
					tokens[tid].line_num
					);
				while(exe != tokens[tid].type)
					tid++;
				tid++;
				return false;
			}
			else if(exs == tokens[tid].type)
			{
				int depth = -1;
				while(true)
				{
					output.emplace_back(tokens[tid]);

					if(exs == tokens[tid].type)
						depth++;
					else if(exe == tokens[tid].type)
					{
						if(0 == depth)
						{
							tid++;
							break;
						}
						depth--;
					}
					tid++;
				}
			}
			else
			{
				output.emplace_back(tokens[tid]);
				tid++;
			}
			continue;

		case t_Directive::sam:
			output.emplace_back(tokens[tid]);
			tid++;
			for(int i = 0; i < 2; i++)
			{
				if(exe == tokens[tid].type)
				{
					Log::error(
						"%same expects argument, EXE found instead",
						tokens[tid].file_name,
						tokens[tid].line_num
						);
					while(exe != tokens[tid].type)
						tid++;
					tid++;
					return false;
				}
				else if(exs == tokens[tid].type)
				{
					int depth = -1;
					while(true)
					{
						output.emplace_back(tokens[tid]);

						if(exs == tokens[tid].type)
							depth++;
						else if(exe == tokens[tid].type)
						{
							if(0 == depth)
							{
								tid++;
								break;
							}
							depth--;
						}
						tid++;
					}
				}
				else
				{
					output.emplace_back(tokens[tid]);
					tid++;
				}
			}
			continue;
			
		case t_Directive::isd:
			tid++;

			if(udf != tokens[tid].type)
			{
				Log::error(
					"%isdefined expects define (prefixed with #) used",
					tokens[tid].file_name,
					tokens[tid].line_num
					);
				while(exe != tokens[tid].type)
					tid++;
				tid++;
				return false;
			}
			tid++;
			output.emplace_back(tokens[tid - 2]);
			output.emplace_back(tokens[tid - 1]);
			continue;

		case t_Directive::isa:
			tid++;

			if(uvr != tokens[tid].type)
			{
				Log::error(
					"%isvariable expects variable (prefixed with $) used",
					tokens[tid].file_name,
					tokens[tid].line_num
					);
				while(exe != tokens[tid].type)
					tid++;
				tid++;
				return false;
			}
			tid++;
			output.emplace_back(tokens[tid - 2]);
			output.emplace_back(tokens[tid - 1]);
			continue;

		default:
			Log::error(
				"Invalid directive "s + to_string(static_cast<t_Directive::Type>(tokens[tid].val)) + " in expression\n       "
				"valid directives are: %typeof, %same, %isdefined, %isassigned",
				tokens[tid].file_name,
				tokens[tid].line_num
				);
			while(exe != tokens[tid].type)
				tid++;
			tid++;
			return false;
					
		}
	case exe:
		output.emplace_back(tokens[tid]);
		tid++;
		return true;
	default:
	invalid_token:
		if(constant_expression)
		{
			if(ulb == tokens[tid].type
			|| slb == tokens[tid].type)
				Log::error(
					"Invalid token in constant expression: ulb of value \""s + strings_get(tokens[tid].val) + "\"\n       "
					"valid tokens are: NUM/DIR/UDF/UVR/EXO/EXS",
					tokens[tid].file_name,
					tokens[tid].line_num);
			else
				Log::error(
					"Invalid token in constant expression: "s + to_string(tokens[tid].type) + "\n       "
					"valid tokens are: NUM/DIR/UDF/UVR/EXO/EXS",
					tokens[tid].file_name,
					tokens[tid].line_num);
		}
		else
			Log::error(
				"Invalid token "s + to_string(tokens[tid].type) + " in expression\n       "
				"valid tokens are: NUM/DIR/UDF/ULB/UVR/EXO/EXS",
				tokens[tid].file_name,
				tokens[tid].line_num);
		while(exe != tokens[tid].type)
			tid++;
		tid++;
		return false;
	}
}

void verify(
	std::vector<t_Token>      & output,
	std::vector<t_Token> const& tokens
	)
{
	Log::is_error = false;
	Log::is_warning = false;

	#define ADVANCE_TO_NEXT_LINE\
		while(tid < size\
		   && tokens[tid].line_num == cur_line_num)\
		  	tid++;

	int address = 0;
	int tid = 0;
	//2B tokens is unlikely
	int const size = tokens.size();

	//verify first
	while(tid < size)
	{
		int const cur_line_num = tokens[tid].line_num;
		std::string_view const file_name = tokens[tid].file_name;

		switch(tokens[tid].type)
		{
		//macros
		case t_Token::mst:
		case t_Token::men: case t_Token::mus: case t_Token::mpr:
		case t_Token::dlb:
			output.emplace_back(tokens[tid]);
			tid++;
			ADVANCE_TO_NEXT_LINE;
			break;
		case t_Token::dir:
		{
			/* defines what each directive has to verify
			 * exp = exs ... exe
			 * 's' = string
			 * 'l' = label
			 * 'd' = define
			 * 'f' = def | ulb WITH NO DOTS
			 * 'r' = var | ulb WITH NO DOTS
			 * 'a' = assign
			 * 'x' = any
			 * 'i' = num | udf | uvr | constant exp
			 * 'v' = 'i' | ulb | exp
			 * for convenience, each entry is 2 chars
			 * but ' ' indicates that no more need to be parsed
			 * at any point, mus followed by arg list is a valid token too
			 */
			constexpr static char const* const pattern_table[] = {
			//	 inc, alg, adr, ddw, rps, 
				"s ","i ","i ","v ","i ",
			//	 rpe, wst, wns, wes, wph, wpp, 
				"  ","s ","s ","s ","  ","  ",
			//	 def, ass, isd, cif, cel, cen, 
				"fi","ri","d ","i ","  ","  ",
			//	 cas, inf, war, err  tof, isa
				"vs","s ","s ","s ","x ","a ",
			//	 sam  cei  tst
				"xx","i ","x "
				};

			const char* pattern = pattern_table[tokens[tid].val];

			int const first_line = tokens[tid].line_num;

			std::string const pattern_err = [&](){
				std::string ret = "";
				
				for(int i = 0; i < 3; i++) switch(pattern[i])
				{
				case ' ': return ret;
			 	case 's': ret += "STR ";                     continue;
			 	case 'f': ret += "ULB/DEF ";                 continue;
			 	case 'r': ret += "ULB/VAR ";                 continue;
			 	case 'd': ret += "UDF ";                     continue;
			 	case 'a': ret += "UVR ";                     continue;
			 	case 'x': ret += "ANY ";                     continue;
			 	case 'i': ret += "IMM/UDF/UVR/CEX ";         continue;
			 	case 'v': ret += "IMM/UDF/UVR/ULB/EXP ";     continue;
				default:  ret += "invalid pattern supplied ";continue;
				}

				return ret;
			}();

			output.emplace_back(tokens[tid]);
			tid++;
			for(int i = 0; i < 2; i++) 
			{
				if(' ' == pattern[i])
					break;
				
				if(tid == size
				|| tokens[tid].line_num != first_line)
				{
					Log::error(
					    "incomplete directive or directive split over multiple lines\n       "
						"expected DIR "s + pattern_err,
						file_name, cur_line_num);

					ADVANCE_TO_NEXT_LINE
					goto next_line;
				}

			//	if(t_Token::mus == tokens[tid].type)
			//	{
			//		output.emplace_back(tokens[tid]);
			//		tid++;
			//		if(tid != size && t_Token::prl == tokens[tid].type)
			//		{
			//			if(verify_arg_list(output, tokens, tid, false))
			//				continue;
			//			
			//			ADVANCE_TO_NEXT_LINE;
			//		}
			//		continue;
			//	}

				switch(pattern[i])
				{
				#define SINGLE_TOKEN(c, tok, str) \
				case c:\
					if(tok == tokens[tid].type) {output.emplace_back(tokens[tid]); tid++; continue; }\
					Log::error("unexpected token, expected " str " but got "s + to_string(tokens[tid].type) + "\n       "\
						"expected DIR "s + pattern_err,\
						file_name, tokens[tid].line_num);\
					ADVANCE_TO_NEXT_LINE\
					goto next_line;

				SINGLE_TOKEN('d', t_Token::udf, "UDF");
				SINGLE_TOKEN('a', t_Token::uvr, "UVR");
				#undef SINGLE_TOKEN

				case 'r':
					if(t_Token::uvr == tokens[tid].type)
						{output.emplace_back(t_Token::ulb, tokens[tid].val, tokens[tid].file_name, tokens[tid].line_num); tid++; continue; }
					if(t_Token::ulb == tokens[tid].type)
					{
						std::string_view str = strings_get(tokens[tid].val);

						for(unsigned i = 0; i < str.size(); i++) if('.' == str[i])
						{
							Log::error("define name cannot contain dots\n       "s
								"expected DIR "s + pattern_err,
								file_name, tokens[tid].line_num);
							ADVANCE_TO_NEXT_LINE
							goto next_line;
						}

						output.emplace_back(t_Token::ulb, tokens[tid].val, tokens[tid].file_name, tokens[tid].line_num); tid++; continue; 
					}

					Log::error("assign expects name with no dots optionally prepended with $\n       "s
						"expected DIR "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				case 'f':
					if(t_Token::udf == tokens[tid].type)
						{output.emplace_back(t_Token::ulb, tokens[tid].val, tokens[tid].file_name, tokens[tid].line_num); tid++; continue; }
					if(t_Token::ulb == tokens[tid].type)
					{
						std::string_view str = strings_get(tokens[tid].val);

						for(unsigned i = 0; i < str.size(); i++) if('.' == str[i])
						{
							Log::error("assign name cannot contain dots\n       "s
								"expected DIR "s + pattern_err,
								file_name, tokens[tid].line_num);
							ADVANCE_TO_NEXT_LINE
							goto next_line;
						}

						output.emplace_back(t_Token::ulb, tokens[tid].val, tokens[tid].file_name, tokens[tid].line_num); tid++; continue; 
					}

					Log::error("define expects name with no dots optionally prepended with #\n       "s
						"expected DIR "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				case 'l':
					if(t_Token::ulb == tokens[tid].type) 
					{output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::slb == tokens[tid].type)
					{
						if('>' == tokens[tid].val || '<' == tokens[tid].val)
							{output.emplace_back(tokens[tid]); tid++; continue; }

						Log::error("., .. can only be used as label definitions", tokens[tid].file_name, tokens[tid].line_num);	
						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}
					Log::error("unexpected token, expected ULB but got "s + to_string(tokens[tid].type) + "\n       "
						"expected DIR "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 's':
					if(t_Token::str == tokens[tid].type) 
						{output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::dir     == tokens[tid].type
					&& t_Directive::tst == tokens[tid].val)
					{
						tid++;
						if(size == tid)
						{
							Log::error("%tostring expects argument", file_name, first_line);
							ADVANCE_TO_NEXT_LINE;
							break;
						}
						output.emplace_back(tokens[tid - 1]); 

						if(t_Token::exs == tokens[tid].type)
						{
							verify_expression(output, tokens, tid, true);

							ADVANCE_TO_NEXT_LINE
							continue;

						}
						output.emplace_back(tokens[tid]); tid += 1; 
						continue; 
					}

					Log::error("unexpected token, expected STR but got "s + to_string(tokens[tid].type) + "\n       "
						"expected DIR "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 'x':
					if(size == tid)
					{
						Log::error("expected token as argument", file_name, first_line);
						return;
					}
					if(t_Token::exs == tokens[tid].type)
					{
						verify_expression(output, tokens, tid, false);

						ADVANCE_TO_NEXT_LINE
						continue;

					}
					output.emplace_back(tokens[tid]);
					tid++;
					break;

				case 'i':
					if(t_Token::num == tokens[tid].type
					|| t_Token::udf == tokens[tid].type
					|| t_Token::uvr == tokens[tid].type)
					{ output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::exs == tokens[tid].type)
					{
						if(verify_expression(output, tokens, tid, true))
							continue;

						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}

					Log::error(
					    "unexpected token, expected IMM/UDF/UVR/CEX but got "s + to_string(tokens[tid].type) + "\n       "
						"expected DIR "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 'v':
					if(t_Token::num == tokens[tid].type
					|| t_Token::udf == tokens[tid].type
					|| t_Token::uvr == tokens[tid].type
					|| t_Token::ulb == tokens[tid].type)
					{ output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::slb == tokens[tid].type)
					{
						if('>' == tokens[tid].val || '<' == tokens[tid].val)
							{output.emplace_back(tokens[tid]); tid++; continue; }

						Log::error("., .. can only be used as label definitions", tokens[tid].file_name, tokens[tid].line_num);	
						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}
					if(t_Token::exs == tokens[tid].type)
					{
						if(verify_expression(output, tokens, tid, false))
							continue;

						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}

					Log::error(
					    "unexpected token, expected IMM/UDF/UVR/ULB/EXP but got "s + to_string(tokens[tid].type) + "\n       "
						"expected DIR "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				}
			}
			if(tid != size
			&& tokens[tid].line_num == first_line)
			{
				Utils::warning("[extradata] extra data after directive",
							   file_name, cur_line_num,
							   Warning::extradata);
				ADVANCE_TO_NEXT_LINE
			}
			break;
		}

		case t_Token::ins:
		{
			/* defines what each instruction has to verify
			 * exp = exs ... exe
			 * 'r' = reg
			 * 'x' = ext
			 * 'i' = num | udf | uvr | constant exp
			 * 'v' = 'i' | ulb | exp
			 * 'o' = 'r' | 'v'
			 * 'a' = 'r' | 'i'
			 * 'c' = CCC
			 * 'f' = FLA | nothing
			 * for convenience, each entry is 3 chars
			 * but ' ' indicates that no more need to be parsed
			 * 'f' allows for nothing as only RET uses it
			 */
			constexpr static char const* const pattern_table[] = {
			//	iadd, iand, ical, icmp, idiv, ihcf,
				"ro ","ro ","o  ","ro ","ro ","   ",
			//	ihlt, iint, iirt, ijcs, ijcu, ijmp,
				"   ","i  ","   ","cv ","cv ","o  ",
			//	imcs, imcu, imov, imrd, imro, imul,
				"crr","crr","ro ","ro ","ro ","ro ",
			//	imwo, imwr, inot, iorr, ipop, iprd,
				"or ","or ","ro ","ro ","r  ","ra ",
			//	ipsh, ipwr, ipxr, ipxw, iret, ishl,
				"r  ","ar ","ri ","ir ","f  ","ro ",
			//	ishr, isub, itst, ixor, ixrd, ixwr,
				"ro ","ro ","ro ","ro ","rx ","xo ",
			//	inop
				"   "
				};
			
			auto const ins = static_cast<t_Instruction_Id::Type>(tokens[tid].val);
			const char* pattern = pattern_table[tokens[tid].val];
			address++;

			int const first_line = tokens[tid].line_num;

			std::string const pattern_err = [&](){
				std::string ret = "";
				
				for(int i = 0; i < 3; i++) switch(pattern[i])
				{
				case ' ': return ret;
				case 'r': ret += "REG ";                     continue;
				case 'x': ret += "EXT ";                     continue;
				case 'i': ret += "IMM/UDF/UVR/CEX ";         continue;
				case 'v': ret += "IMM/UDF/UVR/ULB/EXP ";     continue;
				case 'o': ret += "REG/IMM/UDF/UVR/ULB/EXP "; continue;
				case 'a': ret += "REG/IMM/UDF/UVR/CEX ";     continue;
				case 'c': ret += "CCC ";                     continue;
				case 'f': ret += "FLG ";                     continue;
				default:  ret += "invalid pattern supplied ";continue;
				}

				return ret;
			}();

			output.emplace_back(tokens[tid]);
			tid++;
			for(int i = 0; i < 3; i++) 
			{
				if(' ' == pattern[i])
					break;
				
				if(tid >= size
				|| tokens[tid].line_num != first_line)
				{
					if('f' == pattern[i] && tid < size)
					{
						output.emplace_back(t_Token::fla, 0, tokens[tid].file_name, first_line);
						break;
					}
					Log::error(
					    "incomplete instruction or instruction split over multiple lines\n       "
						"expected INS "s + pattern_err,
						file_name, cur_line_num);

					ADVANCE_TO_NEXT_LINE
					goto next_line;
				}

				switch(pattern[i])
				{
				case 'r':
					if(t_Token::reg == tokens[tid].type) {output.emplace_back(tokens[tid]); tid++; continue; }
					Log::error("unexpected token, expected REG but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 'x':
					if(t_Token::ext == tokens[tid].type) 
					{
						if(t_Instruction_Id::ixwr == ins
						&& tokens[tid].val == 1) //CF
						{
							Log::error("CF cannot be written to", tokens[tid].file_name, tokens[tid].line_num);
							tid++;
							continue;
						}
						if(t_Instruction_Id::ixrd == ins
						&& tokens[tid].val == 9) //UI
						{
							Log::error("UI cannot be read from", tokens[tid].file_name, tokens[tid].line_num);
							tid++;
							continue;
						}
						output.emplace_back(tokens[tid].type, tokens[tid].val & 0b111, tokens[tid].file_name, tokens[tid].line_num); 
						tid++; 
						continue; 
					}
					Log::error("unexpected token, expected EXT but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 'c':
				{
					//try to convert
					if(t_Token::ulb == tokens[tid].type) 
					{
						int const ccc = ccc_to_int(strings_get(tokens[tid].val));
						if(-1 != ccc) {output.emplace_back(t_Token::ccc, ccc, tokens[tid].file_name, tokens[tid].line_num); tid++; continue; }
					}
					Log::error("unexpected token, expected CCC but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				}
				case 'f':
				{
					//try to convert
					if(t_Token::ulb == tokens[tid].type) 
					{
						int const socz = socz_to_int(strings_get(tokens[tid].val));
						if(-1 != socz) {output.emplace_back(t_Token::fla, socz, tokens[tid].file_name, tokens[tid].line_num); tid++; continue; }
					}
					Log::error("unexpected token, expected FLA but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				}

				case 'i':
					if(t_Token::num == tokens[tid].type
					|| t_Token::udf == tokens[tid].type
					|| t_Token::uvr == tokens[tid].type)
					{ output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::exs == tokens[tid].type)
					{
						if(verify_expression(output, tokens, tid, true))
							continue;

						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}

					Log::error(
					    "unexpected token, expected IMM/UDF/UVR/CEX but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 'v':
					if(t_Token::num == tokens[tid].type
					|| t_Token::udf == tokens[tid].type
					|| t_Token::uvr == tokens[tid].type
					|| t_Token::ulb == tokens[tid].type)
					{ output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::slb == tokens[tid].type)
					{
						if('>' == tokens[tid].val || '<' == tokens[tid].val)
							{output.emplace_back(tokens[tid]); tid++; continue; }

						Log::error("., .. can only be used as label definitions", tokens[tid].file_name, tokens[tid].line_num);	
						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}
					if(t_Token::exs == tokens[tid].type)
					{
						if(verify_expression(output, tokens, tid, false))
							continue;

						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}

					Log::error(
					    "unexpected token, expected IMM/UDF/UVR/ULB/EXP but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;

				case 'a':
					if(t_Token::num == tokens[tid].type
					|| t_Token::udf == tokens[tid].type
					|| t_Token::uvr == tokens[tid].type
					|| t_Token::reg == tokens[tid].type)
					{ output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::exs == tokens[tid].type)
					{
						if(verify_expression(output, tokens, tid, true))
							continue;

						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}

					Log::error(
					    "unexpected token, expected REG/IMM/UDF/UVR/ULB/EXP but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				case 'o':
					if(t_Token::num == tokens[tid].type
					|| t_Token::udf == tokens[tid].type
					|| t_Token::uvr == tokens[tid].type
					|| t_Token::ulb == tokens[tid].type
					|| t_Token::reg == tokens[tid].type)
					{ output.emplace_back(tokens[tid]); tid++; continue; }
					if(t_Token::slb == tokens[tid].type)
					{
						if('>' == tokens[tid].val || '<' == tokens[tid].val)
							{output.emplace_back(tokens[tid]); tid++; continue; }

						Log::error("., .. can only be used as label definitions", tokens[tid].file_name, tokens[tid].line_num);	
						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}
					if(t_Token::exs == tokens[tid].type)
					{
						if(verify_expression(output, tokens, tid, false))
							continue;

						ADVANCE_TO_NEXT_LINE
						goto next_line;
					}

					Log::error(
					    "unexpected token, expected REG/IMM/UDF/UVR/ULB/EXP but got "s + to_string(tokens[tid].type) + "\n       "
						"expected INS "s + pattern_err,
						file_name, tokens[tid].line_num);
					ADVANCE_TO_NEXT_LINE
					goto next_line;
				}
			}
			if(tid != size
			&& tokens[tid].line_num == first_line)
			{
				Utils::warning("[extradata] extra data after instruction",
							   file_name, cur_line_num,
							   Warning::extradata);
				ADVANCE_TO_NEXT_LINE
			}
			break;
		}
		case t_Token::slb:
			if(',' == tokens[tid].val || '.' == tokens[tid].val)
			{
				output.emplace_back(tokens[tid]);
				tid++;
				continue;
			}
			Log::error(".> .< can only be used as ulb", tokens[tid].file_name, tokens[tid].line_num);	
			tid++;
			break;
		default:
			Log::error(
				"unexpected token at the start of a line: "s + to_string(tokens[tid].type) + "\n       "
				"expected instruction, label definition, directive, macro or comment",
				file_name, 
				cur_line_num);

			ADVANCE_TO_NEXT_LINE;
			break;
		}
	next_line:
		continue;
	}

	return;
}
