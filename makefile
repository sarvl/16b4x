remake := 

sasm: bogus 
	@$(MAKE) -C ./assembler/ $(if ${remake}, remake, sasm)
dasm: bogus 
	@$(MAKE) -C ./disassembler/ $(if ${remake}, remake, dasm)
cosi: bogus 
	@$(MAKE) -C ./simulator/ $(if ${remake}, remake, cosi)
fost: bogus 
	@$(MAKE) -C ./tests/autotest/ $(if ${remake}, remake, fost)
gen: bogus 
	@$(MAKE) -C ./tests/generator/ $(if ${remake}, remake, gen)

bogus:
