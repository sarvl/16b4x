#this code tests long dependency chain with UI using registers

	xwr 	UI, 1
	mov 	R0, 0
	xwr 	UI, R0
	mov 	R0, 0
	xwr 	UI, R0
	xwr 	UI, 4
	nop
	xwr 	UI, R0
	mov 	R1, 0
	xwr 	UI, R1
	nop
	nop
	xwr 	UI, R1
	mov 	R3, 0


	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
