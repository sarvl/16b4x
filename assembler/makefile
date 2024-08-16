CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CY := '\033[0;33m' #color yellow 

FILES_CMD     := ./out/command_line/input.o ./out/command_line/string_help.o
FILES_PROCESS := ./out/process/expressions.o ./out/process/tokenize.o ./out/process/verify.o ./out/process/preprocess.o ./out/process/code_gen.o 
FILES_TYPES   := ./out/types/nameval.o ./out/types/token.o
FILES_LOCAL   := ${FILES_CMD} ${FILES_PROCESS} ${FILES_TYPES} out/config.o out/main.o out/save.o out/utils.o out/warning.o
FILES_UTIL    := ../utility/out/log.o ../utility/out/file.o
FILES := ${FILES_LOCAL} ${FILES_UTIL}

debug :=
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
	@rm -frv out/*.o 

refresh:
	@touch src/*.cpp src/*.h
	@touch src/*/*.cpp src/*/*.h

clear:
	@clear

ctags:
	@ctags src/*.cpp src/*.h src/*/*.cpp src/*/*.h

#              misspell
assembler sasm sams: clear link
remake: clean refresh sasm

link: ${FILES}
	@echo -e ${CR}linking${CD}
	@g++ ${LINK_OPTIONS} ${FILES} -o sasm 

${FILES_LOCAL}: out/%.o: src/%.cpp
	@echo -e ${CY}compiling $^${CD}
	@g++ ${COMPILE_OPTIONS} -c $^ -o $@

${FILES_UTIL}: ../utility/out/%.o: ../utility/src/%.cpp
	@$(MAKE) -C ./../utility/
