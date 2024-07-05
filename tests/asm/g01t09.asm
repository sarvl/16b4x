#checks whether memory writes are properly NOT executing after the branch
#may be an issue in pipeline implementation
	mov 	R0, 9
	mov 	R1, 8

	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7

	jmp 	end
	mwr 	114, R1
end:
	hcf 	


