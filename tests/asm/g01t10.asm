#check what happens with jmp to next instruction

	mov 	R0, 1
	mov 	R1, 2
	mov 	R2, 4
	mov 	R3, 8

	jmp 	next
next:

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
	
