CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic
OBJ = main.o avl.o item.o

all: avl

avl: $(OBJ)
	$(CC) $(OBJ) -o avl

main.o: main.c avl.h item.h
	$(CC) $(CFLAGS) -c main.c

avl.o: avl.c avl.h item.h
	$(CC) $(CFLAGS) -c avl.c

item.o: item.c item.h
	$(CC) $(CFLAGS) -c item.c

clean:
	rm -f *.o avl
