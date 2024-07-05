#tests whether bytes are saved to stack in the right order
#failure means that LSB is saved to lower address instead of higher

	xwr 	UI, 18
	mov 	R0, 52
	psh 	R0
	
	mov 	R1, 0
	sub 	R1, 1 
	mrd 	R3, R1
	pop 	R4

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf 
