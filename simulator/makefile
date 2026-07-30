CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CC := '\033[0;36m' #color cyan
CY := '\033[0;33m' #color yellow 

FILES_LOCAL := out/cr.o  out/helper.o  out/instruction.o  out/interrupt.o  out/io.o  out/main.o  out/memory.o  out/privilege.o out/internal_state.o 
FILES_UTIL  := ../utility/out/log.o 
FILES := ${FILES_LOCAL} ${FILES_UTIL}

debug := 
noclear :=
SAN := $(if ${debug}, -g -fsanitize=undefined -fsanitize=address,                  )
OPT := $(if ${debug}, -Og                                       , -O3 -march=native)
LTO := $(if ${debug},                                           , -flto            )
STD := -std=c++23
WRN := -Wall -Wextra -pedantic
#consider -Wstrict-overflow=5 but it does not point into actual code
COMPILE_OPTIONS := ${SAN} ${OPT} ${STD} ${WRN}
LINK_OPTIONS    := ${SAN} ${OPT} ${STD} ${WRN} ${LTO}

default:
	@echo -e no option specified

clean:
	@rm -frv out/*.o 

refresh:
	@touch src/*.cpp src/*.h

clear:
	@clear

init:
	@echo -e ${CC}creating directories${CD}
	@mkdir -p out
	
fullclean:
	@echo -e ${CC}removing all artifacts${CD}
	@rm -fr out cosi

simulator cosi: $(if ${noclear}, ,clear) link
remake: clean refresh cosi

link: ${FILES}
	@echo -e ${CR}linking${CD}
	@g++ ${LINK_OPTIONS} ${FILES} -o cosi 

${FILES_LOCAL}: out/%.o: src/%.cpp
	@echo -e ${CY}compiling $^${CD}
	@g++ ${COMPILE_OPTIONS} -c $^ -o $@

${FILES_UTIL}: ../utility/out/%.o: ../utility/src/%.cpp
	@$(MAKE) -C ./../utility/
