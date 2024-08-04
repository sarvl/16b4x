Boot code:
;check whether it is necessary to do anything
	pxr 	R0, 0x03
	tst 	R0, 0x01
	jcs 	LG skip
	xwr 	UI, 0x01
	mov 	R0, 0xFF
loop:
	pwr 	0x04, R0
	prd 	R1, 0x05
	mwr 	R0, R1
	sub 	R0, 1
	jcs 	GE loop
skip:
	mrd 	R1, 0x00
	pwr 	0x00, R1
	fin
TLB update code
	iwr 	0xF8, R0
	iwr 	0xF9, R1
	pxr 	R0, 1
	ird 	R1, 0xFF
	add 	R0, R1
	mrd 	R0, R0
	xwr 	UI, 0x80
	tst 	R0, 0x00
	jcs 	LG continue
	int 	0x01
continue:
	add 	R1, R1 ; R1 *= 2
	iwr 	R1, R0
	add 	R1, 1
	pxr 	R0, 0 ; fetch PID
	iwr 	R1, R0
	ird 	R0, 0xF8
	ird 	R1, 0xF9
	fin
TLB_flush:
	iwr 	0xF8, R0
	mov 	R0, 0x2E
lp:
	iwr 	R0, R0 ; safe because present bit is 0
	sub 	R0, 2  ; skip pid
	jcs 	GE lp
	ird 	R0, 0xF8
	fin
temp_start:
	;display something to the LEDs
	mov 	R0, 0b000001
	pwr 	0x00, R0
	xwr 	IP, 0
