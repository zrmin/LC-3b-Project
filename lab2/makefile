all : compile run clear

utils.c : utils.h

compile:
	gcc -std=c99 -o simulate lc3bsim2.c utils.c

run:
	./simulate output.obj

clear:
	rm -rf simulate core dumpsim
