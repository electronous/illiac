CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g

all: proto

proto: proto.o

proto.o: proto.c proto.h

clean:
	rm -f proto *.o
