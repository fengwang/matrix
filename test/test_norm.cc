#include <iostream>
#include <algorithm>
#include <matrix.hpp>
#include <vg.hpp> //variate generator

int main()
{
    using namespace feng;
    using namespace std;
    using namespace vg;

    const unsigned long n = 3;
    const unsigned long m = 3;
    matrix<double> A( n, m );

    variate_generator<double> v(0.0, 1.0);
    copy( v.begin(), v.begin()+n*m, A.begin() );

    std::cout << "\nA= \n" << A;

    std::cout << "\nnorm inf of A is " << norm(A);
    std::cout << "\nnorm 1 of A is " << norm(A, 1);
    std::cout << "\nnorm 2 of A is " << norm(A, 2);

    return 0;
}

