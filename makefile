.PHONY:code clean
BUILD_DIR := ./build
CFLAGS := -Wall -O2 -g -I./include
CFILES := $(shell find ./src -name "*.c" ! -name "syntax.tab.c")
OBJS := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(CFILES)))))
OBJS += build/syntax.tab.o
all: syntax $(OBJS) 
	gcc-7 $(OBJS) -lfl -ly -o $(BUILD_DIR)/main

syntax: src/syntax.y src/lexical.l
	@mkdir -p build
	bison -d -v -t src/syntax.y -o build/syntax.tab.c
	flex -o build/lex.yy.c src/lexical.l
	gcc -c build/syntax.tab.c $(CFLAGS) -o build/syntax.tab.o

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	gcc -c $(CFLAGS) $< -o $@


	
run: all
	build/main test.cmm

clean:
	rm -r build
	
code:
	cp src/* include/* Code

