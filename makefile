all:lexical.l main.c
	bison -d syntax.y
	flex lexical.l
	gcc-7 main.c syntax.tab.c -lfl -ly -o main
	#gcc-7 main.c lex.yy.c -lfl -ly -o main

run:all
	./main test.cmm