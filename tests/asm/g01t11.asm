#code that checks whether cal and ret are working

	jmp 	start

proc:
	add 	R0, R1
	ret


start:
	mov 	R0, 0
	mov 	R1, 1

	cal 	proc

	mov 	R2, R0

	mov 	R0, 1
	mov 	R1, 2
	cal 	proc

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf

