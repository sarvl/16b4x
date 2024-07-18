CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CY := '\033[0;33m' #color yellow 

WARN := -Wlibrary -Wbinding -Wport -Wreserved -Wnested-comment -Wparenthesis -Wspecs -Wport-bounds -Wruntime-error -Wunused -Wnowrite -Wothers -Wuseless
#WARN := 
FILES := out/arithmetic.o out/decode.o out/register.o out/cpu.o out/memory.o out/timer.o out/main.o out/ghdl_stub.o

UPPER_LIMIT       :=   4000ns

default:
	@echo -e no option specified

clean:
	@rm -frv out/*.o *.o out/work-obj08.cf wave.ghw

refresh:
	@touch src/*.vhdl

clear:
	@clear

test: clear analyse elaborate
remake: clean refresh test 

trun: test run

run: clear
	@echo -e ${CR}running simulation${CD}
	@ghdl -r --workdir=out --std=08 ghdl_stub --wave=wave.ghw --stop-time=${UPPER_LIMIT}

elaborate: 
	@echo -e ${CR}elaborating${CD}
	@ghdl -e --workdir=out --std=08 ${WARN} ghdl_stub 

analyse: ${FILES}

${FILES}: out/%.o: src/%.vhdl
	@echo -e ${CY}analysing $^${CD}
	@ghdl -a --workdir=out --std=08 ${WARN} $^
