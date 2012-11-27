#include <matrix.hpp>
#include <valarray>
#include <iostream>
#include <algorithm>

int main()
{
    unsigned long m = 9;
    unsigned long n = 3;

    auto M = feng::rand<double>( m, n );

    M.save_as("m.matrix");

    feng::matrix<double> N;//( m, n );

    N.load( "m.matrix" );

    std::cout << "M = \n" << M << "\n";
    std::cout << "N = \n" << N << "\n";

    
    feng::matrix<std::complex<double> > cm( n, n );

    auto c_ = feng::rand<double>( n, n );
    auto m_ = feng::rand<double>( n, n );

    feng::for_each( cm.begin(), cm.end(), c_.begin(), m_.begin(),
                    []( std::complex<double>& com, double cc, double mm )
                    { com = std::complex<double>(cc, mm); } );

    cm.save_as( "cm.matrix" );

    feng::matrix<std::complex<double> > dm;//( n, n );

    dm.load( "cm.matrix" );

    std::cout << "cm = \n" << cm << "\n";
    std::cout << "dm = \n" << dm << "\n";

    return 0;
}

