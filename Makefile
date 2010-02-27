CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g

all: proto

proto: proto.c proto.h

clean:
	rm -f proto *.o
