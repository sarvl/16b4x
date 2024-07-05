#tests whether indirect jump works 
#THIS TEST IS WRONG UNTIL ARITHMETIC IS OSSIIBLE TO PROPERLY CALCULATE ADDRESSES

	mov 	R5, mid
	mov 	R6, end
	mov 	R7, start
	tst  	R0, R0
	jmp 	R7	
	mov 	R0, 123
mid:
	jmp 	R6
	mov 	R1, 45
start:
	jmp 	R5
	mov 	R2, 67
end:
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
