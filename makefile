.PHONY:code clean
BUILD_DIR := build
CFLAGS := -Wall -g -I./include
CFILES := $(shell find ./src -name "*.c" ! -name "syntax.tab.c")
OBJS := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(CFILES)))))
OBJS += $(BUILD_DIR)/syntax.tab.o
all: syntax $(OBJS)
	gcc-7 $(OBJS) -lfl -ly -o $(BUILD_DIR)/main

syntax: src/syntax.y src/lexical.l
	@mkdir -p $(BUILD_DIR)
	bison -d -v -t src/syntax.y -o $(BUILD_DIR)/syntax.tab.c
	flex -o $(BUILD_DIR)/lex.yy.c src/lexical.l
	gcc-7 -c $(BUILD_DIR)/syntax.tab.c $(CFLAGS) -o $(BUILD_DIR)/syntax.tab.o

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	gcc-7 -c $(CFLAGS) $< -o $@

	
run: all
	build/main test.cmm

clean:
	rm -r build/

#For submit
code:
	cp src/* include/* Code

