#same as g05t04 but different program loaded
	jmp 	start

program:
#stack test:
	dw  	0x280A
	dw  	0x7000
	dw  	0xC801
	dw  	0xA301
	dw  	0x7900
	dw  	0x890A
	dw  	0xA504
	dw  	0x3864
	dw  	0x3966
	dw  	0x3A68
	dw  	0x3B6A
	dw  	0x3C6C
	dw  	0x3D6E
	dw  	0x3E70
	dw  	0x3F72
	dw  	0x0F00

registers:
r_0:
	dw  	0x0000	
r_1:
	dw  	0x0000	
r_2:
	dw  	0x0000	
r_3:
	dw  	0x0000	
r_4:
	dw  	0x0000	
r_5:
	dw  	0x0000	
r_6:
	dw  	0x0000	
r_7:
	dw  	0x0000	

@macro int_ip 
	R7
@end
@macro int_instr 
	R6
@end
@macro int_op
	R6
@end
@macro int_fl 
	R5
@end
@macro int_lr 
	R4
@end
@macro int_ui 
	R3
@end
@macro int_sp
	SP
@end
@macro int_r0
	R0
@end
@macro int_ccc
	R0
@end
@macro int_r1
	R1
@end
@macro int_imm
	R1
@end
@macro temp
	R2
@end

#due to assembler limitations nested macros do not work
#@macro arithmetic(ins)
#	add 	@int_r0, registers
#	shl 	@int_r0, 1
#	mov 	@temp, @int_r0
#	mrd 	@int_r0, @temp
#
#	_ins 	@int_r0, @int_r1
#
#	mwr 	@int_r0, @temp
#@end
@macro arithmetic(ins)
	#loads R0 with correct value
	add 	R0, registers
	shl 	R0, 1
	mov 	R2, R0 
	mrd 	R0, R2 

	#performs operations 
	_ins 	R0, R1 

	#stores op
	mwr 	R0, R2 
@end

@macro jmp_if_equal(a, b, dest)
	cmp 	_a, _b
	jmp 	E, _dest
@end

start:
	mov 	@int_ip, program
	sub 	@int_ip, 1
loop:
	#int_instr = mem[(int_ip + prog) * 2] 
	mov 	@temp, @int_ip
	add 	@temp, program
	shl 	@temp, 1
	mrd 	@int_instr, @temp
	add 	@int_ip, 1

	#int_r0 = (int_instr >> 8) & 0x7
	mov 	@int_r0, @int_instr 
	shr 	@int_r0, 8
	and 	@int_r0, 0x07

	#int_r1 = int_instr & 0xFF
	mov 	@int_r1, @int_instr 
	and 	@int_r1, 0xFF

	#if 0 == (int_instr & 0xF800) goto format_register
	#else goto format_immiediate
	wrx 	UI, 0xF8
	tst 	@int_instr, 0
	jmp 	Z, format_r

format_i:
	#merge immiediate with upper immiediate
	shl 	@int_ui, 8
	orr 	@int_r1, @int_ui

	shr 	@int_op, 11
	jmp 	execute
format_r:
	shr 	@int_r1, 5
	and 	@int_instr, 0x1F
	mov 	@temp, @int_r1


	#int_r1 = reg[(registers + int_r1) * 2]
	add 	@int_r1, registers
	shl 	@int_r1, 1
	mrd 	@int_r1, @int_r1

execute:
	#jmp mem[(jump_table + int_op) * 2]
	wrx 	UI,      {jump_table 256 /}
	add 	@int_op, {jump_table 256 %}
	shl 	@int_op, 1
	mrd 	@int_op, @int_op
	#jmp below, makes indirect jump faster 

	wrx 	UI,      {ui_storage 0x7F /}
	mwr 	@int_ui, {ui_storage 0xFF % 2 *}
	mov 	@int_ui, 0
	jmp 	@int_op

NOP:
	jmp 	loop
HLT:
	#if int_fl & ccc != 0
	#then halt 
	tst 	@int_fl, @int_ccc

	jmp 	Z, loop 
	wrx 	UI, {loop_end 256 /}
	jmp 	    {loop_end 256 %}

