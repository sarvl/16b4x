#include "./pass.h"

#include <iostream>

#include "./../../utility/src/log.h"

extern std::vector<std::string> strings;

t_Label::t_Label(){}
t_Label::t_Label(std::string_view const n_name, int const n_ins)
	: name(n_name), instruction(n_ins) {}


int ccc_to_int(std::string_view const cc)
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

int socz_to_int(std::string_view const socz)
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


//DOES NOT WORK FOR JMP REG AND CAL REG 
//MODIFY ACCORDINGLY
int instruction_to_opcode(t_Instruction_Id const iid)
{
	using enum t_Instruction_Id;
	switch(iid)
	{
	case iadd: return 0b00011000;
	case iand: return 0b00011011;
	case ical: return 0b00000010;
	case icmp: return 0b00010001;
	case idiv: return 0b00010010;
	case ihcf: return 0b00000001011;
	case ihlt: return 0b00000001010;
	case iint: return 0b00000001000;
	case iirt: return 0b00000001001;
	case ijcs: return 0b00000101;
	case ijcu: return 0b00000100;
	case ijmp: return 0b00000011;
	case imcs: return 0b00010111;
	case imcu: return 0b00010110;
	case imov: return 0b00010100;
	case imrd: return 0b00001100;
	case imro: return 0b00001110;
	case imul: return 0b00010000;
	case imwo: return 0b00001111;
	case imwr: return 0b00001101;
	case inot: return 0b00011010;
	case iorr: return 0b00011100;
	case ipop: return 0b00010101000;
	case iprd: return 0b00001000;
	case ipsh: return 0b00010101001;
	case ipwr: return 0b00001001;
	case ipxr: return 0b00000001110;
	case ipxw: return 0b00000001111;
	case iret: return 0b00010101100;
	case inop: return 0b00010101111;
	case ishl: return 0b00011110;
	case ishr: return 0b00011111;
	case isub: return 0b00011001;
	case itst: return 0b00010011;
	case ixor: return 0b00011101;
	case ixrd: return 0b00001010;
	case ixwr: return 0b00001011;
	//should not be reachable
	default:   return -1;
	}
}

