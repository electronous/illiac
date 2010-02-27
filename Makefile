CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra

all: proto

proto: proto.c

clean:
	rm -f proto *.o
