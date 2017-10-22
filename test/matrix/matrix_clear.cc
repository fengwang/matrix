#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/lu_solver.hpp>

#include <iostream>

int main()
{
    const unsigned int n = 5;

    f::matrix<long double>  A = f::rand<long double>(n, n);
    f::matrix<long double>  B = f::rand<long double>(n, 1);

    std::cout << "the matrix A is \n" << A << "\n";
    std::cout << "the matrix B is \n" << B << "\n";

    A.clear();
    B.clear();

    std::cout << "After clear\n";

    std::cout << "the matrix A is \n" << A << "\n";
    std::cout << "the matrix B is \n" << B << "\n";

    return 0;
}

