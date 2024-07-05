#check whether cache properly differentiates tags 
#and whether data does not get overwritten while in cache
#not sure how to differentiate this two cases without relying on mem content which may be ZZZZ or 0000
	
	mov 	R0, 1
	mov 	R1, 2
	
#2MSb are def in same set
	wrx 	UI, 0x80
	mwr 	0x00, R0

	wrx 	UI, 0x90
	mwr 	0x00, R1

	wrx 	UI, 0x80
	mrd 	R2, 0x00

	wrx 	UI, 0x90
	mrd 	R3, 0x00

	mwr 	100, R0
	mwr 	102, R1
	mwr 	104, R2
	mwr 	106, R3
	mwr 	108, R4
	mwr 	110, R5
	mwr 	112, R6
	mwr 	114, R7

	hcf
