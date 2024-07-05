#this tests detects problem with forwarding that may occur due to memory operations

	mov 	R0, 1
	mov 	R1, 2

	xwr 	UI, 0x5
	mwr 	0, R0
	xwr 	UI, 0x6
	mwr 	0, R1

	xwr 	UI, 0x5
	mrd 	R2, 0
	xwr 	UI, 0x6
	mrd 	R3, 0

	xwr 	UI, 0x7
	mwr 	0, R3

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
