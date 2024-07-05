#test whether condition flags are set correctly for unsigned arithmetic
#wrong result indicates that some part of flag setting/reading doesnt work

	mov 	R0, 4
	cmp 	R0, 5
	jcu 	G g
	jcu 	E e
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
