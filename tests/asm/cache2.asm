#check whether data gets properly arbitrated when writeback writes evicted AND wants to store read

	mov 	R0, 1
	mov 	R1, 2
	mov 	R2, 4
	mov 	R3, 8

	wrx 	UI, 0x80
	mwr 	0x00, R0
	wrx 	UI, 0x90
	mwr 	0x00, R1

	wrx 	UI, 0x80
	mwr 	0x00, R0
	wrx 	UI, 0x90
	mrd 	R5, 0x00


	mwr 	100, R0
	mwr 	102, R1
	mwr 	104, R2
	mwr 	106, R3
	mwr 	108, R4
	mwr 	110, R5
	mwr 	112, R6
	mwr 	114, R7

	hcf

