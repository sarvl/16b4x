#include "./process.h"

#include <iostream>

#include "top/gen_utils/log.h"
#include "top/types/types.h"
#include "top/utils.h"

using namespace std::literals;

void         strings_add(std::string const& str);
std::string& strings_get(int const ind);
std::string& strings_back();
int          strings_size();

static int get_const_val(
	std::vector<t_Token>    const& tokens,
	std::vector<t_Variable> const& variables,
	std::vector<t_Define>   const& defines,
	int                          & tid
	)
{
	using enum t_Token::Type;
	switch(tokens[tid].type)
	{
	case num:
		tid++;	
		return tokens[tid - 1].val;
	case uvr:
	{
		std::string const& to_cmp = strings_get(tokens[tid].val);
		for(auto const& var : variables)
		{
			if(var.name == to_cmp)
			{
				tid++;
				return var.value;
			}
		}
		
		Log::error("variable "s + to_cmp + " does not exist", tokens[tid].file_name, tokens[tid].line_num);
		tid++;
		return 0;
	}
	case udf:
	{
		std::string const& to_cmp = strings_get(tokens[tid].val);
		for(auto const& def : defines)
		{
			if(def.name == to_cmp)
			{
				tid++;
				return def.value;
			}
		}
		
		Log::error("define "s + to_cmp + " does not exist", tokens[tid].file_name, tokens[tid].line_num);
		tid++;
		return 0;
	}
	case exs:
		return evaluate_const_expr(tokens, variables, defines, tid);
	default:
		Log::internal_error("unhandled token in get_const_val");
		return 0;
	}
}


