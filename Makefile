COMP=gcc
WARN=-Wall
OPT=-O0
DEB=-g

default: all

all: nibble_swapper.exe

nibble_swapper.exe: nibble_swapper.c
	${COMP} ${WARN} ${OPT} ${DEB} -o $@ $?

PHONY: clean
clean:
	rm ./*.exe
