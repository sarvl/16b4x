#common pattern of pushing and poping LR 

	jmp 	start

f1:
	add 	R0, 2
	ret

f2:
	xrd 	R7, LR
	psh 	R7

	add 	R0, 1

	cal 	f1

	add 	R0, 3


	pop 	R7
	xwr 	LR, R7
	ret


start:
	mov 	R0, 0
	cal 	f2

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
