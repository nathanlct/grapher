
CC := g++
CFLAGS := -c -std=c++17 -O3 --static

LD := g++
LDFLAGS := -Wall -std=c++17 -I/usr/local/lib/ -I/usr/lib/
LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRCDIR  := src
INCLDIR := include
OBJDIR  := obj
BINDIR  := bin

SRCFILES := ${shell find ${SRCDIR} | grep .cpp}
OBJECTS  := ${patsubst ${SRCDIR}/%.cpp,${OBJDIR}/%.o,${SRCFILES}}

TARGET := grapher

.PHONY: all
all : build

${OBJDIR}/%.o : ${SRCDIR}/%.cpp
	@printf "\tBuilding %-30s" $@
	@mkdir -p ${dir $@}
	@${CC} ${CFLAGS} $< -MM -I./${INCLDIR} -MT $@ > ${patsubst %.o,%.d,$@}
	@${CC} -I./${INCLDIR} ${CFLAGS} -c $< -o $@
	@printf "done\n"

-include ${patsubst %.o,%.d,${OBJECTS}}

${BINDIR}/${TARGET} : ${OBJECTS}
	@echo "\tBuilding executable at $@"
	@mkdir -p ${dir $@}
	@${LD} $^ ${LDFLAGS} -o $@ ${LIBS}

.PHONY: build run clean

build: ${BINDIR}/${TARGET}

run: build
	@echo "\tRunning application..."
	@${BINDIR}/${TARGET}

rerun: clean run
rebuild: clean build

clean:
	@echo "\tRemoving objects and binary files"
	@rm -rf obj
	@rm -rf bin
