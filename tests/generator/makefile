CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CY := '\033[0;33m' #color yellow 

FILES_UTIL  := ../../utility/out/log.o ../../utility/out/file.o
FILES := gen.cpp ${FILES_UTIL}

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
	@rm -frv *.o 

refresh:
	@touch *.cpp 

clear:
	@clear

gen: clear link
	@true # I dont know why but without dummy some bulshit is invoked

remake: clean refresh gen

link: ${FILES}
	@echo -e ${CR}linking${CD}
	@g++ ${LINK_OPTIONS} ${FILES} -o gen 

${FILES_UTIL}: ../../utility/out/%.o: ../../utility/src/%.cpp
	@$(MAKE) -C ../../utility/
