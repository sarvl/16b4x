#test whether BP can properly predict alternating branches

start:
	mov 	R0, 0
	nop
	
	wrx 	UI, 0x1
	mov 	R1,   0 

loop:
	#alternates
	tst 	R1, 1
	jcs 	LG skip
	add 	R0, 1
skip:
	sub 	R1, 1
	#always taken
	jmp 	GE loop
	
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
