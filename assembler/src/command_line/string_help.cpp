extern char const* const string_help;
extern char const* const string_help_warn;
extern char const* const string_help_format;
extern char const* const string_help_directives;
extern char const* const string_help_expressions;
extern char const* const string_help_labels;
extern char const* const string_help_macros;
extern char const* const string_help_terminology;

char const* const string_help = R"(SASM PARAMETERS FILE...
Synthesizing Assembler

parameters
	-h	
		displays this help
	-hW -hH
		help regarding warnings
	-hf -fh
		help regarding output formats
	-hd
		help regarding directives
	-he
		help regarding expressions
	-hL
		help regarding expressions
	-hM 
		help regarding macros
	-hT
		help regarding terminology
	
	-lx 
		sets log level to x
		 e - errors 
		 w - errors, warnings
		 i - errors, warnings, info (default)
	
	-W 
		enable warning
	-WN
		disable warning
	-WE
		promote warning to error

	-D
		debug, undocumented and reserved

	-f
		sets output format
	-o filename
		sets output file to the filename
		defaults to "out.bin"

	-a 
		abort after first error
	-d 
		command line define in format -dNAME=VAL
		if no value is provided, assigns 1
		VAL must be a decimal number

`-o` option requires separation of filename from `-o`

all options should be specified as separate arguments
)";

char const* const string_help_warn = R"(WARNINGS
general:
	specify -W.*  to enable  particular warning
	specify -WN.* to disable particular warning
	specify -WE.* to promote warning to error

	options are evaluated from left to right and can override each other

	within option, warnings can be separated by comma

	WN removes warnings promoted to errors too but W does not add it back there

	by default all warnings are enabled

	warning name must follow W, WE or WN directly
	see examples

available warnings:
	all
		ALL warnings listed below
	immnofit
		immediate not fitting into available space
	extradata
		extra data after instruction
	extravals
		extra unused values in expressions
	custom
		custom warning used by %warn

examples:
	-WEall -WNimmnofit
		promote all warnings to errors 
		disable immediate warning, will not produce error

	-WEall -WNimmnofit -Wimmnofit
		promote all warnings to errors 
		disable immediate warning, will not produce error or warning
		enable immnofit warning, will not produce error but will produce warning

	-WNextravals,custom -WEimmnofit
		disables extravals and custom warnings
		immnofit will produce error

	-WNimmnofit -WEall
		disables immnofit warning
		all warnings except of immnofit will produce error
)";

char const* const string_help_format = R"(FORMAT
general:
	formats can be specified by their full name, short name or id
	rightmost format specified is used

	by default format is set to txt_1

	formats are listed as: id full_name short_name

available formats:

	text_formats:
		general_info:
			instructions are specified with hexadecimal digits
			unspecified addresses are filled with 0s, including padding
			all addresses from first to last are listed

		00 text_1_instruction txt_1
			1 instruction  per line
		01 text_2_instruction txt_2
			2 instructions per line
		02 text_4_instruction txt_4
			4 instructions per line
		09 text_continuous txt_c
			continuous list of instructions

	binary formats:
		general info:
			instructions are encoded directly with bits
			unspecified addresses are filled with 0s, including padding
			all addresses from first to last are listed

		10 binary_1_instruction bin_1
			1 instruction  per line
		11 binary_2_instruction bin_2
			2 instructions per line
		12 binary_4_instruction bin_4
			4 instructions per line
		19 binary_continuous bin_c
			continuous list of instruction
	
	vhdl formats:
		general info:
			instructions are specified in `addr => x"....",` format
			at the end, `OTHERS => x"0000"` is appended
			lines which would contain only x"0000" are not listed
			
		20 vhdl_1_instruction vhdl_1
			1 instruction  per line
		21 vhdl_2_instruction vhdl_2
			2 instructions per line
		22 vhdl_4_instruction vhdl_4
			4 instructions per line
		23 vhdl_5_instruction vhdl_5
			5 instructions per line

