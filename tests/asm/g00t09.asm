#this code tests whether all dependencies are properly forwarded in case of long dependency chain

	mov 	R0, 1
	mov 	R1, 2

	mov 	R3, R0
	mov 	R4, R3
	mov 	R0, R1
	mov 	R4, R4
	mov 	R5, R4
	mov 	R6, R5
	mov 	R7, R6
	mov 	R1, R7

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
