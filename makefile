all:lexical.l main.c syntax.y
	flex lexical.l
	bison -d -v syntax.y
	gcc-7 main.c syntax.tab.c -lfl -ly -o main

run:all
	./main test.cmm

debug:lexical.l main.c syntax.y
	bison -d -v -t syntax.y
	flex lexical.l
	gcc-7 main.c syntax.tab.c -lfl -ly -o main
	./main test.cmm >temp.txt 2>&1