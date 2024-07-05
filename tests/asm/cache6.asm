#this code makes sure that self modifying code works 
#failure indicates improper handling of self modification
#same as g02t05 but self modifying instruction is not aligned

	mov 	R1, 0 
	mov 	R2, 1 
loop:
	mrd 	R0, modify
	xor 	R0, 0xAA
	nop 
modify:
	mwr 	R0, modify

	sub 	R2, 1
	jcs 	GE loop

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
	
