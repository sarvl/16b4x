	mov 	R0, 2
	xwr 	LR, 3
lp:
	cal 	lb0 
	pop 	R1
	cal 	lb1	
	cmp 	R0, 0
	jcs 	GE lp 

	mwr 	80, R0
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf

lb0:
	ret
lb1:
	sub 	R0, 1
	ret
