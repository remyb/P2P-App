#########################################################################
#
# NAME:         Remy Baumgarten & Chris Uyehara
#
# PROJECT:      2
#
# CLASS:        ICS 451
#
# DATE:	        October 15th, 2010
#
# FILE:	        makefile
#
# DESCRIPTION:  This file contains the makefile targets for Project 2
#               Run make to compile the program.
#
#########################################################################

# Compiler
CC=gcc

# Compiler flags
CFLAGS=-c -g -Wall

# Make targets
all: p2p

p2p: p2p.o libstring.o
	$(CC) p2p.o libstring.o -o p2p -lpthread

p2p.o: p2p.c p2p.h libstring.h
	$(CC) $(CFLAGS) p2p.c

libstring.o: libstring.c libstring.h
	$(CC) $(CFLAGS) libstring.c

clean:
	rm -rf *.o p2p

test: libstring.o test.o
	$(CC) test.o libstring.o -o test

test.o: test.c
	$(CC) $(CFLAGS) test.c