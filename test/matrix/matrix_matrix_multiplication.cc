#include <f/matrix/matrix.hpp>

#include <iostream>

int main()
{
    const unsigned int n = 10;

    f::matrix<long double> const& A = f::rand<long double>(n, n);
    f::matrix<long double> const& B = f::rand<long double>(n, n);

    std::cout << "A*B/A-B = \n" << A * B / A - B << "\n";
    std::cout << "A*B/B-A = \n" << A * B / B - A << "\n";
    std::cout << "B*A/B-A = \n" << B * A / B - A << "\n";
    std::cout << "B*A/A-B = \n" << B * A / A - B << "\n";

    return 0;
}

