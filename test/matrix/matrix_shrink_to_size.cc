#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>

int main()
{
    using namespace f;

    unsigned long n = 7;

    variate_generator<double> vg;

    matrix<double> A( n, n );

    std::generate( A.begin(), A.end(), vg );

    std::cout << "A=\n" << A << "\n";

    A.shrink_to_size( n-1, n-3 );

    std::cout << "afer shrinking, A=\n" << A << "\n";

    return 0;
}

