#include <vg.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>
#include <valarray>

int main()
{
    using namespace feng;

    unsigned int const n = 100;
    vg::variate_generator<double> v;
    matrix<double> A(n, n);
    matrix<double> V(n, n);
    matrix<double> Q(n, n);
    matrix<double> D(n, n);
    std::valarray<double> Lambda(n);
    matrix<double> L(n, n);
    std::copy( v.begin(), v.begin()+n*n, A.begin() );

    //for ( unsigned int i = 2; i != n; ++i )
    //    std::fill( A.upper_diag_begin(i), A.upper_diag_end(i), double(0) );

    for ( unsigned int i = 1; i != n; ++i )
        std::copy( A.upper_diag_begin(i), A.upper_diag_end(i), A.lower_diag_begin(i) );


    householder( A, Q, D );
    std::cout << "iteration is " << eigen_jacobi( D, V, std::begin(Lambda) );

    V = Q*V;

    //std::cout << "iterations is " << feng::tridiagonal_cyclic_eigen_jacobi( A, V, std::begin(Lambda) );
    //std::cout << "iterations is " << feng::cyclic_eigen_jacobi( A, V, std::begin(Lambda) );
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



