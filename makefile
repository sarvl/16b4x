CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CC := '\033[0;36m' #color cyan
CY := '\033[0;33m' #color yellow 

default:
	@echo -e no option specified
	@echo -e use make help     to get more detailed help
	@echo -e use make init     to create and compile all programs
	@echo -e use make initfull to create and compile all programs, and then to generate tests

help:
	@echo -e "16b4x - Sarvelian System"
	@echo -e "For documentation, see files in documentation/"
	@echo -e ""
	@echo -e "This Makefile provides convienient ways to run most common commands"
	@echo -e ""
	@echo -e "  'init'      -                       initialize all necessary stuff"
	@echo -e "  'initfull'  -                       initialize all necessary stuff, generate tests"
	@echo -e "'reinit'      - clear existing stuff, initialize all necessary stuff"
	@echo -e "'reinitfull'  - clear existing stuff, initialize all necessary stuff, generate tests"
	@echo -e ""
	@echo -e "'testgencpu'  - generate tests for cpu and simulator"
	@echo -e "'testgensasm' - generate tests for assembler"
	@echo -e "'testgenall'  - generate all tests"
	@echo -e ""
	@echo -e "'testcpu'     - run tests for cpu and simulator"
	@echo -e "'testsasm'    - run tests for assembler"
	@echo -e "'testall'     - run all tests"
	@echo -e ""
	@echo -e "'sasm'        - build assembler"
	@echo -e "'dasm'        - build disassembler"
	@echo -e "'cosi'        - build simulator"
	@echo -e "'fost'        - build autotester"
	@echo -e "'fgen'        - build test generator"
	@echo -e "By adding 'remake=true' it is possible to rebuild some stuff mentioned above"
	
remake := 

sasm: bogus 
	@$(MAKE) -C ./assembler/ $(if ${remake}, remake, sasm)
dasm: bogus 
	@$(MAKE) -C ./disassembler/ $(if ${remake}, remake, dasm)
cosi: bogus 
	@$(MAKE) -C ./simulator/ $(if ${remake}, remake, cosi)
fgen fost: bogus 
	@$(MAKE) -C ./autotest/ $(if ${remake}, remake, fost)

testgencpu: bogus
	@echo -e ${CC}=== GENERATING TESTS - CPU ===${CD}
	@./fgen sim -fS

testgensasm: bogus
	@echo -e ${CC}=== GENERATING TESTS - SASM ===${CD}
	@./fgen sasm -fS

testsasm: bogus
	@echo -e ${CC}=== RUNNING TESTS - ASSEMBLER ===${CD}
	@-./fost sasm -fS

testsim: bogus
	@echo -e ${CC}=== RUNNING TESTS - SIMULATOR ===${CD}
	@-./fost sim -fS


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
	@$(MAKE) -C ./autotest/ init
	@$(MAKE) -C ./autotest/ remake noclear=True
	ln -s ./autotest/fost ./
	ln -s ./autotest/fost ./fgen
	@echo -e ${CC}=== OTHER DIRECTORIES ===${CD}
	@mkdir -p tests/cpu/out/ tests/cpu/bin/
	@mkdir -p tests/sasm/fout/ tests/sasm/sout/
	@echo -e ${CC}done${CD}

clean:
	@echo -e ${CC}=== CLEANING ===${CD}
	@$(MAKE) -C ./utility/ fullclean
	@$(MAKE) -C ./assembler/ fullclean
	@$(MAKE) -C ./disassembler/ fullclean
	@$(MAKE) -C ./simulator/ fullclean
	@$(MAKE) -C ./autotest/ fullclean
	rm -f sasm dasm cosi fost fgen
	@echo -e ${CC}done${CD}

initfull: init testgen 

reinit: clean init
reinitfull: clean init initfull

testgen: testgencpu testgensasm
testall: testsasm testsim
	
bogus:
