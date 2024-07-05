	jmp 	start

;takes src  in R0
;takes dst  in R1
;takes size in R2
memcpy:
	cmp 	R2, 0
	jcs 	G skip_ret
	ret 	
skip_ret:

	mrd 	R3, R0
	mwr 	R1, R3

	add 	R0, 2
	add 	R1, 2
	sub 	R2, 2
	jmp 	memcpy

; takes ptr  in R0 
; takes size IN BYTES in R1
bubble_sort:
	mov 	R6, R0
	add 	R6, R1

	cmp 	R1, 2
	jcs 	G skip_ret_bs
	ret 	
skip_ret_bs:

	mov 	R2, R0
	mov 	R3, R0
	add 	R3, 2

bubble_sort_loop:
	mrd 	R4, R2
	mrd 	R5, R3
	
	cmp 	R4, R5
	; R3 is not greater
	jcs 	LE bubble_sort_loop_cont

	;R3 is greater
	mwr 	R5, R2
	mwr 	R4, R3

bubble_sort_loop_cont:
	add 	R2, 2
	add 	R3, 2
	
	cmp 	R6, R3
	jcs 	L bubble_sort_loop

	sub 	R1, 2
	jcs 	G bubble_sort

	ret


start:
	; populate memory 

	mov 	R6, 164
	mov 	R0, 1
start_loop:
	mul 	R0, 123

	shl 	R0, 1
	shr 	R0, 1
	mwr 	R6, R0
	sub 	R6, 2
	cmp 	R6, 150
	jcs 	GZ, start_loop

	mov 	R0, 150
	mov 	R1, 200
	mov 	R2, 16
	cal 	memcpy

	mov 	R0, 150 
	mov 	R1, 16
	cal 	bubble_sort

print:
	mrd 	R0, 150 
	mrd 	R1, 152 
	mrd 	R2, 154 
	mrd 	R3, 156 
	mrd 	R4, 158 
	mrd 	R5, 160
	mrd 	R6, 162
	mrd 	R7, 164

	hcf

