#basic test of alu and jumps
#failure indicates either not working

	mov 	R0, 123
	mov 	R1, 72
	mov 	R3, 10
	nop
start:
	add 	R0, 41
	xor 	R0, R1
	shl 	R0, 5
	xor 	R1, R0
	sub 	R3, 1

	jcs 	G, start

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf

