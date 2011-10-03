####### Compiler, tools and options
CC            = gcc
#CC            = gcc-4.6
#CXX           = icpc
#CXX           = g++-4.6
CXX           = g++
#DEFINES       = -Wall
DEFINES       = -Wall -std=c++0x
CFLAGS        = -O2 $(DEFINES)
CXXFLAGS        = -O2 $(DEFINES)
INCPATH       = -Iinclude  -I/home/feng/include
#LINK          = icpc
LINK          = g++
#LINK          = g++-4.6
LFLAGS        = 
#LFLAGS        = -Wl,--as-needed -Wl,-O1
DEL_FILE      = rm -f
DEL_DIR       = rmdir
MOVE          = mv -f
MAKE_DIR      = mkdir

####### Output directory
OBJECTS_DIR   = ./obj
BIN_DIR       = ./bin

all: test1 test_ge

clean: 
	rm -rf $(OBJECTS_DIR)/*
	rm -rf $(BIN_DIR)/*

test1: test/test1.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test1.o test/test1.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test1 $(OBJECTS_DIR)/test1.o

test_ge: test/test_ge.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_ge.o test/test_ge.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_ge $(OBJECTS_DIR)/test_ge.o

