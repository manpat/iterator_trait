
OPT ?= -O3
FLAGS = -std=c++14 -Wall -Wextra -I. -pedantic

build: main.cpp $(wildcard *.h)
	@g++ main.cpp $(FLAGS) $(OPT) -obuild

asm: main.cpp $(wildcard *.h)
	@g++ -S main.cpp  $(FLAGS) $(OPT) -obuild.s -masm=intel

run: build
	@./build