#include <matrix.hpp>
#include <vg.hpp>
#include <iostream>
#include <complex>

int main()
{
    using namespace feng;

    unsigned int const n = 1000;
    unsigned int const N = 100;

    matrix<std::complex<double>, 256 > m;
    //matrix<std::complex<double>, 256, feng::matrix_allocator<std::complex<double> > > m;

    vg::variate_generator<double> v;

    for ( unsigned int i = 0; i != N; ++i )
    {
        matrix<double, 256 > A(n, n);
        //matrix<double, 256, feng::matrix_allocator<double> > A(n, n);
        std::copy( v.begin(), v.begin()+n*n, A.begin() );
        matrix<std::complex<double>, 64 > B(n, n);
        //matrix<std::complex<double>, 64, feng::matrix_allocator<std::complex<double> > > B(n, n);
        std::for_each( B.begin(), B.end(), [&v](std::complex<double>& c){c.real(v()); c.imag(v());} );
        
        std::cerr << "\nprocessing the " << i+1 << "th expm now......";

        m += expm(B);
    }

    std::cout << std::accumulate( m.begin(), m.end(), std::complex<double>(0) );

    return 0;
}

