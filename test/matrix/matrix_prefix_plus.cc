#include <f/matrix/matrix.hpp>

#include <algorithm>
#include <iostream>

int main()
{
    f::matrix<long> A( 3, 2 );

    std::fill( A.begin(), A.end(), -1 );

    std::cout << "Matrix A is:\n" << A << "\n";
    std::cout << "Matrix +A is:\n" << +A << "\n";

    return 0;
}

/*
Output:

Matrix A is:
-1       -1
-1       -1
-1       -1

Matrix +A is:
1        1
1        1
1        1 

*/   
