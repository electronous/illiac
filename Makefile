CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra

all: proto

proto: proto.c proto.h

clean:
	rm -f proto *.o
