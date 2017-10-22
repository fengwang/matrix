#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/expm.hpp>
#include <f/variate_generator/variate_generator.hpp>

#include <complex>
#include <algorithm>
#include <iostream>

int main()
{
    const unsigned long n = 200;

    f::matrix<double> fm( n, n );
    f::matrix<double> f_( n, n );
    f::matrix<double> _f( n, n );

    f::variate_generator<double> vg( -0.0500, 0.05 );

    std::generate( fm.diag_begin(), fm.diag_end(), vg );
    std::generate( f_.begin(), f_.end(), vg );

    _f = f_.inverse();

    auto const& A_ = f_ * fm * _f;
    auto const& S_ = f::expm( A_ ); //approximation

    std::for_each( fm.diag_begin(), fm.diag_end(), []( double& d ) { d = std::exp( d ); } );

    auto const& _S = f_ * fm * _f;

    std::cout << "\nthe total difference is \n" << S_ - _S << "\n";


    f::matrix<std::complex<double> > FM( n, n );
    f::matrix<std::complex<double> > F_( n, n );
    f::matrix<std::complex<double> > _F( n, n );

    for ( unsigned long i = 0; i != n; ++i )
    {
        FM[i][i] = std::complex<double>( vg(), vg() );

        for ( unsigned long j = 0; j != n; ++j )
            F_[i][j] = std::complex<double>( vg(), vg() );
    }

    _F = F_.inverse();

    auto const A = F_ * FM * _F;

    auto const& S = f::expm( A );

    std::for_each( FM.diag_begin(), FM.diag_end(), []( std::complex<double>& c ) { c = std::exp(c); } );

    std::cout << "\n\ndiff for complex is \n" << F_ * FM * _F - S << "\n";


    f::matrix<std::complex<double> > CI( n, n );

    std::fill( CI.begin(), CI.end(), std::complex<double>{ 0.0, 0.0 } );
    std::fill( CI.diag_begin(), CI.diag_end(), std::complex<double>{ 1.0, 0.0 } );

    auto An = A;

    //double factor = 1.0;
    std::complex<double> factor{ 1.0, 0.0 };

    for ( unsigned long i = 2; i != 10; ++i )
    {
        CI += An / factor;
        factor *= i;
        An *= A;
    }

    std::cout << "\n\ndiff for complex is \n" << CI - S << "\n";

    return 0;
}

