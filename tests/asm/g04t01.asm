#test of SP register
#failure incidates wrong implementation

	mov 	R0, 123

	psh 	R0
	psh 	R0
	psh 	R0 

	xwr 	SP, 0
	
	mov 	R1, 42
	psh 	R1
	psh 	R1

	xrd 	R7, SP

	xwr 	SP, 0
	mov 	R2, 32
	psh 	R2

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
