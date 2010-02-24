CC=gcc
CFLAGS=-pedantic -ansi -std=c99 -Wall

all: proto

proto: proto.c proto.h

clean:
	rm -f proto *.o
