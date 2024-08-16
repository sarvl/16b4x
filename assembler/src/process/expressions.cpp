#include "./process.h"

#include <iostream>

#include "top/gen_utils/log.h"
#include "top/utils.h"

using namespace std::literals;

void         strings_add(std::string const& str);
std::string& strings_get(int const ind);
std::string& strings_back();
int          strings_size();

int evaluate_const_expr(
	std::vector<t_Token>    const& tokens,
	std::vector<t_Variable> const& variables,
	std::vector<t_Define>   const& defines,
	int                          & tid
	)
{
	std::vector<int> stack;
	int const start = tid;
	tid++;
	while(static_cast<unsigned>(tid) < tokens.size()
	   && t_Token::exe != tokens[tid].type) 
	{
		switch(tokens[tid].type)
		{
		case t_Token::num:
			stack.emplace_back(tokens[tid].val);
			tid++;
			continue;

		case t_Token::udf:
		{
			std::string const& str = strings_get(tokens[tid].val);
			for(auto const& define : defines) if(define.name == str)
			{
				stack.emplace_back(define.value);
				tid++;
				goto next_token;
			}

			Log::error(str + " not defined", tokens[tid].file_name, tokens[tid].line_num);
			//push 0 and continue
			stack.emplace_back(0);
			tid++;
			break;
		}
		case t_Token::uvr:
		{
			std::string const& str = strings_get(tokens[tid].val);
			for(auto const& variable : variables) if(variable.name == str)
			{
				stack.emplace_back(variable.value);
				tid++;
				goto next_token;
			}

			Log::error(str + " does not exist", tokens[tid].file_name, tokens[tid].line_num);
			//push 0 and continue
			stack.emplace_back(0);
			tid++;
			break;
		}
		case t_Token::dir:
			switch(static_cast<t_Directive::Type>(tokens[tid].val))
			{
			case t_Directive::tof:
				stack.emplace_back(static_cast<int>(tokens[tid + 1].type));
				tid += 2;
				break;
			case t_Directive::sam:
				stack.emplace_back(
					(tokens[tid + 1].type == tokens[tid + 2].type) && (tokens[tid + 1].val == tokens[tid + 2].val)
					);
				tid += 3;
				continue;
			case t_Directive::isd:
			{
				std::string_view const str = strings_get(tokens[tid + 1].val);

				int v = 0;
				for(auto const& def : defines) if(def.name == str)
				{
					v = 1;
					break;
				}
				stack.emplace_back(v);
				tid += 2;
				break;
			}
			case t_Directive::isa:
			{
				std::string_view const str = strings_get(tokens[tid + 1].val);

				int v = 0;
				for(auto const& var : variables) if(var.name == str)
				{
					v = 1;
					break;
				}
				stack.emplace_back(v);
				tid += 2;
				break;
			}
			default:
				std::cout << tokens[tid].line_num << '\n';
				Log::internal_error("unhandled directive in constant expression evaluation");
				tid++;
				break;
			}
			break;
		case t_Token::exo:
		{
			#define ARGS1\
			if(stack.size() < 1) Log::error("not enough arguments on stack", tokens[tid].file_name, tokens[tid].line_num);\
			else {a = stack.back(); stack.pop_back();}

			#define ARGS2\
			if(stack.size() < 2) Log::error("not enough arguments on stack", tokens[tid].file_name, tokens[tid].line_num);\
			else { b = stack.back(); stack.pop_back();\
			       a = stack.back(); stack.pop_back();}

			#define OP1(op) \
			 ARGS1\
			 stack.emplace_back(op a);
			#define OP2(op) \
			 ARGS2\
			 stack.emplace_back(a op b);
			
			int a = 0, b = 0;
			switch(tokens[tid].val)
			{
			case '+': OP2(+) ; break;
			case '-': OP2(-) ; break;
			case '*': OP2(*) ; break;
			case '/': OP2(/) ; break;
			case '%': OP2(%) ; break;
			case '&': OP2(&) ; break;
			case '|': OP2(|) ; break;
			case '^': OP2(^) ; break;
			case '=': OP2(==); break;

			case ',': 
				ARGS2
				stack.emplace_back(b);
				stack.emplace_back(a);
				break;

			case '~': OP1(~) ; break;
			case '?': OP1(!!); break;
			case '!': OP1(!) ; break;

			case '.':
				if(stack.size() < 1) Log::error("not enough arguments on stack", tokens[tid].file_name, tokens[tid].line_num);\
				else {stack.emplace_back(stack.back());}
				break;
			case '#':
				if(stack.size() < 1) Log::error("not enough arguments on stack", tokens[tid].file_name, tokens[tid].line_num);\
				else {stack.pop_back();}
				break;

			case '<': 
				ARGS2
				if(b >= 16) a = 0;
				else        a <<= b;
				stack.emplace_back(a);
				break;
			case '>':
				ARGS2
				if(b >= 16) a = 0;
				else        a >>= b;
				stack.emplace_back(a);
				break;
			case '$': 
				Log::internal_error("unhandled token in constant expression evaluation");
				break;

			}
			
			tid++;
			continue;
		}
		default:
				std::cout << tokens[tid].line_num << '\n';
			Log::internal_error("unhandled token in constant expression evaluation");
			continue;
		}

	next_token:
		continue;
	}
	tid++;
	if(0 == stack.size())
	{
		Log::error("expression contains no result", tokens[start].file_name, tokens[start].line_num);
		return -1;
	}

	if(1 != stack.size())
	{
		Utils::warning("[extravals] expression has more than 1 result, returning top of the stack", tokens[start].file_name, tokens[start].line_num, 
			Warning::extravals, "^ if this is not an error, remove extra arguments with '#' or disable this warning");
	}

	return stack.back();
}


