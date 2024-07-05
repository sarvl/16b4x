#code to test whether psh/pop work
#failure incidates generally wrong push/pop implementation

	mov 	R0, 10

loopa:
	psh 	R0
	sub 	R0, 1
	jcs 	GE loopa

	mov 	R0, 10
loopb:
	pop 	R1
	sub 	R0, 1
	jcs 	GE loopb

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
