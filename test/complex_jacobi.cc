#include <vg.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>
#include <valarray>
#include <complex>

int main()
{
    using namespace feng;
    typedef std::complex<double> doublecomplex;

    unsigned int const n = 35;
    vg::variate_generator<double> v;
    matrix<doublecomplex> A(n, n);
    matrix<doublecomplex> V(n, n);
    std::valarray<doublecomplex> Lambda(n);
    matrix<doublecomplex> L(n, n);
    //std::copy( v.begin(), v.begin()+n*n, A.begin() );

    std::for_each( A.begin(), A.end(), [&v](doublecomplex& c) { c.real(v()); c.imag(v()); } );
#if 0
    A[0][0]= 2;
    A[0][1]=-1;
    A[0][2]= 5;
    A[1][1]= 2;
    A[1][2]=-1;
    A[2][2]= 2;
#endif

    for ( unsigned int i = 1; i != n; ++i )
        std::copy( A.upper_diag_begin(i), A.upper_diag_end(i), A.lower_diag_begin(i) );

    std::cout << "iterations is " << feng::cyclic_eigen_jacobi( A, V, std::begin(Lambda) );
    //std::cout << "iterations is " << feng::eigen_jacobi( A, V, std::begin(Lambda) );

    std::cout << "\nV V^T = \n";
    std::cout << V.transpose() * V << std::endl;

    std::copy( std::begin(Lambda), std::end(Lambda), L.diag_begin() );

    std::cout << "\nA - V L V^T = \n";
    std::cout << A - (V * L * (V.inverse())) << std::endl;


    std::cout << "\nA=\n" << A << "\n";
    std::cout << "\nV=\n" << V << "\n";
    std::cout << "\nL=\n" << L << "\n";

    return 0;
}



