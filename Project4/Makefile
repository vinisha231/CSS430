CC = gcc
CFLAGS = -Wall
all: memory
memory: main.o memory.o
	$(CC) $(CFLAGS) -o memory main.o memory.o
main.o: main.c memory.h
	$(CC) $(CFLAGS) -c main.c
memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c
clean:
	rm -f *.o memory