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
HEADER = include/HotPlate.h
SRC = src/HotPlate.c

$(EXE): $(HEADER) $(SRC)
	$(CC) $(COMMON_FLAGS) $(RELEASE_FLAGS) -c $(SRC) -o $(REL_OBJ)
	$(CC) $(COMMON_FLAGS) $(RELEASE_FLAGS) $(REL_OBJ) -o $(EXE)

run: $(EXE)
	bin/HotPlate

# run with 2 threads
t2: $(EXE)
	bin/HotPlate --nt=2

# run with 4 threads
t4: $(EXE)
	bin/HotPlate --nt=4

# run with 8 threads
t8: $(EXE)
	bin/HotPlate --nt=8

# run with 16 threads
t16: $(EXE)
	bin/HotPlate --nt=16

# run with 32 threads
t32: $(EXE)
	bin/HotPlate --nt=32

debug: $(HEADER) $(SRC) clean
	$(CC) $(COMMON_FLAGS) $(DEBUG_FLAGS) -c $(SRC) -o $(DBG_OBJ)
	$(CC) $(COMMON_FLAGS) $(DEBUG_FLAGS) $(DBG_OBJ) -o $(EXE)

ctags: $(HEADER) $(SRC)
	ctags -t -f tags $(SRC) 

# utilities to make sure directories exist and clean stuff...

prep:
	if [ -d bin ];then echo "bin ok...";else mkdir bin; echo "created bin...";fi 
	if [ -d bin/debug ];then echo "bin/debug ok...";else mkdir bin/debug; echo "created bin/debug...";fi
	if [ -d bin/release ];then echo "bin/release ok...";else mkdir bin/release; echo "created bin/release...";fi

clean:
	-rm -rfv $(EXE)
	-rm -rfv bin/debug/*
	-rm -rfv bin/release/*

