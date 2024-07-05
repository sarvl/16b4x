#test of ip register
#failrue indicates wrong implementation

	xwr 	IP, start


	mov 	R0, 123
start:
	xrd 	R7, IP
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
