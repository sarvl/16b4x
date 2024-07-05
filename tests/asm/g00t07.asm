#basic test of all alu functions with dependencies
#failure probably indicates not working alu or wrong pipeline/OOOE implementaion 

	mov 	R0, 0
	mov 	R1, 1
	mov 	R2, 2
	mov 	R3, 3

	add 	R0, R1
	add 	R2, R3

	sub 	R1, R0
	sub 	R2, R3

	xor 	R0, R2
	xor 	R1, R3

	add 	R0, R0
	add 	R1, R1

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
