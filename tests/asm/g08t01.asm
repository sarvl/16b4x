#this code tests whether div instruction is working 
#failure indicates lack of support (in which case this is NOT an error)
# or improper multiplication support

	mov 	R0, 123
	mov 	R1, 45
	div 	R0, R1
	div 	R3, R0

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