int evaluate_expr(
	std::vector<t_Token> const& tokens,
	std::vector<t_Label> const& labels,
	int                       & tid,
	int                  const  addr
	)
{
	std::vector<int> stack;
	int const start = tid;
	tid++;
	while(static_cast<unsigned>(tid) < tokens.size()
	   && t_Token::exe != tokens[tid].type) 
	{
		switch(tokens[tid].type)
		{
		case t_Token::num:
			stack.emplace_back(tokens[tid].val);
			tid++;
			continue;

		case t_Token::ulb:
		{
			std::string_view const cur_label = strings_get(tokens[tid].val);
			//find the string mapping 
			for(auto const& label : labels)
			{
				if(cur_label == label.name)
				{
					tid++;
					if(addr >= 0)
						stack.emplace_back(label.value - addr - 1);
					else
						stack.emplace_back(label.value);

					goto next_token;
				}
			}
			
			Log::error(
				"label \""s + std::string(cur_label) + "\" could not be found", 
				tokens[tid].file_name, tokens[tid].line_num);
			stack.emplace_back(0);

			tid++;
			continue;
		}
		case t_Token::exo:
		{	
			int a = 0, b = 0;
			switch(tokens[tid].val)
			{
			case '+': OP2(+) ; break;
			case '-': OP2(-) ; break;
			case '*': OP2(*) ; break;
			case '/': OP2(/) ; break;
			case '%': OP2(%) ; break;
			case '&': OP2(&) ; break;
			case '|': OP2(|) ; break;
			case '^': OP2(^) ; break;
			case '=': OP2(==); break;

			case ',': 
				ARGS2
				stack.emplace_back(b);
				stack.emplace_back(a);
				break;

			case '~': OP1(~) ; break;
			case '?': OP1(!!); break;
			case '!': OP1(!) ; break;

			case '.':
				if(stack.size() < 1) Log::error("not enough arguments on stack", tokens[tid].file_name, tokens[tid].line_num);\
				else {stack.emplace_back(stack.back());}
				break;
			case '#':
				if(stack.size() < 1) Log::error("not enough arguments on stack", tokens[tid].file_name, tokens[tid].line_num);\
				else {stack.pop_back();}
				break;

			case '<': 
				ARGS2
				if(b >= 16) a = 0;
				else        a <<= b;
				stack.emplace_back(a);
				break;
			case '>':
				ARGS2
				if(b >= 16) a = 0;
				else        a >>= b;
				stack.emplace_back(a);
				break;
			case '$': 
				//unreachable, replaced before
				Log::internal_error("unhandled operator in expression evaluation");
				break;

			}
			
			tid++;
			continue;
		}
		default:
			Log::internal_error("unhandled token in expression evaluation");
			continue;
		}

	next_token:
		continue;
	}
	tid++;
	if(0 == stack.size())
	{
		Log::error("expression contains no result", tokens[start].file_name, tokens[start].line_num);
		return -1;
	}

	if(1 != stack.size())
	{
		Utils::warning("[extravals] expression has more than 1 result, returning top of the stack", tokens[start].file_name, tokens[start].line_num, 
			Warning::extravals, "^ if this is not an error, remove extra arguments with '#' or disable this warning");
	}

	return stack.back();
}
