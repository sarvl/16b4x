CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CY := '\033[0;33m' #color yellow 

FILES_LOCAL := out/main.o
FILES_UTIL  := ../utility/out/log.o ../utility/out/file.o
FILES := ${FILES_LOCAL} ${FILES_UTIL}

debug :=
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

simulator cosi: clear link
remake: clean refresh cosi

link: ${FILES}
	@echo -e ${CR}linking${CD}
	@g++ ${LINK_OPTIONS} ${FILES} -o cosi 

${FILES_LOCAL}: out/%.o: src/%.cpp
	@echo -e ${CY}compiling $^${CD}
	@g++ ${COMPILE_OPTIONS} -c $^ -o $@

${FILES_UTIL}: ../utility/out/%.o: ../utility/src/%.cpp
	@$(MAKE) -C ./../utility/
