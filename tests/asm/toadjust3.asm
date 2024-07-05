;implementation of quicksort, tests performance on largeish data sets 

@def COUNT 25
@def ARR_H 0x5
@def ARR_L 0x0

	jmp 	start

[ALIGN 2]
;num in R0
xorshift:
	;x ^= x << 7
	mov 	R1, R0 
	shl 	R1, 7
	xor 	R0, R1

	;x ^= x >> 9
	mov 	R1, R0 
	shr 	R1, 9
	xor 	R0, R1

	;x ^= x << 8
	mov 	R1, R0 
	shl 	R1, 8
	xor 	R0, R1

	ret

;R0 contains ptr start 
;R1 contains ptr end inclusive
;R1 - R0 >= 2
qsort:
	rdx 	R7, LR
	psh 	R7

	psh 	R0
	mov 	R6, R1 ; save

;partition
	mov 	R3, R0 
	mrd	 	R2, R1

[ALIGN 2]
partition_loop:
	mrd 	R4, R3 
	cmp 	R4, R2 
	jmp 	G partition_loop_if_skip

	;swap
	mrd 	R5, R0 
	mwr 	R5, R3
	mwr 	R4, R0

	add 	R0, 2

partition_loop_if_skip:

	add 	R3, 2
	cmp 	R3, R1
	jmp 	L partition_loop
	
partition_loop_end:

	;swap
	mrd 	R5, R0 
	mwr 	R5, R1
	mwr 	R2, R0
;partition_end

	mov 	R1, R6 ;restore 

	psh 	R0

	add 	R0, 2

	cmp 	R0, R1
	cal 	L qsort

	pop 	R1
	pop 	R0

	sub 	R1, 2
	pop 	R7
	wrx 	LR, R7

	cmp 	R0, R1
	jmp 	L qsort
	jmp 	R7
	ret


[ALIGN 2]
start:
	;load arr start to R6
	wrx 	UI, ARR_H 
	mov 	R6, ARR_L 

	;load element count to R7
	mov 	R7, COUNT 
	;get starting xorshift val to R0
	mov 	R0, 42 

populate_arr:
	cal  	xorshift
	;make sure value is unsigned
 	shr 	R0, 1
	mwr 	R0, R6
	add 	R6, 2
	sub 	R7, 1
	jmp 	G, populate_arr

	;load arr start to R0
	wrx 	UI, ARR_H 
	mov 	R0, ARR_L 

	;load end inclusive to R1
	mov 	R1, COUNT 
	shl 	R1, 1
	add 	R1, R0
	sub 	R1, 2

	cal 	qsort

	hlt
	
	
