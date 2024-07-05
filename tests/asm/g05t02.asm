#3x3 matrix multiplication

	jmp 	start

; takes addr in R0 
; creates matrix of the form 
; 1 2 3
; 4 5 6
; 7 8 9
createmat3x3:
	mov 	R1, 9
	add 	R0, 16
createmat3x3_loop:
	mwr 	R0, R1
	sub 	R0, 2
	sub	 	R1, 1
	jcs 	G createmat3x3_loop

	ret 


;takes row adr of A in R0
;takes col adr of B in R1
;takes where to write in R2
;writes dot product to R2
;unrolled to improve performance
matmult3x3_rowcol:
; regular:
;
;	mrd 	R7, R0
;	mrd 	R5, R1
;	mul 	R7, R5
;	add 	R0, 2
;	add 	R1, 6
;	
;	mrd 	R4, R0
;	mrd 	R5, R1
;	mul 	R4, R5
;	add 	R7, R4
;	add 	R0, 2
;	add 	R1, 6
;	
;	mrd 	R4, R0
;	mrd 	R5, R1
;	mul 	R4, R5
;	add 	R7, R4
;
;	mwr 	R7, R2
;
;	ret
;
;reordered
	mrd 	R7, R0

	mrd 	R5, R1
	add 	R0, 2
	mul 	R7, R5
	add 	R1, 6
	
	mrd 	R4, R0
	mrd 	R5, R1
	add 	R0, 2
	mul 	R4, R5

	add 	R1, 6
	add 	R7, R4

	mrd 	R4, R0
	mrd 	R5, R1
	mul 	R4, R5
	add 	R7, R4

	mwr 	R2, R7

	ret


;takes mat0adr in R0
;takes mat1adr in R1
;takes mat2adr in R2

;calculates R0*R1 and puts result in R2 
matmult3x3:
	xrd 	R7, LR
	psh 	R7

;precompute starting addresses
	mwr 	200, R1
	add 	R1, 2
	mwr 	202, R1
	add 	R1, 2
	mwr 	204, R1

	mov 	R6, 3
matmult3x3_loop_outer:
	mwr 	254, R0

;unrolled loop
	mrd 	R1, 200
	;R0 already what is should be
	cal 	matmult3x3_rowcol
	add 	R2, 2
	
	mrd 	R0, 254
	mrd 	R1, 202
	cal 	matmult3x3_rowcol
	add 	R2, 2
	
	mrd 	R0, 254
	mrd 	R1, 204
	cal 	matmult3x3_rowcol
	add 	R2, 2


	mrd 	R0, 254
	add 	R0, 6
	sub 	R6, 1
	jcs 	G, matmult3x3_loop_outer


	pop 	R7
	xwr 	LR, R7
	ret


start:
	xwr 	UI, 1
	mov 	R0, 0
	cal 	createmat3x3
	xwr 	UI, 2 
	mov 	R0, 0
	cal 	createmat3x3

	
	xwr 	UI, 1
	mov 	R0, 0
	xwr 	UI, 2
	mov 	R1, 0
	xwr 	UI, 3
	mov 	R2, 0
	cal 	matmult3x3

	xwr 	UI, 3
	mrd 	R0, 0

	xwr 	UI, 3
	mrd 	R1, 1

	xwr 	UI, 3
	mrd 	R2, 2

	xwr 	UI, 3
	mrd 	R3, 3

	xwr 	UI, 3
	mrd 	R4, 4

	xwr 	UI, 3
	mrd 	R5, 5

	xwr 	UI, 3
	mrd 	R6, 6

	xwr 	UI, 3
	mrd 	R7, 7


	hcf

