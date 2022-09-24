all:lexical.l main.c
	flex lexical.l
	gcc-7 main.c lex.yy.c -lfl -o main

run:all
	./main test.cmm