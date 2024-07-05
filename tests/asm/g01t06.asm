#progam computing fibonacci number
#failure indicates not working flags/branching

	mov 	R0, 0
	mov 	R1, 1

	mov 	R7, 7

	cmp 	R7, 0
	jcs 	LE, end

loop: 
	mov 	R2, R0
	mov 	R0, R1
	add 	R1, R2

	sub 	R7, 1
	jcs 	GE, loop

end:
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
