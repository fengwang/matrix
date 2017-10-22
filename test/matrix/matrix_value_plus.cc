#include <f/matrix/matrix.hpp>

#include <algorithm>
#include <iostream>

int main()
{
    f::matrix<long> A( 3, 2 );
    long const b = 2;

    std::fill( A.begin(), A.end(), -1 );

    std::cout << "Matrix A is:\n" << A << "\n";

    std::cout << "Variable b is: \n" << b << "\n\n";

    std::cout << "A+b is:\n" << A + b << "\n";
    std::cout << "b+A is:\n" << b + A << "\n";
    std::cout << "b+A+b is:\n" << b + A + b << "\n";

    return 0;
}

/*
Output:

Matrix A is:
-1       -1
-1       -1
-1       -1

Variable b is:
2

A+b is:
1        1
1        1
1        1

b+A is:
1        1
1        1
1        1

b+A+b is:
3        3
3        3
3        3  

*/   
