#include <matrix.hpp>
#include <vg.hpp>
#include <iostream>
#include <complex>

int main()
{
    using namespace feng;

    unsigned int const n = 10;

    //matrix<std::complex<double>, 256 > m;
    matrix<double> m;
    matrix<double> a, a_, lambda;

    vg::variate_generator<double> v;
#if 0
    for ( unsigned int i = 0; i != N; ++i )
    {
        matrix<double, 256 > A(n, n);
        std::copy( v.begin(), v.begin()+n*n, A.begin() );
        matrix<std::complex<double>, 64 > B(n, n);
        std::for_each( B.begin(), B.end(), [&v](std::complex<double>& c){c.real(v()); c.imag(v());} );
        
        std::cerr << "\nprocessing the " << i+1 << "th expm now......";

        m += expm(B);
    }

    std::cout << std::accumulate( m.begin(), m.end(), std::complex<double>(0) );
#endif

    a.resize( n, n );
    std::copy( v.begin(), v.begin()+n*n, a.begin() );
    a_ = inverse(a);
    lambda.resize(n,n);
    std::copy( v.begin(), v.begin()+n, lambda.diag_begin() );

    m = a * lambda * a_;

    std::cout << expm(m) - ( a * (exp(lambda)) * a_ );

    return 0;
}

