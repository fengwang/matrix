#include <vg.hpp>
#include <matrix.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <valarray>
#include <complex>

int main()
{
    using namespace feng;

    unsigned int const n = 40;
    vg::variate_generator<double> v;
    matrix<std::complex<double> > A(n, n);
    matrix<std::complex<double> > V(n, n);
    matrix<double> L(n, n);
    std::for_each( A.begin(), A.end(), [&v](std::complex<double>& c) { c.real(v()); c.imag(v()); } );

    for ( unsigned int i = 1; i != n; ++i )
    {
        feng::for_each( A.upper_diag_begin(i), A.upper_diag_end(i), A.lower_diag_begin(i),
                        []( const std::complex<double>& c1, std::complex<double>& c2 )
                        { c2 = std::conj(c1); } );
    }
    std::for_each( A.diag_begin(), A.diag_end(), [](std::complex<double>&c){ c.imag(0); } );
        //std::copy( A.upper_diag_begin(i), A.upper_diag_end(i), A.lower_diag_begin(i) );

    eigen_hermitian(A, V, L);

    
    std::cout << "\nA - V L V^(-1) = \n";
    std::cout << A - (V * L * (V.inverse())) << std::endl;

    std::cout << "\nA=\n" << A << "\n";
    std::cout << "\nV=\n" << V << "\n";
    std::cout << "\nL=\n" << L << "\n";

    std::cout << "\nV*V^*=\n" << V*feng::conj(V) << "\n";


    return 0;
}



