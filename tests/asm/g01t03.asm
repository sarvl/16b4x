#test whether condition flags are set correctly for signed arithmetic
#wrong result indicates that some part of flag setting/reading doesnt work

	cmp 	R0, 0
	jcs 	G g
	jcs 	E e
l:
	mov 	R0, 1
	mov 	R1, 2
e:
	mov 	R2, 3
	mov 	R3, 4
g:
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf 	
