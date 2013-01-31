#include <matrix.hpp>
#include <complex>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>

int main()
{
    unsigned long const n = 1500;
    feng::matrix<std::complex<float> > m(n, n);

    std::fill( m.begin(), m.end(), std::complex<float>( 1.0, 2.0 ) );

    m *= m.inverse();

    return 0; 
}

