####### Compiler, tools and options
#CC            = gcc
#CXX           = g++
DEFINES       = -Wall -std=c++11  -g -O0 -p
#CFLAGS        = $(DEFINES)
#CXXFLAGS        = $(DEFINES)
INCPATH       = -Iinclude  -I/home/feng/include  -I/Users/feng/include
LINK          = $(CXX)
#LFLAGS        = -pg
DEL_FILE      = rm -f
DEL_DIR       = rmdir
MOVE          = mv -f
MAKE_DIR      = mkdir

####### Output directory
OBJECTS_DIR   = ./obj
BIN_DIR       = ./bin

all: test1 test_ge test_lu test_svd test_magic test_llsf test_llsf2 test_svd nllsf_test test_inverse test_jacobi test_sparse_multiply test_power_iteration test_norm test_expm test_rand test_bicgstab householder eigen_symmetric eigen_hermitian test_anti_diag test_diag test_blkdiag test_cgs test_bs test_lus test_import test_data

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

test_sparse_multiply: test/test_sparse_multiply.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_sparse_multiply.o test/test_sparse_multiply.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_sparse_multiply $(OBJECTS_DIR)/test_sparse_multiply.o

test_power_iteration: test/test_power_iteration.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_power_iteration.o test/test_power_iteration.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_power_iteration $(OBJECTS_DIR)/test_power_iteration.o

test_norm: test/test_norm.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_norm.o test/test_norm.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_norm $(OBJECTS_DIR)/test_norm.o

test_expm: test/test_expm.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_expm.o test/test_expm.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_expm $(OBJECTS_DIR)/test_expm.o

test_rand: test/test_rand.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_rand.o test/test_rand.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_rand $(OBJECTS_DIR)/test_rand.o

test_bicgstab: test/test_bicgstab.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_bicgstab.o test/test_bicgstab.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_bicgstab $(OBJECTS_DIR)/test_bicgstab.o

test_cgs: test/test_cgs.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_cgs.o test/test_cgs.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_cgs $(OBJECTS_DIR)/test_cgs.o

complex_jacobi: test/complex_jacobi.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/complex_jacobi.o test/complex_jacobi.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/complex_jacobi $(OBJECTS_DIR)/complex_jacobi.o

householder: test/householder.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/householder.o test/householder.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/householder $(OBJECTS_DIR)/householder.o

eigen_symmetric: test/eigen_symmetric.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/eigen_symmetric.o test/eigen_symmetric.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/eigen_symmetric $(OBJECTS_DIR)/eigen_symmetric.o

eigen_hermitian: test/eigen_hermitian.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/eigen_hermitian.o test/eigen_hermitian.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/eigen_hermitian $(OBJECTS_DIR)/eigen_hermitian.o

test_anti_diag: test/test_anti_diag.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_anti_diag.o test/test_anti_diag.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_anti_diag $(OBJECTS_DIR)/test_anti_diag.o

test_diag: test/test_diag.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_diag.o test/test_diag.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_diag $(OBJECTS_DIR)/test_diag.o

test_blkdiag: test/test_blkdiag.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_blkdiag.o test/test_blkdiag.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_blkdiag $(OBJECTS_DIR)/test_blkdiag.o

test_bs: test/test_bs.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_bs.o test/test_bs.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_bs $(OBJECTS_DIR)/test_bs.o

test_lus: test/test_lus.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_lus.o test/test_lus.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_lus $(OBJECTS_DIR)/test_lus.o

test_import: test/test_import.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_import.o test/test_import.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_import $(OBJECTS_DIR)/test_import.o

test_data: test/test_data.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/test_data.o test/test_data.cc
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/test_data $(OBJECTS_DIR)/test_data.o