examples:
	-f00
		specify text_1_instruction format
	-f 23
		specify vhdl_5_instruction format
	-fvhdl_1
		specify vhdl_1_instruction format
	-fbinary_continuous
		specify binary_continuous format
)";

char const* const string_help_directives = R"(DIRECTIVES
general:
	all directives start with '%'

	directives are specified within assembly file
	they can be used to greatly speed up and simplify writing code
	as well as to make it more generic

	<> denote arguments to be supplied, without `<` or `>`

available directives:
	%inc <filename>
			
		act as if <filename> was pasted here 
		NOT SUPPORTED YET
	
	%align <count>
		insert nops until next instruction address mod <count> = 0

	%adr <address>
		insert 0x0000 until current address is incremented up to <address>
		MUST be greater than or equal to current address
	
	%dw <value>
		insert <value> into output file
	
	%rps <count>
		starts repeat section, must be terminated with %rpe
		repeats specified secion <count> times
	
	%rpe
		ends repeat section started by %rps
	
	%ws <name>
		enable warning <name> 
		only 1 warning at a time
		NOT SUPPORTED YET
	
	%wns <name>
		disable warning <name>
		only 1 warning at a time
		NOT SUPPORTED YET
	
	%wes <name>
		promote warning <name> to error
		only 1 warning at a time
		NOT SUPPORTED YET

	%wpsh
		push curent warning settings to stack
		NOT SUPPORTED YET

	%wpop
		pop current warning settings from stack
		if stack is empty, sets to command line arguments
		NOT SUPPORTED YET
	
	%define <name> <value>
		create constant <name> with <value>
		defines have to be referenced by prepending #
	
	%assign <name> <value>
		create variable <name> with <value>
		variables have to be referenced by $
		can be later reassigned, in this case '$' should be omitted

	%isdefined <name>
		checks if <name> is defined, if so returns 1, else returns 0
		can only be used in expression

	%isassigned <name>
		checks if <name> has been assigned, if so returns 1, else returns 0
		can only be used in expression
	
	%typeof	<argument>
		returns type id of <argument>
		returned number may change between different versions
		can only be used in expression
	
	%same <arg0> <arg1>
		returns 1 if <arg0> and <arg1> have the same types and value (if applicable)
		can only be used in expression 

	%if <value>
		if <value> /= 0, assembles following code until %else, %elsif or %end

	%elsif <value>
		must follow some %if
		if <value> /= 0 and all previous %if and %elsif's <value> evaluated to false
		 assembles following code until %else, %elsif or %end

	%else 
		must follow some %if or %elsif
		if all previous %if and %elsif's <value> evaluated to false
		 assembles following code until %end
		else skips to %end
	
	%end
		must follow %if, %else orr %elsif

	%assert <value> <string>
		almost shorthand for %if {<value> !} %error <string> %end
		however assert allows for non constant expressions

	%info <string>
		raises info with <string>
	
	%warn <string>
		raises [custom] warning with <string>
	
	%error <string>
		raises error with <string>

	%tostring <arg>
		converts <arg> to string
)";

char const* const string_help_expressions = R"(EXPRESSIONS
general:
	assembler uses postfix syntax
	that is values are evaluated from left to right and pushed onto stack
	whenever operator is encountered, specified # of arguments is popped and appropriate action is performed
	operators must be separated by space

	multi arguments are popped in reverse order 
	if operand takes arguments X and Y, first Y is popped, then X
	so 4 3 - would compute 1

	expressions must be enclosed in {}
	expressions can use labels defined ANYWHERE 
	expressions can use defines and variables created ABOVE them
	expressions can use following directives:
		%typeof %same %isdefined %isassigned

	comments cannot be used inside expressions

	within expressions function macros can be used

	expressions can be embedded within other expressions

	expressions may be classified as 'constant' this means that no label can be used within them
	if label is used in constant expression, appropriate error is thrown so full list of contexts is not documented here

