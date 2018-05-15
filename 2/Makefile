# Makefile for ush
#
# Vincent W. Freeh
# 

CC=gcc
CFLAGS=-g -Wall
SRC=main.c parse.c parse.h engine.c engine.h builtins.c builtins.h signal_handlers.c signal_handlers.h
OBJ=main.o parse.o engine.o builtins.o signal_handlers.o


all: ush

ush:	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

tar:
	tar czvf ush.tar.gz $(SRC) Makefile README

clean:
	\rm $(OBJ) ush pipe

ush.1.ps:	ush.1
	groff -man -T ps ush.1 > ush.1.ps