MOV:
	#mem[(registers + int_r0) * 2] = int_r1
	add 	@int_r0, registers
	shl 	@int_r0, 1
	mwr 	@int_r1, @int_r0
	
	jmp 	loop
RDM:
	#int_r1 from now on is address
	#0x7F to incorporate shift
	wrx 	UI,      {program_data 0x7F / }
	add 	@int_r1, {program_data 0xFF % 2 *}

	mrd 	@temp, @int_r1

	add 	@int_r0, registers
	shl 	@int_r0, 1
	mwr 	@temp, @int_r0

	jmp 	loop
WRM:
	#int_r0 = mem[(registers + int_r0) * 2]
	add 	@int_r0, registers
	shl 	@int_r0, 1
	mrd 	@int_r0, @int_r0
	
	#int_r1 from now on is address
	#0x7F to incorporate shift
	wrx 	UI,      {program_data 0x7F / }
	add 	@int_r1, {program_data 0xFF % 2 *}

	mwr 	@int_r0, @int_r1

	jmp 	loop
RDX:
	add 	@int_r0, registers
	shl 	@int_r0, 1

	@jmp_if_equal(R2, 0, RDX_IP)
	@jmp_if_equal(R2, 1, RDX_SP)
	@jmp_if_equal(R2, 2, RDX_LR)
	@jmp_if_equal(R2, 4, RDX_UI)
	@jmp_if_equal(R2, 5, RDX_FL)
	@jmp_if_equal(R2, 7, RDX_CF)

	jmp 	loop
RDX_IP:
	#mem[(registers + int_r0) * 2] = int_ip
	mwr 	@int_ip, @int_r0
	jmp 	loop
RDX_SP:
	#mem[(registers + int_r0) * 2] = int_sp
	rdx 	@temp, SP
	mwr 	@temp, @int_r0
	jmp 	loop
RDX_LR:
	#mem[(registers + int_r0) * 2] = int_lr
	mwr 	@int_lr, @int_r0
	jmp 	loop
RDX_UI:
	#mem[(registers + int_r0) * 2] = int_ui
	wrx 	UI,    {ui_storage 0x7F /}
	mrd 	@temp, {ui_storage 0xFF % 2 *}

	mwr 	@temp, @int_r0
	jmp 	loop
RDX_FL:
	#mem[(registers + int_r0) * 2] = int_fl
	mwr 	@int_fl, @int_r0
	jmp 	loop
RDX_CF:
	mov 	@temp, 0x1
	mwr 	@temp, @int_r0
	jmp 	loop

WRX:
	@jmp_if_equal(R0, 0, WRX_IP)
	@jmp_if_equal(R0, 1, WRX_SP)
	@jmp_if_equal(R0, 2, WRX_LR)
	@jmp_if_equal(R0, 4, WRX_UI)
	@jmp_if_equal(R0, 5, WRX_FL)

	jmp 	loop
WRX_IP:
	mov 	@int_ip, @int_r1
	jmp 	loop
WRX_SP:
	wrx 	SP, @int_r1
	jmp 	loop
WRX_LR:
	mov 	@int_lr, @int_r1
	jmp 	loop
WRX_UI:
	mov 	@int_ui, @int_r1
	jmp 	loop
WRX_FL:
	mov 	@int_fl, @int_r1
	jmp 	loop

PSH:
	#int_r0 = mem[(registers + int_r0) * 2]
	add 	@int_r0, registers
	shl 	@int_r0, 1
	mrd 	@int_r0, @int_r0
   
	psh 	@int_r0

	jmp 	loop
POP:
	pop 	@temp

	#mem[(registers + int_r0) * 2] = temp
	add 	@int_r0, registers
	shl 	@int_r0, 1
	mwr 	@temp, @int_r0

	jmp 	loop
MUL:
	@arithmetic(mul)
	rdx 	@int_fl, FL
	jmp 	loop
