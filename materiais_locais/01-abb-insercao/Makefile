CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -g

all: main

main: main.c abb.c abb.h
	$(CC) $(CFLAGS) main.c abb.c -o main

run: main
	./main

clean:
	rm -f main
