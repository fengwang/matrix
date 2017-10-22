#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/lu_solver.hpp>

#include <iostream>

int main()
{
    const unsigned int n = 15;

    f::matrix<long double>  A = f::rand<long double>(n, n);
    f::matrix<long double>  B = f::rand<long double>(n, 1);
    f::matrix<long double>  b = B;
    f::matrix<long double>  C = A*B;

    if ( 0 == lu_solver( A, B, C ) )
        std::cout << "Successfully solved the linear equations.\nB-B' = \n" << b - B << "\n"; 
    else
        std::cout << "Failed to solve the equaiton.\n";

    return 0;
}

