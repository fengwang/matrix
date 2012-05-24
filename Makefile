####### Compiler, tools and options
CC            = gcc
CXX           = g++
DEFINES       = -Wall -std=c++11  -g -O0
CFLAGS        = $(DEFINES)
CXXFLAGS        = $(DEFINES)
INCPATH       = -Iinclude  -I/home/feng/include
LINK          = g++
LFLAGS        = -g
DEL_FILE      = rm -f
DEL_DIR       = rmdir
MOVE          = mv -f
MAKE_DIR      = mkdir

####### Output directory
OBJECTS_DIR   = ./obj
BIN_DIR       = ./bin

all: test1 test_ge test_lu test_svd test_magic test_llsf test_llsf2 test_svd nllsf_test test_inverse test_jacobi

clean: 
	rm -rf $(OBJECTS_DIR)/*
	rm -rf $(BIN_DIR)/*

test1: test/test1.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test1.o test/test1.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test1 $(OBJECTS_DIR)/test1.o

test_ge: test/test_ge.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_ge.o test/test_ge.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_ge $(OBJECTS_DIR)/test_ge.o

test_lu: test/test_lu.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_lu.o test/test_lu.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_lu $(OBJECTS_DIR)/test_lu.o

test_cd: test/test_cd.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_cd.o test/test_cd.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_cd $(OBJECTS_DIR)/test_cd.o

test_magic: test/test_magic.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_magic.o test/test_magic.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_magic $(OBJECTS_DIR)/test_magic.o

test_llsf: test/test_llsf.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_llsf.o test/test_llsf.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_llsf $(OBJECTS_DIR)/test_llsf.o

nllsf_test: test/nllsf_test.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/nllsf_test.o test/nllsf_test.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/nllsf_test $(OBJECTS_DIR)/nllsf_test.o

test_llsf2: test/test_llsf2.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_llsf2.o test/test_llsf2.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_llsf2 $(OBJECTS_DIR)/test_llsf2.o

test_svd: test/test_svd.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_svd.o test/test_svd.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_svd $(OBJECTS_DIR)/test_svd.o

test_inverse: test/test_inverse.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_inverse.o test/test_inverse.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_inverse $(OBJECTS_DIR)/test_inverse.o

test_jacobi: test/test_jacobi.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_jacobi.o test/test_jacobi.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_jacobi $(OBJECTS_DIR)/test_jacobi.o

