	mov 	R0, 5
	mov 	R1, R0
	shl 	R0, 2
	xwr 	UI, 1
	mov 	R2, 0
	shr 	R2, 7
loop:
	add 	R0, 1
	mul 	R0, R0
	sub 	R2, 1
	jcs 	EG, loop
	xwr 	UI, 1
	xrd 	R4, UI
	xwr 	LR, end
	ret
end2:
	mov 	R5, 9
	mul 	R5, 4
	mwr 	100, R5
	mrd 	R6, 100
	xor 	R5, R5
	xwr 	IP, halt
end:
	cal 	end2
halt:
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	hcf
