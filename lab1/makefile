all: compile run clear

utils.c: utils.h lc3bassemblerdefines.h

compile:
	gcc -o assemble assembler.c utils.c -std=c99

run:
	./assemble test.asm output.obj

clear:
	rm -rf assemble output.obj core
