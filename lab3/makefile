all : compile run clear

utils.c:utils.h

compile:
	gcc -std=c99 -o simulator utils.c lc3bsim3.c

run:
	./simulator ucode output.obj

clear:
	rm -rf simulator *.swp core dumpsim
