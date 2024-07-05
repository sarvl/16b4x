#this code is indented to make sure that branch predictor correctly predicts 2 branches, one T one NT 

start:
	mov 	R0, 0
	nop
	
	wrx 	UI, 0x1
	mov 	R1,   0 

loop:
	#always not taken
	cmp 	R1, 0
	jcs 	E skip

	add 	R0, 1

	sub 	R1, 1
	#always taken
	jcs 	GE loop
skip:
	
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
