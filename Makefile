CXX           = g++-7
CXXFLAGS      = -std=c++17 -Wall -pg -O0 -DDEBUG -g3
LINK          = $(CXX)
LFLAGS        = -O0 -pg -g3

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

