CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CC := '\033[0;36m' #color cyan
CY := '\033[0;33m' #color yellow 

FILES_GEN     := out/gen/sim.o out/gen/sasm.o
FILES_TEST    := out/test/sim.o out/test/sasm.o
FILES_LOCAL   := out/main.o ${FILES_GEN} ${FILES_TEST} 
FILES_UTIL    := ../utility/out/log.o
FILES := ${FILES_LOCAL} ${FILES_UTIL}

debug :=
noclear :=
SAN := $(if ${debug}, -g -fsanitize=undefined -fsanitize=address,                  )
OPT := $(if ${debug}, -Og                                       , -O3 -march=native)
LTO := $(if ${debug},                                           , -flto            )
STD := -std=c++20
WRN := -Wall -Wextra -pedantic
COMPILE_OPTIONS := ${SAN} ${OPT} ${STD} ${WRN}
LINK_OPTIONS    := ${SAN} ${OPT} ${STD} ${WRN} ${LTO}

default:
	@echo -e no option specified

clean:
	@rm -fv out/*.o out/*/*.0

refresh:
	@touch src/*.cpp
	@touch src/*/*.cpp

clear:
	@clear

init:
	@echo -e ${CC}creating directories${CD}
	@mkdir -p out/test out/gen
	
fullclean:
	@echo -e ${CC}removing all artifacts${CD}
	@rm -fr out fost

autotest fost: $(if {noclear}, , clear) link
remake: clean refresh fost

link: ${FILES}
	@echo -e ${CR}linking${CD}
	@g++ ${LINK_OPTIONS} ${FILES} -o fost 

${FILES_UTIL}: ../utility/out/%.o: ../utility/src/%.cpp
	@$(MAKE) -C ../utility/

${FILES_LOCAL}: out/%.o: src/%.cpp
	@echo -e ${CY}compiling $^${CD}
	@g++ ${COMPILE_OPTIONS} -c $^ -o $@
