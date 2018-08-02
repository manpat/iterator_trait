
OPT ?= -O3
FLAGS = -std=c++14 -Wall -Wextra -I. -pedantic

.PHONY: build asm

build: main.cpp $(wildcard *.h)
	@g++ main.cpp $(FLAGS) $(OPT) -obuild

asm: main.cpp $(wildcard *.h)
	@g++ -S main.cpp  $(FLAGS) $(OPT) -obuild.s -masm=intel

run: build
	@./build