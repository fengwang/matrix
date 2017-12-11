CXX           = clang++
CXXFLAGS      = -std=c++17 -stdlib=libc++ -O2 -ferror-limit=2 -Weverything
GXXFLAGS      = -mtune=corei7 -march=corei7 -std=c++11 -O3 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -msse4 -pipe -Wall -Wextra -fPIC -fmax-errors=2
LINK          = $(CXX)
LFLAGS        = -lc++ -lc++abi -O3 -pthread

####### Output directory
OBJECTS_DIR   = .
BIN_DIR       = .
LIB_DIR       = .
LOG_DIR       = .

test: tests/test.cc ./matrix.hpp
	$(CXX) -c $(CXXFLAGS) -o $(OBJECTS_DIR)/test.o tests/test.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test $(OBJECTS_DIR)/test.o

