#basic test of all alu functions, specifically avoiding hazards for short pipelines
#failure probably indicates not working alu 

	mov 	R0, 0
	mov 	R1, 1
	mov 	R2, 3
	mov 	R3, 6
	mov 	R4, 10
	mov 	R5, 15
	mov 	R6, 21
	mov 	R7, 28

	add 	R0, R1
	sub 	R1, R2
	not 	R2, R3
	and 	R3, R4
	orr 	R4, R5
	xor 	R5, R6
	shl 	R6, R7
	shr 	R7, R0

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf

