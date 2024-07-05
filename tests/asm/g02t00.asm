#test designed to test whether memory read/write works
#failure here if previous tests work indicates that either read doesnt work or oooe/pipeline implementation is wrong

	mov 	R0, 100
	mov 	R1, 104

	mwr 	100, R0
	mwr 	104, R1 

	mrd 	R5, 100
	mrd 	R6, 104 

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
