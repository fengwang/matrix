CXX           = clang++
CXXFLAGS      = -std=c++17 -stdlib=libc++ -O3 -ferror-limit=2 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-sign-conversion -Wno-exit-time-destructors -Wno-float-equal -Wno-global-constructors -Wno-missing-declarations -Wno-unused-parameter -Wno-padded -Wno-shadow -Wno-weak-vtables -Wno-missing-prototypes -Wno-unused-variable -ferror-limit=1 -Wno-deprecated -Wno-conversion -Wno-double-promotion
GXXFLAGS      = -mtune=corei7 -march=corei7 -std=c++11 -O3 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -msse4 -pipe -Wall -Wextra -fPIC -fmax-errors=1
INCPATH       = -Iinclude
LINK          = $(CXX)
LFLAGS        = -lc++ -lc++abi -O3 -pthread
GLFLAGS       =
DEL_FILE      = rm -f
DEL_DIR       = rmdir
MOVE          = mv -f
MAKE_DIR      = mkdir

####### Output directory
OBJECTS_DIR   = ./obj
BIN_DIR       = ./bin
LIB_DIR       = ./lib
LOG_DIR       = ./log

laplace_1d: test/matrix/laplace_1d.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/laplace_1d.o test/matrix/laplace_1d.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/laplace_1d $(OBJECTS_DIR)/laplace_1d.o

matrix_inverse: test/matrix/matrix_inverse.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_inverse.o test/matrix/matrix_inverse.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_inverse $(OBJECTS_DIR)/matrix_inverse.o

matrix_value_plus: test/matrix/matrix_value_plus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_value_plus.o test/matrix/matrix_value_plus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_value_plus $(OBJECTS_DIR)/matrix_value_plus.o

matrix_value_minus: test/matrix/matrix_value_minus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_value_minus.o test/matrix/matrix_value_minus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_value_minus $(OBJECTS_DIR)/matrix_value_minus.o

matrix_value_divide: test/matrix/matrix_value_divide.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_value_divide.o test/matrix/matrix_value_divide.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_value_divide $(OBJECTS_DIR)/matrix_value_divide.o

value_matrix_minus: test/matrix/value_matrix_minus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/value_matrix_minus.o test/matrix/value_matrix_minus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/value_matrix_minus $(OBJECTS_DIR)/value_matrix_minus.o

matrix_load: test/matrix/matrix_load.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_load.o test/matrix/matrix_load.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_load $(OBJECTS_DIR)/matrix_load.o

matrix_value_multiply: test/matrix/matrix_value_multiply.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_value_multiply.o test/matrix/matrix_value_multiply.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_value_multiply $(OBJECTS_DIR)/matrix_value_multiply.o

matrix_matrix_plus: test/matrix/matrix_matrix_plus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_matrix_plus.o test/matrix/matrix_matrix_plus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_matrix_plus $(OBJECTS_DIR)/matrix_matrix_plus.o

matrix_matrix_minus: test/matrix/matrix_matrix_minus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_matrix_minus.o test/matrix/matrix_matrix_minus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_matrix_minus $(OBJECTS_DIR)/matrix_matrix_minus.o

matrix_prefix_minus: test/matrix/matrix_prefix_minus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_prefix_minus.o test/matrix/matrix_prefix_minus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_prefix_minus $(OBJECTS_DIR)/matrix_prefix_minus.o

matrix_prefix_plus: test/matrix/matrix_prefix_plus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_prefix_plus.o test/matrix/matrix_prefix_plus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_prefix_plus $(OBJECTS_DIR)/matrix_prefix_plus.o

matrix_construct: test/matrix/matrix_construct.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_construct.o test/matrix/matrix_construct.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_construct $(OBJECTS_DIR)/matrix_construct.o

matrix_llsf: test/matrix/matrix_llsf.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_llsf.o test/matrix/matrix_llsf.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_llsf $(OBJECTS_DIR)/matrix_llsf.o

cc_solve: test/matrix/solve.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/solve.o test/matrix/solve.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/cc_solve $(OBJECTS_DIR)/solve.o

matrix_nlsf: test/matrix/matrix_nlsf.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_nlsf.o test/matrix/matrix_nlsf.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_nlsf $(OBJECTS_DIR)/matrix_nlsf.o

exp_nlsf: test/matrix/exp_nlsf.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/exp_nlsf.o test/matrix/exp_nlsf.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/exp_nlsf $(OBJECTS_DIR)/exp_nlsf.o

matrix_backward_substitution: test/matrix/matrix_backward_substitution.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_backward_substitution.o test/matrix/matrix_backward_substitution.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_backward_substitution $(OBJECTS_DIR)/matrix_backward_substitution.o

matrix_expm: test/matrix/matrix_expm.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_expm.o test/matrix/matrix_expm.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_expm $(OBJECTS_DIR)/matrix_expm.o

ir_expm: test/matrix/ir_expm.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/ir_expm.o test/matrix/ir_expm.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/ir_expm $(OBJECTS_DIR)/ir_expm.o

matrix_abs: test/matrix/matrix_abs.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_abs.o test/matrix/matrix_abs.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_abs $(OBJECTS_DIR)/matrix_abs.o

matrix_range_for: test/matrix/matrix_range_for.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_range_for.o test/matrix/matrix_range_for.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_range_for $(OBJECTS_DIR)/matrix_range_for.o

matrix_reshape: test/matrix/matrix_reshape.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_reshape.o test/matrix/matrix_reshape.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_reshape $(OBJECTS_DIR)/matrix_reshape.o

matrix_matrix_multiplication: test/matrix/matrix_matrix_multiplication.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_matrix_multiplication.o test/matrix/matrix_matrix_multiplication.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_matrix_multiplication $(OBJECTS_DIR)/matrix_matrix_multiplication.o

matrix_matrix_swap: test/matrix/matrix_matrix_swap.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_matrix_swap.o test/matrix/matrix_matrix_swap.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_matrix_swap $(OBJECTS_DIR)/matrix_matrix_swap.o

matrix_lu: test/matrix/matrix_lu.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_lu.o test/matrix/matrix_lu.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_lu $(OBJECTS_DIR)/matrix_lu.o

matrix_svd: test/matrix/matrix_svd.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_svd.o test/matrix/matrix_svd.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_svd $(OBJECTS_DIR)/matrix_svd.o

matrix_clear: test/matrix/matrix_clear.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_clear.o test/matrix/matrix_clear.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_clear $(OBJECTS_DIR)/matrix_clear.o

matrix_xor: test/matrix/matrix_xor.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_xor.o test/matrix/matrix_xor.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_xor $(OBJECTS_DIR)/matrix_xor.o

matrix_shrink_to_size: test/matrix/matrix_shrink_to_size.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/matrix_shrink_to_size.o test/matrix/matrix_shrink_to_size.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/matrix_shrink_to_size $(OBJECTS_DIR)/matrix_shrink_to_size.o

