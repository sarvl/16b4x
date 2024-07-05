#test of LR register
#failure indicates wrong implementation 

	jmp 	start
proc:
	mov 	R0, 100
	mov 	R1, 101
	ret

proct:
	mov 	R4, 104
	xwr 	LR, end
	mov 	R5, 105
	mov 	R6, 106
	mov 	R7, 107
	ret

procx:
	mov 	R6, 1
	mov 	R7, 2
	xrd 	R5, LR
	xwr 	IP, R5
	

start:
	mov 	R2, 102
	cal 	proc
	mov 	R3, 103
	cal 	proct
	mov 	R0, 0
	mov 	R1, 0
	mov 	R2, 0
	mov 	R3, 0
	mov 	R4, 0
	mov 	R5, 0
	mov 	R6, 0
	mov 	R7, 0
end:
	cal 	procx

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
	
