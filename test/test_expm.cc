#include <matrix.hpp>
#include <vg.hpp>
#include <iostream>
#include <complex>

int main()
{
    using namespace feng;

    unsigned int const n = 500;
    vg::variate_generator<double> v;
    matrix<double> A(n, n);
    std::copy( v.begin(), v.begin()+n*n, A.begin() );
#if  0
    std::cout << "\nA=\n" << A << "\n";
    std::cout << "\ne^A=\n" << expm(A) << "\n";
#endif
#if 1
    matrix<std::complex<double> > B(n, n);
    std::for_each( B.begin(), B.end(), [&v](std::complex<double>& c){c.real(v()); c.imag(v());} );

    //std::cout << "\nA=\n" << B << "\n";
    //std::cout << "\ne^A=\n" << expm(B) << "\n";
    expm(B);

#endif

    return 0;
}

