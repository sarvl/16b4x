#program designed to test cache correctness by writing to addr with same index but different tag 
#in total 12 cache misses should be generated in writeback cache
#         11                                  in writethrough cache

# out of this file make several test groups to test cache behavior 
# following problems may occur:
#	does not properly differentiate tag
#	data gets overwritten while in cache (wrong eviction, only in writeback)
#   data does not get properly arbitrated when writeback writes evicted AND wants to store read
#
# these ones may occur but they usually have to be detected manually 
#  or maybe with added perf counters but are highly dependent on initial configuration of cache
# cache misses are not properly aligned to instr that caused them 
# 

	mov 	R0, 1
	mov 	R1, 2
	mov 	R2, 3
	mov 	R3, 4

	wrx 	UI, 0x5
#cache miss because of evicting last value 
	mwr 	0, R0 

	wrx 	UI, 0x5
#cache hit but write different value so if cache does not properly update memory, it may break 
#however in writeback cache this should not generate a miss
	mwr 	0, R1

	wrx 	UI, 0x6
#cache miss
#in writeback it should write data
	mwr 	0, R1
	wrx 	UI, 0x7
#cache miss
#in writeback it should write data
	mwr 	0, R2
	wrx 	UI, 0x8
#cache miss
#in writeback it should write data
	mwr 	0, R3

	wrx 	UI, 0x5
#cache miss
#in writeback this should generate double miss
#one for write old and one for read R4
	mrd 	R4, 0
	wrx 	UI, 0x6
#cache miss
#single, only read
	mrd 	R5, 0
	wrx 	UI, 0x7
#cache miss
#single, only read
	mrd 	R6, 0
	wrx 	UI, 0x8
#cache miss
#single, only read
	mrd 	R7, 0

#now evict next instruction, marked with label
#single, only read
	wrx 	UI, 0x5
#cache miss
#evict instruction 
#(that actually depends on how the stuff is inited)
	mwr 	evict, R4

evict:
#cache miss
#double in writeback
#write prev and read instr
	mov 	R0, 0x96

	mwr 	100, R0
	mwr 	102, R1
	mwr 	104, R2
	mwr 	106, R3
	mwr 	108, R4
	mwr 	110, R5
	mwr 	112, R6
	mwr 	114, R7

	hcf 	

