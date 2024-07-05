#tests whether UI register works when instructions are not aligned
#failure indicates that value is not properly cleared or not properly used by next instruction

	xwr 	UI, 123
	mov 	R0, 123
	nop
	xwr 	UI, 1
	mov 	R1, 1

	sub 	R0, R1
	xor 	R3, R0 
	add 	R0, R3
	mov 	R4, R0
	and 	R4, R3
	orr 	R5, R3

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
	
