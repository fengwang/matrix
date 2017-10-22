#include <f/matrix/matrix.hpp>

#include <algorithm>
#include <iostream>

int main()
{
    f::matrix<double> A( 3, 2 );
    double const b = 2.0;

    std::fill( A.begin(), A.end(), 0.375 );

    std::cout << "Matrix A is:\n" << A << "\n";

    std::cout << "Variable b is: \n" << b << "\n\n";

    std::cout << "A*b is:\n" << A * b << "\n";
    std::cout << "b*A is:\n" << b * A << "\n";
    std::cout << "b*A*b is:\n" << b * A * b << "\n";

    return 0;
}

/*
Output:

*/   
