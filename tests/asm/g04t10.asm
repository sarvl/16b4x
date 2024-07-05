#this code tests long dependency chain with UI


	xwr 	UI, 1
	xwr 	UI, 2
	xwr 	UI, 3
	xwr 	UI, 4
	xwr 	UI, 5
	xwr 	UI, 6
	xwr 	UI, 7
	add 	R0, 8


	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
