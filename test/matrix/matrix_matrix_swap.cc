#include <f/matrix/matrix.hpp>

#include <iostream>

int main()
{
    const unsigned int n = 5;

    f::matrix<long double>  A = f::rand<long double>(n, n);
    f::matrix<long double>  B = f::rand<long double>(n, n-3);

    std::cout << "A = \n" << A << "\n";
    std::cout << "B = \n" << B << "\n";

    A.swap(B);

    std::cout << "\nafter swap\n";
    std::cout << "A = \n" << A << "\n";
    std::cout << "B = \n" << B << "\n";

    return 0;
}

