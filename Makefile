CXX           = clang++
CXXFLAGS      = -std=c++17 -stdlib=libc++ -O2 -ferror-limit=2 -Weverything
GXXFLAGS      = -mtune=corei7 -march=corei7 -std=c++11 -O3 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -msse4 -pipe -Wall -Wextra -fPIC -fmax-errors=2
LINK          = $(CXX)
LFLAGS        = -lc++ -lc++abi -O2

####### Output directory
OBJECTS_DIR   = .
BIN_DIR       = .
LIB_DIR       = .
LOG_DIR       = .

all: test example

test: tests/test.cc ./matrix.hpp
	$(CXX) -c $(CXXFLAGS) -o $(OBJECTS_DIR)/test.o tests/test.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test $(OBJECTS_DIR)/test.o

example: examples/example.cc ./matrix.hpp
	$(CXX) -c $(CXXFLAGS) -o $(OBJECTS_DIR)/example.o examples/example.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/example $(OBJECTS_DIR)/example.o

.PHONY: clean clean_obj clean_test clean_example
clean: clean_obj clean_test clean_example
clean_obj:
	rm ./*.o
clean_test:
	rm ./test
clean_example:
	rm ./example