available operators:
	+	arithmetic addition
		2 arguments
	-	arithmetic subtraction
		2 arguments
	*	arithmetic multiplication
		2 arguments
	/	arithmetic divide
		2 arguments
	%	arithmetic modulo
		2 arguments
	&	binary and
		2 arguments
	|	binary or
		2 arguments
	^	binary xor
		2 arguments
	~	binary not
		2 arguments
	?	convert to bool
		1 argument
	!	convert to bool, then negate
		1 argument
	=	if equal, return 1 else return 0
		2 arguments
	,	swap order of 2 arguments at the top 
		2 arguments
	.	duplicate arguments 
		1 argument
	#	remove last argument
		1 argument
	<	logical shift left, as defined by ISA
		2 arguments
	>	logical shift right, as defined by ISA
		2 arguments

	$	get address of instruction on that line
		can only be used in non constant expressions
		0 arguments

examples:
	;compute square
	{n . *}

	;get distance between labels
	{label1 label2 -}

	;if def is defined then output 3, else 0 
	;note that directive uses regular syntax
	{3 %isdefined #def *}

	;get distance from current address to label and get 8 most significant bits
	{$ label - 8 > 0xFF &}
)";

char const* const string_help_labels = R"(LABELS
general:
	labels are defined by string followed by colon
	string must match following regex 
		[a-zA-Z][a-zA-Z0-9_-]*
	string must not identify any instruction, internal or external register

	if a label starts with . then it is considered special label, see below

	label's full name is limited to 500 bytes
		full name is concatenation of name + top level name (if nested) 

	label behavior is different for jmp/cal/jcs/jcu (including expressions for them) than for the rest
	they get labels RELATIVE address while other get labels' ABSOLUTE address

special:
	.[a-zA-Z][a-zA-Z0-9_-]*
		nested label
		nested labels can be referenced by providing name with dots or by providing full name
		only 1 level of nestedness is supported

	.,
		define anonymous top level label
		can be referenced with .> .< and nested labels
	..
		define anonymous nested label 
		can be referenced with .> .< 
	.>
		ANY next label
	.<
		ANY previous label


examples:
	//create a label
	label:

	//nested label
	.loop:

	//reference to nested label
		jmp 	.loop

	//anonymous top level label
	..

	//reference to label in another tree 
		mov 	R0, label.loop
	
	//jump back to anonymous label
		jmp 	.<
	
	
)";


char const* const string_help_macros = R"(MACROS
general:
	NOT SUPPORTED YET

	all macro keywords and references start with '@'

	macros are defined by enclosing related piece of code with '@macro' and '@end' 
	macros support named parameters/arguments 
		parameters are defined by providing identifier
		arguments are specified by prepending identifier with `_`

	as an argument, macro can take any single token or expression
	if an argument is macro itself, it is evaluated AFTER main macro is to be evaluated

	need to work within expressions as functions
	embedded expressions would be useful
	macros should not cross string or expression boundary
	optimally disallow conditional crossing too

usage:
	macros can be used only after they have been defined
	syntax:
		name(arg_list...)
		name
	
	arg_list is list of tokens
	tokens can be separated with comma or whitespace

	name alone without parentheses can be used when macro takes no arguments


examples
	//macro used to define custom instruction
	@macro nand Reg Arg
		and 	_Reg, _Arg
		not 	_Reg, _Reg
	@end 
	//usage
	@nand(R0, R1)
	
	//macro used to create constant
	@macro zero
		0
	@end
	//usage
	@nand R0, @zero
	
	@macro transfer R V
		%if {%isexternal Reg}
			xwr 	_R, _V
		%else
			mov 	_R, _V
		%end
	@end

	//synthesize custom almost-directive
	@macro same_type A B
		{%typeof A %typeof B =}
	@end

)";




