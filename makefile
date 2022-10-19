.PHONY:code clean
all:lexical.l main.c syntax.y semantic.c
	bison -d -v syntax.y
	flex lexical.l
	gcc-7 main.c syntax.tab.c semantic.c -lfl -ly -g -o main

run:all
	./main test.cmm

debug:lexical.l main.c syntax.y
	bison -d -v -t syntax.y
	flex lexical.l
	gcc-7 main.c syntax.tab.c -lfl -ly -o main
	./main test.cmm >temp.txt 2>&1

clean:
	rm syntax.tab.h syntax.tab.c lex.yy.c main

code:
	cp syntax.y lexical.l main.c semantic.c semantic.h tree.h ./Code

