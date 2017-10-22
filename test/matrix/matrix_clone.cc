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

    matrix<double> B{ A, 0, 7, 3, 4 };
    std::cout << "B(0,7,3,4)=\n" << B << "\n";

    matrix<double> C{ A, 2, 5, 1, 6 };
    std::cout << "C(2,5,1,6)=\n" << C << "\n";


    std::cout << "\ndet(A) = " << A.det() << "\n";

    return 0;
}

