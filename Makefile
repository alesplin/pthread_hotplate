# makefile for the pthread implementation of my HotPlate solver
#

COMMON_FLAGS = -Wall -pthread -Iinclude/
RELEASE_FLAGS = -O3
DEBUG_FLAGS = -g

clean:
	-rm -rfv bin/*
	-rm -rfv HotPlate

