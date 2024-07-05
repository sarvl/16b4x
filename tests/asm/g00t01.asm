#basic test of all alu functions
#failure probably indicates not working alu 

 	mov 	R0, 0
 	mov 	R1, 1
 	mov 	R2, 2
 	mov 	R3, 3
 	mov 	R4, 4
 	mov 	R5, 5
 	mov 	R6, 6
 	mov 	R7, 7

	add 	R0, R1
	add 	R1, R2
	add 	R2, R3
	add 	R3, R4
	add 	R4, R5
	add 	R5, R6
	add 	R6, R7
	add 	R7, R0


	shl 	R0, 1
	shl 	R1, 1
	shl 	R2, 1
	shl 	R3, 1
	shl 	R4, 1
	shl 	R5, 1
	shl 	R6, 1
	shl 	R7, 1


	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
