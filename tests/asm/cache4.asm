#this code makes sure that self modifying code works 
#failure indicates improper caching

	mov 	R1, 0 
	mov 	R2, 1 
instr:
	add 	R1, 0xFF

	mrd 	R0, instr
	xor 	R0, 0xAA
	mwr 	instr, R0

	sub 	R2, 1
	jcs 	GE instr

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
	
