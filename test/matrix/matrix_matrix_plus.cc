#include <f/matrix/matrix.hpp>

#include <algorithm>
#include <iostream>

int main()
{
    f::matrix<double> A( 3, 2 );
    f::matrix<double> B( 3, 2 );

    std::fill( A.begin(), A.end(), -1 );
    std::fill( B.diag_begin(), B.diag_end(), 7 );

    std::cout << "Matrix A is:\n" << A << "\n";
    std::cout << "Matrix B is:\n" << B << "\n";
    std::cout << "A+B is:\n" << A + B << "\n";
    std::cout << "A+B+A is:\n" << A + B + A << "\n";

    return 0;
}

/*
Output:

Matrix A is:
-1       -1
-1       -1
-1       -1

Matrix B is:
7        0
0        7
0        0

A+B is:
6        -1
-1       6
-1       -1

A+B+A is:
5        -2
-2       5
-2       -2  

*/   
