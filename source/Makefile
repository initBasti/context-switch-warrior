CC=gcc
CFLAGS=-Wall -Wextra -pedantic -D_POSIX_SOURCE

all: csw clean

get-time.o: get-time.c include/get-time.h
	$(CC) -c -o $@ get-time.c  $(CFLAGS)

config.o: config.c include/config.h
	$(CC) -c -o $@ config.c $(CFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

csw: main.o get-time.o config.o
	$(CC) -o $@ main.o get-time.o config.o

clean:
	rm *.o
