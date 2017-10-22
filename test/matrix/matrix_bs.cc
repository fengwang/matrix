#include <iostream>
#include <algorithm>
#include <numeric>
#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>

int main()
{
    using namespace f;

    unsigned int N = 10;

    variate_generator<double> v;

    matrix<double> A(N,N);
    matrix<double> x(N,1);
    matrix<double> x_(N,1);

    std::copy( v.begin(), v.begin()+N*N, A.begin() );
    std::copy( v.begin(), v.begin()+N, x.begin() );

    std::cout  << "\nA=\n" << A;
    std::cout  << "\nx=\n" << x;


    std::cout << "\ncalculating A*x ......" << std::endl;
    matrix<double> b = A * x;

    std::cout << "\ntrying to solve Ax = b ......" << std::endl;

    assert( 0 == lu_solver( A, x_, b ) );

    std:: cout << "\nAx-b=\n" << A * x_ - b;
    std:: cout << "\nx-x_=\n" << x - x_;

    for ( auto const& x : A )
        std::cout << x << "\t";

    return 0;
}

