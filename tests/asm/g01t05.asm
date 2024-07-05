#specifically designed to test whether jump to misaligned branch works
#this test failing while g01t04 working means that misaligned jumps do not work 

	mov 	R0, 1
	mov 	R1, 2
	mov 	R2, 4
	mov 	R3, 8


	cmp 	R3, R2
	jcs 	G, misaligned_address
	
	mov 	R4, 100
	mov 	R5, 104

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf 	

misaligned_address:
	mov 	R4, 16
	mov 	R5, 32

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf 	