void pass_first(
	std::vector<t_Token>      & output,
	std::vector<t_Token> const& tokens, 
	std::vector<t_Label>      & labels
	)
{
	Log::is_error = false;
	Log::is_warning = false;

	#define ADVANCE_TO_NEXT_LINE\
		while(tid < size\
		   && tokens[tid].line_num == cur_line_num)\
		  	tid++;

	int instruction_count = 0;
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
		//directive, for now, skip completely
		case t_Token::dir:
			Log::warning("directives have not been implemented yet", file_name, cur_line_num);
			ADVANCE_TO_NEXT_LINE;
			break;
		case t_Token::str:
			if(tid == size - 1
			|| tokens[tid + 1].type != t_Token::col)
			{
				Log::error("labels require colon at the end", file_name, cur_line_num);
				ADVANCE_TO_NEXT_LINE;
				break;
			}
			labels.emplace_back(strings[tokens[tid].val], instruction_count);

			tid += 2;
			break;


		case t_Token::ins:
		{
			instruction_count++;
			using enum t_Instruction_Id;
			//within instructions, stuff may get complicted 
			switch(static_cast<t_Instruction_Id>(tokens[tid].val))
			{
			//ins reg imm
			//ins reg reg 
			case iadd: case iand: case icmp: case idiv:
			case imov: case imrd: case imro: case imul:
			case inot: case iorr: case iprd: case ishl: 
			case ishr: case isub: case itst: case ixor: 
				//ensure enough tokens
				if(tid + 2 >= size
				//ensure all on the same line
				|| tokens[tid + 2].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::reg
				|| (  tokens[tid + 2].type != t_Token::reg
				   && tokens[tid + 2].type != t_Token::num
				   && tokens[tid + 2].type != t_Token::str) //labels
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS REG REG or INS REG IMM\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				output.emplace_back(tokens[tid + 2]);

				ADVANCE_TO_NEXT_LINE;

				break;
			//ins imm reg 
			//ins reg reg 
			case imwo: case imwr: case ipwr:
				//ensure enough tokens
				if(tid + 2 >= size
				//ensure all on the same line
				|| tokens[tid + 2].line_num != cur_line_num
				//ensure proper format
				|| (  tokens[tid + 1].type != t_Token::reg
				   && tokens[tid + 1].type != t_Token::num
				   && tokens[tid + 1].type != t_Token::str) //labels
				|| tokens[tid + 2].type != t_Token::reg
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS REG REG or INS IMM REG\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				output.emplace_back(tokens[tid + 2]);

				ADVANCE_TO_NEXT_LINE;

				break;
			//ins ext imm
			//ins ext reg
			case ixwr:
				//ensure enough tokens
				if(tid + 2 >= size
				//ensure all on the same line
				|| tokens[tid + 2].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::ext
				|| (  tokens[tid + 2].type != t_Token::reg
				   && tokens[tid + 2].type != t_Token::num
				   && tokens[tid + 2].type != t_Token::str) //labels
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS EXT REG or INS EXT IMM\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				output.emplace_back(tokens[tid + 2]);

				ADVANCE_TO_NEXT_LINE;

				break;
			//ins reg ext
			case ixrd:
				//ensure enough tokens
				if(tid + 2 >= size
				//ensure all on the same line
				|| tokens[tid + 2].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::reg
				|| tokens[tid + 2].type != t_Token::ext
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS REG EXT\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				output.emplace_back(tokens[tid + 2]);
				ADVANCE_TO_NEXT_LINE;
				break;
			//ins imm
			case iint:
				//ensure enough tokens
				if(tid + 1 >= size
				//ensure all on the same line
				|| tokens[tid + 1].line_num != cur_line_num
				//ensure proper format
				|| (  tokens[tid + 1].type != t_Token::num
				   && tokens[tid + 1].type != t_Token::str) //labels
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS IMM\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 2 < size
				&& tokens[tid + 2].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				ADVANCE_TO_NEXT_LINE;
				break;
			//ins imm
			//ins reg
			case ical: case ijmp: 
				//ensure enough tokens
				if(tid + 1 >= size
				//ensure all on the same line
				|| tokens[tid + 1].line_num != cur_line_num
				//ensure proper format
				|| (  tokens[tid + 1].type != t_Token::num
				   && tokens[tid + 1].type != t_Token::str //labels
				   && tokens[tid + 1].type != t_Token::reg)
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS IMM or INS REG\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 2 < size
				&& tokens[tid + 2].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				ADVANCE_TO_NEXT_LINE;
				break;
			//ins ccc imm
			case ijcu: case ijcs:
			{
				//ensure enough tokens
				if(tid + 1 >= size
				//ensure all on the same line
				|| tokens[tid + 1].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::str
				|| (  tokens[tid + 2].type != t_Token::num
				   && tokens[tid + 2].type != t_Token::str) //labels
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS CCC IMM\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}

				//check whether string contains only valid condition codes
				int const ccc = ccc_to_int(strings[tokens[tid + 1].val]);
				if(-1 == ccc)
				{
					Log::error(
						"invalid conditions specified\n       "
						"valid conditions are: LEGZlegz",
						file_name,
						cur_line_num);
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid]);
				output.emplace_back(t_Token::ccc, ccc, tokens[tid + 1].file_name, tokens[tid + 1].line_num);
				output.emplace_back(tokens[tid + 2]);

				ADVANCE_TO_NEXT_LINE;
				break;
			}
			//ins
			case ihcf: case ihlt: case iirt:
			case inop:
				if(tid + 1 < size
				&& tokens[tid + 1].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				ADVANCE_TO_NEXT_LINE;
				break;

			//ins
			//ins socz
			case iret:
				//check whether next token on the same line
				if(tid + 1 < size
				&& tokens[tid + 1].line_num == cur_line_num)
				{
					//ins socz
					//ensure proper format
					if(tokens[tid + 1].type != t_Token::str)
					{
						Log::error(
							"incomplete instruction\n       "
							"expected INS SOCZ or INS\n       "
							"ENTIRE instruction must be on the same line", 
							file_name, 
							cur_line_num);

						ADVANCE_TO_NEXT_LINE;
						break;
					}

					//check whether string contains only valid flags
					int const socz = socz_to_int(strings[tokens[tid + 1].val]);
					if(-1 == socz)
					{
						Log::error(
							"invalid flags specified\n       "
							"valid flags are: SOCZEsocze",
							file_name,
							cur_line_num);
						break;
					}
					
					if(tid + 2 < size
					&& tokens[tid + 2].line_num == cur_line_num)
						Log::warning(
							"extra data after instruction, ignoring", 
							file_name, 
							cur_line_num);

					output.emplace_back(tokens[tid]);
					output.emplace_back(t_Token::fla, socz, tokens[tid + 1].file_name, tokens[tid + 1].line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				else
				{
					//ins
					output.emplace_back(tokens[tid + 0]);
					ADVANCE_TO_NEXT_LINE;
				}
				break;
			//ins ccc reg reg 
			case imcs: case imcu:
			{
				//ensure enough tokens
				if(tid + 3 >= size
				//ensure all on the same line
				|| tokens[tid + 3].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::str
				|| tokens[tid + 2].type != t_Token::reg
				|| tokens[tid + 3].type != t_Token::reg
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS CCC REG REG\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}

				//check whether string contains only valid condition codes
				int const ccc = ccc_to_int(strings[tokens[tid + 1].val]);
				if(-1 == ccc)
				{
					Log::error(
						"invalid ccc specified\n       "
						"valid ccc are: LEGZlegz",
						file_name,
						cur_line_num);
					break;
				}
				
				if(tid + 4 < size
				&& tokens[tid + 4].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(t_Token::ccc, ccc, tokens[tid + 1].file_name, tokens[tid + 1].line_num);
				output.emplace_back(tokens[tid + 2]);
				output.emplace_back(tokens[tid + 3]);
				ADVANCE_TO_NEXT_LINE;
				break;
			}
			//ins reg
			case ipop: case ipsh:
				//ensure enough tokens
				if(tid + 1 >= size
				//ensure all on the same line
				|| tokens[tid + 1].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::reg
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS REG\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 2 < size
				&& tokens[tid + 2].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				ADVANCE_TO_NEXT_LINE;
				break;
			//ins reg imm
			case ipxr: 
				//ensure enough tokens
				if(tid + 2 >= size
				//ensure all on the same line
				|| tokens[tid + 2].line_num != cur_line_num
				//ensure proper format
				|| tokens[tid + 1].type != t_Token::reg
				|| (  tokens[tid + 2].type != t_Token::num
				   && tokens[tid + 2].type != t_Token::str) //labels
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS REG IMM\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				output.emplace_back(tokens[tid + 2]);
				ADVANCE_TO_NEXT_LINE;
				break;
			//ins reg imm
			case ipxw:
				//ensure enough tokens
				if(tid + 2 >= size
				//ensure all on the same line
				|| tokens[tid + 2].line_num != cur_line_num
				//ensure proper format
				|| (  tokens[tid + 1].type != t_Token::num
				   && tokens[tid + 1].type != t_Token::str) //labels
				|| tokens[tid + 2].type != t_Token::reg
					)
				{
					Log::error(
						"incomplete instruction\n       "
						"expected INS IMM REG\n       "
						"ENTIRE instruction must be on the same line", 
						file_name, 
						cur_line_num);

					ADVANCE_TO_NEXT_LINE;
					break;
				}
				
				if(tid + 3 < size
				&& tokens[tid + 3].line_num == cur_line_num)
					Log::warning(
						"extra data after instruction, ignoring", 
						file_name, 
						cur_line_num);

				output.emplace_back(tokens[tid + 0]);
				output.emplace_back(tokens[tid + 1]);
				output.emplace_back(tokens[tid + 2]);
				ADVANCE_TO_NEXT_LINE;
				break;
			}
			break;
		}
		default:
			Log::error(
				"unexpected token\n       "
				"expected instruction, label, directive or comment",
				file_name, 
				cur_line_num);

			ADVANCE_TO_NEXT_LINE;
			break;
		}
	}

	return;
}


void pass_second(
	std::string               & result,
	std::vector<t_Token> const& tokens, 
	std::vector<t_Label> const& labels
	)
{
	int tid = 0;
	int instruction_count = -1; //so that it starts at 0
	int const size = static_cast<int>(tokens.size());
	while(true)
	{
		//skip until instruction
		while(tid < size
		   && tokens[tid].type != t_Token::ins)
			tid++;
		if(tid >= size)
			break;

		instruction_count++;
		unsigned int opcode = instruction_to_opcode(static_cast<t_Instruction_Id>(tokens[tid].val));
		unsigned int rf     = 0;
		unsigned int rs     = 0;
		unsigned int imm    = 0;
		unsigned int ccc    = 0;
		unsigned int socz   = 0;
		unsigned int res    = 0;

		using enum t_Instruction_Id;
		//within instructions, stuff may get complicted 
		switch(static_cast<t_Instruction_Id>(tokens[tid].val))
		{
		//ins reg imm
		//ins reg reg 
		case iadd: case iand: case icmp: case idiv:
		case imov: case imrd: case imro: case imul:
		case inot: case iorr: case iprd: case ishl: 
		case ishr: case isub: case itst: case ixor: 
		{
			rf = tokens[tid + 1].val;

			if(tokens[tid + 2].type == t_Token::reg)
			{
				rs = tokens[tid + 2].val;
				goto short_opcode_reg_reg;
			}
			if(tokens[tid + 2].type == t_Token::num)
			{
				imm = tokens[tid + 2].val;
				goto short_opcode_reg_imm;
			}
			std::string_view const cur_label = strings[tokens[tid + 2].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					imm = label.instruction;
					goto short_opcode_reg_imm;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		//ins imm reg 
		//ins reg reg 
		}
		case imwo: case imwr: case ipwr:
			rf = tokens[tid + 2].val;
			{

			if(tokens[tid + 1].type == t_Token::reg)
			{
				rs = tokens[tid + 1].val;
				goto short_opcode_reg_reg;
			}
			if(tokens[tid + 1].type == t_Token::num)
			{
				imm = tokens[tid + 1].val;
				goto short_opcode_reg_imm;
			}
			std::string_view const cur_label = strings[tokens[tid + 1].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					imm = label.instruction;
					goto short_opcode_reg_imm;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		//ins ext imm
		//ins ext reg
		}
		case ixwr:
		{
			rf = tokens[tid + 1].val;

			if(tokens[tid + 2].type == t_Token::reg)
			{
				rs = tokens[tid + 2].val;
				goto short_opcode_reg_reg;
			}
			if(tokens[tid + 2].type == t_Token::num)
			{
				imm = tokens[tid + 2].val;
				goto short_opcode_reg_imm;
			}
			std::string_view const cur_label = strings[tokens[tid + 2].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					imm = label.instruction;
					goto short_opcode_reg_imm;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		//ins reg ext
		}
		case ixrd:
		{
			rf = tokens[tid + 1].val;
			rs = tokens[tid + 2].val;
			goto short_opcode_reg_reg;
		//ins imm
		}
		case iint:
		{
			if(tokens[tid + 1].type == t_Token::num)
			{
				imm = tokens[tid + 1].val;
				goto long_opcode_immediate;
			}
			std::string_view const cur_label = strings[tokens[tid + 1].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					imm = label.instruction;
					goto long_opcode_immediate;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		}
		//ins imm
		//ins reg
		case ical: case ijmp: 
		{
			if(tokens[tid + 1].type == t_Token::reg)
			{
				if(ical == static_cast<t_Instruction_Id>(tokens[tid].val))
					opcode = 0b00010101010;
				else
					opcode = 0b00010101011;

				rf = tokens[tid + 1].val;
				goto long_opcode_reg_imm;
			}
			if(tokens[tid + 1].type == t_Token::num)
			{
				imm = tokens[tid + 1].val;
				goto short_opcode_long_immediate;
			}
			std::string_view const cur_label = strings[tokens[tid + 1].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					//-1 because ip points to NEXT instruction
					//so offset = 0 implies next instruction
					imm = label.instruction - instruction_count - 1;

					goto short_opcode_long_immediate;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		//ins ccc imm
		}
		case ijcu: case ijcs:
		{
			ccc = tokens[tid + 1].val;

			if(tokens[tid + 2].type == t_Token::num)
			{
				imm = tokens[tid + 2].val;
				goto short_opcode_jmp_ccc;
			}
			std::string_view const cur_label = strings[tokens[tid + 2].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					//-1 because ip points to NEXT instruction
					//so offset = 0 implies next instruction
					imm = label.instruction - instruction_count - 1;
					goto short_opcode_jmp_ccc;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		//ins 
		}
		case ihcf: case ihlt: case iirt: case inop:
		{
			goto long_opcode_immediate;
		//ins ccc reg reg
		}

		//ins
		//ins socz
		case iret:
			//check whether next token on the same line
			if(tid + 1 < size
			&& tokens[tid + 1].line_num == tokens[tid + 0].line_num)
				socz = tokens[tid + 1].val;
			else
				socz = 0;	
			
			goto long_opcode_ret_flags;
		case imcs: case imcu:
		{
			ccc = tokens[tid + 1].val;
			rf  = tokens[tid + 2].val;
			rs  = tokens[tid + 3].val;
			goto short_opcode_mov_ccc;
		//ins reg
		}
		case ipop: case ipsh:
		{
			rf = tokens[tid + 1].val;
			goto long_opcode_reg_imm;
		//ins reg imm
		}
		case ipxr:
		{
			rf = tokens[tid + 1].val;

			if(tokens[tid + 2].type == t_Token::num)
			{
				imm = tokens[tid + 2].val;
				goto long_opcode_reg_imm;
			}
			std::string_view const cur_label = strings[tokens[tid + 2].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					imm = label.instruction;
					goto long_opcode_reg_imm;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		//ins imm reg
		}
		case ipxw:
		{
			rf = tokens[tid + 2].val;

			if(tokens[tid + 1].type == t_Token::num)
			{
				imm = tokens[tid + 1].val;
				goto long_opcode_reg_imm;
			}
			std::string_view const cur_label = strings[tokens[tid + 1].val];
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					imm = label.instruction;
					goto long_opcode_reg_imm;
				}
			}
			
			std::string err = "label \"";
			err.append(cur_label);
			err.append("\" could not be found");
			Log::error(err, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			continue;
		}
		}

	#define WARN_IMM_TOO_BIG(BIT_COUNT)\
	{\
		if((static_cast<int32_t>(imm) >  (1 << (BIT_COUNT)) - 1)\
		|| (static_cast<int32_t>(imm) < -(1 << ((BIT_COUNT) - 1))))\
			Log::warning(\
				"immediate does not fit in BIT_COUNT bits\n"\
				"         consider prepending WRX UI or toggle switch so SASM can do it automatically",\
				tokens[tid].file_name, tokens[tid].line_num);\
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
		constexpr static char transl[] = "0123456789ABCDEF";
		
		char to_append[] = "0000\n";

		to_append[0] = transl[(res >> 12) & 0xF];
		to_append[1] = transl[(res >>  8) & 0xF];
		to_append[2] = transl[(res >>  4) & 0xF];
		to_append[3] = transl[(res >>  0) & 0xF];

		result.append(to_append, 5);

		tid++;	
		continue;
	}


	return;
}
