all: compile run clear

compile:
	gcc -o assemble assembler.c -std=c99

run:
	./assemble test.asm output.obj

clear:
	rm -rf assemble output.obj core
