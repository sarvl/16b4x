#this code tests whether IP is really read and writtern properly
#failure indicates invalid read after write 

	xwr 	IP, start
	hcf 
start:
	xrd 	R0, IP


	xwr 	IP, test
test:
	xrd 	R1, IP

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
