CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic

all: avl

avl: main.o avl.o
	$(CC) main.o avl.o -o avl

main.o: main.c avl.h
	$(CC) $(CFLAGS) -c main.c

avl.o: avl.c avl.h
	$(CC) $(CFLAGS) -c avl.c

clean:
	rm -f *.o avl
