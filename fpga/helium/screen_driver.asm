//restart, not reset
interrupt_00:
	jmp 	start
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop

//timer 
interrupt_01:
	mov 	R4, 1 
	mwr 	interrupt_01_state, R4
	irt
	nop

	nop
	nop
	nop
interrupt_01_state:
//only low bit used, which is 0, good enough as default
	nop

//blink led
interrupt_10:
	mrd 	R4, interrupt_10_state
	not 	R4, R4 
	pwr 	0x00, R4
	mwr 	interrupt_10_state, R4

	irt
	nop
	nop
interrupt_10_state:
	nop ; low 6 are 0, good enough

//nothing for now
interrupt_11:
	irt
	nop
	nop
	nop

	nop
	nop
	nop
	nop


wait_for_1ms:
	mrd 	R7, interrupt_01_state
	tst 	R7, 0b1
	jcs 	Z wait_for_1ms
	mov 	R7, 0
	mwr 	interrupt_01_state, R7
	ret

//argument in R0
//if R1 = 0 then command else data
send_byte:
	xrd 	R5, LR

	//sync to make sure that 1ms is really waited
	cal 	wait_for_1ms

	//set RS to R1
	pwr 	0x02, R1

	cal 	wait_for_1ms

	//write the data
	pwr 	0x01, R0

	cal 	wait_for_1ms

	//toggle enable
	mov 	R2, 1 
	pwr 	0x03, R2
	cal 	wait_for_1ms
	mov 	R2, 0 
	pwr 	0x03, R2


	cal 	wait_for_1ms

	xwr 	LR, R5
	ret



start:
	mov 	R0, 0
	mov 	R6, 0
	//set RS pin to 0 
	pwr 	0x02, R0
	//set E  pin to 0 
	pwr 	0x03, R0

	//send initial byte
	mov 	R0, 0b00111000
	mov 	R1, 0
	cal 	send_byte

	//wait 100ms 
	mov 	R2, 100
wait_100:
	cal 	wait_for_1ms
	sub 	R2, 1
	jcs 	G wait_100
	
	mov 	R3, 1
repeat:
	//send byte
	mov 	R0, 0b00111000
	//R1 is set properly
	cal 	send_byte

	//wait 10ms 
wait_10:
	mov 	R2, 10
lp:
	cal 	wait_for_1ms
	sub 	R2, 1
	jcs 	G lp

	sub 	R3, 1
	jcs 	GZ repeat


	//send more commands
	mov 	R0, 0b0001100
	cal 	send_byte
	mov 	R0, 0b0000001
	cal 	send_byte
	mov 	R0, 0b0000110
	cal 	send_byte

	//start sending data
	mov 	R1, 1

	mov 	R0, 0b01001000 //'H'
	cal 	send_byte
	mov 	R0, 0b01000001 //'A'
	cal 	send_byte
	mov 	R0, 0b01001001 //'I'
	cal 	send_byte
	mov 	R0, 0b01001100 //'L'
	cal 	send_byte
	mov 	R0, 0b00100000 //' '
	cal 	send_byte
	mov 	R0, 0b01001111 //'O'
	cal 	send_byte
	mov 	R0, 0b01001101 //'M'
	cal 	send_byte
	mov 	R0, 0b01001110 //'N'
	cal 	send_byte
	mov 	R0, 0b01001001 //'I'
	cal 	send_byte
	mov 	R0, 0b01010011 //'S'
	cal 	send_byte
	mov 	R0, 0b01010011 //'S'
	cal 	send_byte
	mov 	R0, 0b01001001 //'I'
	cal 	send_byte
	mov 	R0, 0b01000001 //'A'
	cal 	send_byte
	mov 	R0, 0b01001000 //'H'
	cal 	send_byte
	
end:
	jmp 	end