void preprocess(
	std::vector<t_Token>       & output,
	std::vector<t_Label>       & labels,
	std::vector<t_Token>  const& tokens,
	//copy because no longer needed by caller
	std::vector<t_Define>        defines
	)
{
	std::vector<t_Variable> variables;

	int const size = tokens.size();

	int address = 0;
	//so there is no garbage
	std::string current_top_level = "^-1";
	int current_label = -1;

	int tid = 0;
	std::vector<int> cif_depth;
	std::vector<int> rps_depth;
	std::vector<int> rps_to_repeat;
	while(static_cast<unsigned>(tid) < tokens.size()) 
	{
		int              const cur_line = tokens[tid].line_num;
		std::string_view const cur_file = tokens[tid].file_name;

		using enum t_Token::Type;
		using enum t_Directive::Type;
		switch(tokens[tid].type)
		{
		case ins:
			address++;
			output.emplace_back(tokens[tid]);
			tid++;
			break;
		case dlb:
		{
			current_label++;
			if('.' == strings_get(tokens[tid].val)[0])
			{
				strings_add(current_top_level + strings_get(tokens[tid].val));
				labels.emplace_back(strings_back(), address);
			}
			else
			{
				labels.emplace_back(strings_get(tokens[tid].val), address);
				current_top_level = strings_get(tokens[tid].val);
			}
			tid++;
			break;
		}
		case ulb:
		{
			if('.' == strings_get(tokens[tid].val)[0])
			{
				output.emplace_back(ulb, strings_size(), tokens[tid].file_name, tokens[tid].line_num);
				strings_add(current_top_level + strings_get(tokens[tid].val));
			}
			else
			{
				//its already ok
				output.emplace_back(tokens[tid]);
			}

			tid++;
			break;
		}
		case slb:
			if(',' == tokens[tid].val)
			{
				current_label++;
				//uses special character that cannot be normally referenced and some text that is definitely unique
				strings_add("^" + std::to_string(labels.size()));	
				labels.emplace_back(strings_back(), address);
				current_top_level = strings_back();
				tid++;
				continue;
			}
			else if('.' == tokens[tid].val)
			{
				current_label++;
				//uses special character that cannot be normally referenced and some text that is definitely unique
				strings_add(current_top_level + ".^"s + std::to_string(labels.size()));	
				labels.emplace_back(strings_back(), address);
				tid++;
				continue;
			}
			
			//change meaning of slb, from now on it POINTS to label 
			//much easier to check later
			if('<' == tokens[tid].val)
				output.emplace_back(t_Token::slb, current_label    , tokens[tid].file_name, tokens[tid].line_num);
			else
				output.emplace_back(t_Token::slb, current_label + 1, tokens[tid].file_name, tokens[tid].line_num);

			tid++;
			break;

		//handle them here so there is no need to pass defines and variables later
		case uvr: case udf:
		{
			int const v = get_const_val(tokens, variables, defines, tid);
			output.emplace_back(t_Token::num, v, tokens[tid - 1].file_name, tokens[tid - 1].line_num);
			break;
		}
		case dir:
		{
			char severity = ' ';
			switch(static_cast<t_Directive::Type>(tokens[tid].val))
			{
			case ddw:
				address++;
				output.emplace_back(tokens[tid]);
				tid++;
				break;
			case cif:
				cif_depth.emplace_back(cur_line);
				tid++;

				//handled at %cei %cel %cen
				if(get_const_val(tokens, variables, defines, tid))
					continue;

				while(tid < size)
				{
					if(dir == tokens[tid].type)
					{
						if(cif == tokens[tid].val)
							cif_depth.emplace_back(-1 * tokens[tid].line_num);
						if(cei == tokens[tid].val)
						{
							if(0 > cif_depth.back())
							{
								tid++;
								continue;
							}

							tid++;
							if(get_const_val(tokens, variables, defines, tid))
								goto next_token;
							continue;
						}
						if(cel == tokens[tid].val)
						{
							if(0 > cif_depth.back())
							{
								tid++;
								continue;
							}

							tid++;
							goto next_token;
						}
						if(cen == tokens[tid].val)
						{
							if(-1 == cif_depth.back())
							{
								cif_depth.pop_back();
								tid++;
								continue;
							}

							cif_depth.pop_back();
							tid++;
							goto next_token;
						}
					}
					tid++;
				}
				break;
			case cei: case cel:
				if(0 == cif_depth.size())
				{
					Log::error("%elsif or %else without matching %if", tokens[tid].file_name, tokens[tid].line_num);
					//no point in trying to find more errors
					return;
				}
				while(tid < size)
				{
					if(dir == tokens[tid].type)
					{
						if(cif == tokens[tid].val)
							cif_depth.emplace_back(-1); //whatever
						if(cen == tokens[tid].val)
						{
							if(-1 != cif_depth.back())
							{
								tid++;
								cif_depth.pop_back();
								break;
							}

							cif_depth.pop_back();
						}
					}

					tid++;
				}
				break;
			case adr:
			{
				std::string_view const file_name = tokens[tid].file_name;
				int              const line_num  = tokens[tid].line_num;
				output.emplace_back(tokens[tid]);
				tid++;
				int const to_get_to = get_const_val(tokens, variables, defines, tid);
				output.emplace_back(t_Token::num, to_get_to, tokens[tid - 1].file_name, tokens[tid - 1].line_num);
				
				if(to_get_to < address)
					Log::error("desired address ("s + std::to_string(to_get_to) + ") is smaller than current address ("s + std::to_string(address) + ")", 
					file_name, line_num);
				address = to_get_to;
				break;
			}
			case alg:
			{
				std::string_view const file_name = tokens[tid].file_name;
				int              const line_num  = tokens[tid].line_num;
				output.emplace_back(tokens[tid]);
				tid++;
				int const alignment = get_const_val(tokens, variables, defines, tid);
				output.emplace_back(t_Token::num, alignment, tokens[tid - 1].file_name, tokens[tid - 1].line_num);
				
				if(alignment <= 0)
				{
					Log::error("alignment must be positive", file_name, line_num);
					break;
				}

				//probably can be done as single computation
				while(address % alignment != 0)
					address++;

				break;
			}
			case cen:
				if(0 == cif_depth.size())
				{
					Log::error("%end without matching %if", tokens[tid].file_name, tokens[tid].line_num);
					//no point in trying to find more errors
					return;
				}
				cif_depth.pop_back();
				tid++;
				break;
			case rps:
			{
				tid++;
				int const rep_count = get_const_val(tokens, variables, defines, tid);
				int const rep_start = tid;

				//skip to next rpe
				if(0 >= rep_count)
				{
					while(tid < size)
					{
						if(dir == tokens[tid].type)
						{
							if(rps == tokens[tid].val)
								rps_depth.emplace_back(tid + 1);
							if(rpe == tokens[tid].val)
							{
								if(0 == rps_depth.size())
								{
									tid++;
									goto next_token;
								}
								rps_depth.pop_back();
							}
						}
						tid++;
					}
				}

				rps_depth.emplace_back(rep_start);
				rps_to_repeat.emplace_back(rep_count - 1);
				break;
			}
			case rpe:
				if(0 == rps_depth.size())
				{
					Log::error("%rpe with no matching %rps", cur_file, cur_line);
					tid++;
					break;
				}
				if(0 >= rps_to_repeat.back())
				{
					rps_to_repeat.pop_back();
					rps_depth.pop_back();
					tid++;
					break;
				}
				rps_to_repeat.back()--;
				tid = rps_depth.back();
				break;
			case ass:
			{
				std::string_view const name = strings_get(tokens[tid + 1].val);
				tid += 2;
				int              const val  = get_const_val(tokens, variables, defines, tid);

				//find whether assigned
				for(auto& var : variables)
				{
					if(var.name == name)
					{
						var.value = val;
						goto next_token;
					}
				}
				
				variables.emplace_back(name, val);
				break;
			}
			case def:
			{
				std::string_view const name = strings_get(tokens[tid + 1].val);
				tid += 2;
				int              const val  = get_const_val(tokens, variables, defines, tid);

				//find whether define
				for(auto& def : defines)
				{
					if(def.name == name)
					{
						Log::error("reused define "s + std::string(name), 
						           "use %assign if redefinition is desider behavior",
						           cur_file, cur_line);

						goto next_token;
					}
				}
				
				defines.emplace_back(name, val);
				break;
			}
			case inf:
				severity = 'i';
				goto dir_print;
			case war:
				severity = 'w';
				goto dir_print;
			case err:
				severity = 'e';
			dir_print:
			{
				std::string to_print;
				tid++;
				//either str or %tostring 
				if(str == tokens[tid].type)
				{
					to_print += strings_get(tokens[tid].val);
					tid++;
					goto dir_print_real;
				}

				if(dir == tokens[tid].type)
				{
					tid++;
					switch(tokens[tid].type)
					{
					case str:
						to_print += strings_get(tokens[tid].val);
						tid++;
						break;
					case num: case uvr: case udf: case exs:
						to_print += std::to_string(get_const_val(tokens, variables, defines, tid));
						break;
					case dir:
						to_print += to_string(static_cast<t_Directive::Type>(tokens[tid].val));
						tid++;
						break;
					default:
						to_print += "tostring does not support this argument yet";
						while(cur_line == tokens[tid].line_num)
							tid++;

						break;
					}
			dir_print_real:
				if('e' == severity)
					Log::error(to_print, cur_file, cur_line);
				if('w' == severity)
					Utils::warning(to_print, cur_file, cur_line, Warning::custom);
				if('i' == severity)
					Log::info(to_print, cur_file, cur_line);
				}
				break;
			}
			case isd: case isa: case tof: case sam:
				Log::error("directive can only be used in expression", cur_file, cur_line);
				while(cur_line == tokens[tid].line_num)
					tid++;
				break;
			default:
				//cant be handled yet
				output.emplace_back(tokens[tid]);
				tid++;
				break;
				
			}
			break;
		}
		case exo:
			if('$' == tokens[tid].val)
			{
				//adjust since comes after address incrementing ins/dir
				output.emplace_back(t_Token::num, address - 1, tokens[tid].file_name, tokens[tid].line_num);
				tid++;
				break;
			}

		[[fallthrough]];
		default:
			output.emplace_back(tokens[tid]);
			tid++;
			break;

		}
	next_token:
		continue;

	}

	while(cif_depth.size() > 0)
	{
		if(cif_depth.back() < 0)
			Log::error("unterminated conditional started at ", tokens[0].file_name, -1 * cif_depth.back());
		else
			Log::error("unterminated conditional started at ", tokens[0].file_name,      cif_depth.back());
		cif_depth.pop_back();
	}
	while(rps_depth.size() > 0)
	{
		Log::error("unterminated repetition block started at ", tokens[0].file_name, tokens[rps_depth.back() - 1].line_num);
		rps_depth.pop_back();
	}

	return;
}
