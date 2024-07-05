#this code tests what happens when jump  goes directly between xwr and use of imm
#mainly to make sure arch is not cheating by merging all xwr and suceeding use


	mov 	R0, 0
	mov 	R1, 1

	xwr 	UI, 0xFF
lp:
	add 	R0, 3

	sub 	R1, 1
	jcs 	GE lp 

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
