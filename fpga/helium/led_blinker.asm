	mov 	R0, 0 
loop:
	mov 	R1, R0
	shr 	R1, 10

	pwr 	0, R1

	add 	R0, 1
	jmp 	loop