CMP:
	#int_r0 = mem[(registers + int_r0) * 2]
	add 	@int_r0, registers
	shl 	@int_r0, 1
	mrd 	@int_r0, @int_r0
	
	#int_fl = flags(int_r0 - int_r1)
	cmp 	@int_r0, @int_r1
	rdx 	@int_fl, FL

	jmp 	loop

TST:
	#int_r0 = mem[(registers + int_r0) * 2]
	add 	@int_r0, registers
	shl 	@int_r0, 1
	mrd 	@int_r0, @int_r0
	
	#int_fl = flags(int_r0 - int_r1)
	tst 	@int_r0, @int_r1
	rdx 	@int_fl, FL

	jmp 	loop

JMP:
	#if int_fl & ccc != 0
	#then int_ip = int_r1
	tst 	@int_fl, @int_ccc

	jmp 	Z, loop 
	mov 	@int_ip, @int_r1
	jmp 	loop

CAL:
	#if int_fl & ccc != 0
	#then int_ip = int_r1
	#     int_lr = int_ip
	tst 	@int_fl, @int_ccc

	jmp 	Z, loop 
	mov 	@int_lr, @int_ip
	mov 	@int_ip, @int_r1
	jmp 	loop

RET:
	#if int_fl & ccc != 0
	#then int_ip = int_lr
	tst 	@int_fl, @int_ccc

	jmp 	Z, loop 
	mov 	@int_ip, @int_lr
	jmp 	loop

ADD:
	@arithmetic(add)
	rdx 	@int_fl, FL
	jmp 	loop
SUB:
	@arithmetic(sub)
	rdx 	@int_fl, FL
	jmp 	loop
NOT:
	@arithmetic(not)
	rdx 	@int_fl, FL
	jmp 	loop
AND:
	@arithmetic(and)
	rdx 	@int_fl, FL
	jmp 	loop
ORR:
	@arithmetic(orr)
	rdx 	@int_fl, FL
	jmp 	loop
XOR:
	@arithmetic(xor)
	rdx 	@int_fl, FL
	jmp 	loop
SLL:
	@arithmetic(shl)
	jmp 	loop
SLR:
	@arithmetic(shr)
	jmp 	loop

loop_end:
	wrx 	UI, {r_0 2 * 256 /}
	mrd 	R0, {r_0 2 * 256 %}
	wrx 	UI, {r_1 2 * 256 /}
	mrd 	R1, {r_1 2 * 256 %}
	wrx 	UI, {r_2 2 * 256 /}
	mrd 	R2, {r_2 2 * 256 %}
	wrx 	UI, {r_3 2 * 256 /}
	mrd 	R3, {r_3 2 * 256 %}
	wrx 	UI, {r_4 2 * 256 /}
	mrd 	R4, {r_4 2 * 256 %}
	wrx 	UI, {r_5 2 * 256 /}
	mrd 	R5, {r_5 2 * 256 %}
	wrx 	UI, {r_6 2 * 256 /}
	mrd 	R6, {r_6 2 * 256 %}
	wrx 	UI, {r_7 2 * 256 /}
	mrd 	R7, {r_7 2 * 256 %}


	hlt

jump_table:
	dw  	NOP
	dw  	HLT
	dw  	0x0000
	dw  	0x0000
	dw  	0x0000
	dw  	MOV
	dw  	RDM
	dw  	WRM
	dw  	0x0000
	dw  	0x0000
	dw  	0x0000
	dw  	0x0000
	dw  	RDX
	dw  	WRX
	dw  	PSH
	dw  	POP
	dw  	MUL
	dw  	CMP
	dw  	0x0000
	dw  	TST
	dw  	JMP
	dw  	CAL
	dw  	RET
	dw  	0x0000
	dw  	ADD
	dw  	SUB
	dw  	NOT
	dw  	AND
	dw  	ORR
	dw  	XOR
	dw  	SLL
	dw  	SLR

ui_storage:
	dw   	0x0000

#added as offset to all RDM and WRM to NOT destroy code of the program itself
program_data:
	nop

