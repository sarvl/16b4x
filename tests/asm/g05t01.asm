	jmp 	start

; returns res in R0
; takes arg0 in R0
; takes arg1 in R1
; it is advised to put smaller value into R1 

;x0001
mult:
	cmp 	R1, 0 
	jcs 	LG mult_count
	
	mov 	R0, 0
	jmp 	mult_end

mult_count:
	mov 	R2, R0 

	sub 	R1, 1
mult_loop:
	add 	R0, R2

	sub 	R1, 1
	jcs 	G mult_loop

mult_end:
	ret

; returns factorial in R0
; takes n in R0

;x000B
fact:
	xrd 	R7, LR
	psh 	R7


	cmp 	R0, 1
	jcs 	LE fact_ret

	psh 	R0
	sub 	R0, 1
	jcs     L cal_skip
	cal 	fact
cal_skip:
	
	pop 	R1
	; R0 = fact(n - 1)
	; R1 = n 
	cal 	mult
	jmp 	fact_end

fact_ret:
	mov 	R0, 1
fact_end:
; link register
	pop 	R7
	xwr 	LR, R7
	ret

;x0019
start:
	mov 	R0, 6
	cal 	fact


	mwr 	80, R0
	mwr 	100, R0
	mwr 	101, R1
	mwr 	102, R2
	mwr 	103, R3
	mwr 	104, R4
	mwr 	105, R5
	mwr 	106, R6
	mwr 	107, R7
	hcf
	
