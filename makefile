CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CC := '\033[0;36m' #color cyan
CY := '\033[0;33m' #color yellow 

default:
	@echo -e no option specified
	@echo -e use make init     to create and compile all programs
	@echo -e use make initfull to create and compile all programs, and then to generate tests
	
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

testgensim: bogus
	@echo -e ${CC}=== GENERATING TESTS - CPU ===${CD}
	@./tests/generator/gen

testsim: bogus
	@echo -e ${CC}=== RUNNING TESTS - SIMULATOR ===${CD}
	@./fost -fS

init:
	@clear
	@echo -e ${CC}====== INITTING ======${CD}
	@echo -e ${CC}=== UTILITY ===${CD}
	@$(MAKE) -C ./utility/ init 
	@$(MAKE) -C ./utility/ remake noclear=True
	@echo -e ${CC}=== ASSEMBLER ===${CD}
	@$(MAKE) -C ./assembler/ init
	@$(MAKE) -C ./assembler/ remake noclear=True
	ln -s ./assembler/sasm ./
	@echo -e ${CC}=== DISASSEMBLER ===${CD}
	@$(MAKE) -C ./disassembler/ init
	@$(MAKE) -C ./disassembler/ remake noclear=True
	ln -s ./disassembler/dasm ./
	@echo -e ${CC}=== SIMULATOR ===${CD}
	@$(MAKE) -C ./simulator/ init
	@$(MAKE) -C ./simulator/ remake noclear=True
	ln -s ./simulator/cosi ./
	@echo -e ${CC}=== AUTOTEST ===${CD}
	@$(MAKE) -C ./tests/autotest/ init
	@$(MAKE) -C ./tests/autotest/ remake noclear=True
	ln -s ./tests/autotest/fost ./
	@echo -e ${CC}=== TESTGEN ===${CD}
	@$(MAKE) -C ./tests/generator/ init
	@$(MAKE) -C ./tests/generator/ remake noclear=True
	@echo -e ${CC}=== OTHER DIRECTORIES ===${CD}
	@mkdir -p tests/out/ tests/bin/
	@echo -e ${CC}done${CD}

clean:
	@echo -e ${CC}=== CLEANING ===${CD}
	@$(MAKE) -C ./utility/ fullclean
	@$(MAKE) -C ./assembler/ fullclean
	@$(MAKE) -C ./disassembler/ fullclean
	@$(MAKE) -C ./simulator/ fullclean
	@$(MAKE) -C ./tests/autotest/ fullclean
	@$(MAKE) -C ./tests/generator/ fullclean
	rm -f sasm dasm cosi fost
	@echo -e ${CC}done${CD}

initfull: init testgen 

reinit: clean init
reinitfull: clean init initfull

testgen: testgensim
testrun: testsim
	
bogus:
