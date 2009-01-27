# makefile for the pthread implementation of my HotPlate solver
#

# aliases for compiler executable and release/debug flags
CC = gcc-4.2
COMMON_FLAGS = -Wall -pthread -Iinclude/
RELEASE_FLAGS = -O3
DEBUG_FLAGS = -g

# aliases for files
DBG_OBJ = bin/debug/hp.o
REL_OBJ = bin/release/hp.o
EXE = bin/HotPlate

HotPlate: include/HotPlate.h src/HotPlate.c
	$(CC) $(COMMON_FLAGS) $(RELEASE_FLAGS) -c src/HotPlate.c -o bin/hp.o

# utilities to make sure directories exist and clean stuff...

prep:
	if [ -d bin ];then echo "bin ok...";else mkdir bin; echo "created bin...";fi 
	if [ -d bin/debug ];then echo "bin/debug ok...";else mkdir bin/debug; echo "created bin/debug...";fi
	if [ -d bin/release ];then echo "bin/release ok...";else mkdir bin/release; echo "created bin/release...";fi

clean:
	-rm -rfv $(EXE)
	-rm -rfv bin/debug/*
	-rm -rfv bin/release/*

