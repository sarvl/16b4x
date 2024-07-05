#basic tests of dependencies in alu
#failure probably indicates wrong OOOE/pipeline implementation

	mov 	R0, 1
	mov 	R1, 2
	mov 	R2, 4
	mov 	R3, 8

	add 	R0, R2
	add 	R3, R0

	add 	R2, R1
	add 	R2, R0


	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7


	hcf

