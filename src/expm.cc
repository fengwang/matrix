#include <c_interfaces/expm.h>
#include <algorithm/for_each.hpp>
#include <matrix.hpp>
#include <complex>

extern "C" 
void dexpm( double* a, double* b, unsigned long n )
{
    feng::matrix<double> A( n, n );
    std::copy( a, a+n*n, A.begin() );
    A = feng::expm( A );
    std::copy( A.begin(), A.end(), b );
}

extern "C" 
void zexpm( double* a_r, double* a_i, double* b_r, double* b_i, unsigned long n )
{
    feng::matrix<std::complex<double> > A(n, n);
    feng::for_each( A.begin(), A.end(), a_r, a_i, []( std::complex<double>&c, double const r, double const i ) { c = std::complex<double>(r, i); } );
    A = feng::expm( A );
    feng::for_each( A.begin(), A.end(), b_r, b_i, []( const std::complex<double>&c, double& r, double& i ) { r = std::real(c); i = std::imag(c); } );
}

