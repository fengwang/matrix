OPENCV := 0
ifeq ($(OPENCV), 1)
	OPENCVOP = -DFENG_MATRIX_OPENCV `pkg-config --cflags opencv4` -Wno-deprecated-enum-enum-conversion
	OPENCVLOP = `pkg-config --libs opencv4`
else
	OPENCVOP =
	OPENCVLOP =
endif

# Portable -O2 by default; FAST=1 restores the old -Ofast -march=native set (D-006).
FAST          ?= 0
OP            = -DFENG_MATRIX_PARALLEL
CXX           = g++
ifeq ($(FAST), 1)
CXXFLAGS      = -std=c++20 -Wall -Wextra -fmax-errors=1 -Ofast -flto=auto  -funroll-all-loops -pipe -march=native $(OP) -isystem tests -pthread $(OPENCVOP)
LFLAGS        = -Ofast $(OPENCVLOP)  -pthread -lstdc++fs -Wl,--gc-sections -flto=auto
else
CXXFLAGS      = -std=c++20 -O2 -Wall -Wextra $(OP) -isystem tests -pthread $(OPENCVOP)
LFLAGS        = -O2 $(OPENCVLOP) -pthread
endif

LINK          = $(CXX)

####### Output directory
BUILD_DIR     ?= build
OBJECTS_DIR   = $(BUILD_DIR)
BIN_DIR       = $(BUILD_DIR)

all: test example

test: $(BIN_DIR)/test_test
example: $(BIN_DIR)/test_example

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/test_test: tests/test.cc ./matrix.hpp $(wildcard tests/cases/*.hpp) | $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o $(OBJECTS_DIR)/test.o tests/test.cc
	$(LINK) -o $(BIN_DIR)/test_test $(OBJECTS_DIR)/test.o $(LFLAGS)

$(BIN_DIR)/test_example: examples/example.cc ./matrix.hpp $(wildcard examples/cases/*.hpp) | $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o $(OBJECTS_DIR)/example.o examples/example.cc
	$(LINK) -o $(BIN_DIR)/test_example $(OBJECTS_DIR)/example.o $(LFLAGS)

.PHONY: all test example clean clean_obj clean_test clean_example
clean: clean_obj clean_test clean_example
clean_obj:
	rm -f $(OBJECTS_DIR)/test.o $(OBJECTS_DIR)/example.o
clean_test:
	rm -f $(BIN_DIR)/test_test
clean_example:
	rm -f $(BIN_DIR)/test_example
