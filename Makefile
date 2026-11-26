CC      = gcc
CFLAGS  = -std=c17 -Wall -Wextra -g
SRC     = src/main.c src/debugger.c
BIN     = dbg

all: $(BIN) tests/hello

$(BIN): $(SRC) src/debugger.h
	$(CC) $(CFLAGS) $(SRC) -o $@

tests/hello: tests/hello.asm
	nasm -f elf64 $< -o tests/hello.o
	ld tests/hello.o -o $@

clean:
	rm -f $(BIN) tests/*.o tests/hello

.PHONY: all clean
