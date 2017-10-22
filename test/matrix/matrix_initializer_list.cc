#include <f/matrix/matrix.hpp>

#include <iostream>

int main()
{
    f::matrix<double> A( 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } );

    std::cout << "A=\n";
    std::cout << A << "\n";

    A = { 2,3,4,5,6,7,8,9,10 };
    std::cout << "A=\n";
    std::cout << A << "\n";

    return 0;
}

