#include <f/matrix/matrix.hpp>

#include <complex>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <iomanip>

int main()
{
    f::matrix<double> Ar;
    Ar.load("upload/ar.data");

    f::matrix<double> Ai;
    Ai.load("upload/ai.data");

    f::matrix<double> Sr;
    Sr.load("upload/sr.data");

    f::matrix<double> Si;
    Si.load("upload/si.data");

    f::matrix<std::complex<double>> A( Ar.row(), Ar.col() );
    f::matrix<std::complex<double>> S( Sr.row(), Sr.col() );

    for ( unsigned long r = 0; r != A.row(); ++r )
        for ( unsigned long c = 0; c != A.col(); ++c )
        {
            A[r][c] = std::complex<double>{ Ar[r][c], Ai[r][c] };
            S[r][c] = std::complex<double>{ Sr[r][c], Si[r][c] };
        }

    auto const nm = f::norm_1( A );

    std::cout << "\nnorm 1 of A is " << nm << "\n";

    unsigned long i = nm < 1.0 ? 0 : std::ceil( std::log2(nm) );
    double s__2 = i ? 1 << i : 1.0;

    auto A_ = A / s__2;
    auto An = A_;

    //double factor = 1.0;
    std::complex<double> factor{ 1.0, 0.0 };

    f::matrix<std::complex<double> > CI( A.row(), A.col() );

    std::fill( CI.begin(), CI.end(), std::complex<double>{ 0.0, 0.0 } );
    std::fill( CI.diag_begin(), CI.diag_end(), std::complex<double>{ 1.0, 0.0 } );


    //for ( unsigned long i = 2; i != 10; ++i )
    for ( unsigned long i = 2; i != 13; ++i )
    {
        CI += An / factor;
        factor *= i;
        //An *= A;
        An *= A_;
    }

/*
        const double c0[4] = { 0.73769924933048902638, 0.13172525137117249705, 0.072080085802694884856, -2.1299489346443743676e-06 };
        const double c1[4] = { -0.77307934030428138161, -0.76302801814514820133, -0.28579955930369160821, -0.041199562184697238743 };
        const double c2[4] = { -1.7534629347707793023, 0.29030564199814418158, -0.086501686893504473042, 3.1635840353521517065e-06 };
*/
#if 0
const double c0[4] = { 1.0, 0.33159674557019125363, 0.084702745557608025151, 0.012034014046639418885 };
const double c1[4] = { 1.0, 0.062573672005820632491, 0.0055295200734670913595, 0.007105742503453451632 };
const double c2[4] = { 1.0, 0.60582951103774895785, 0.15021890262424952844, 0.013946343053716966573 };


        auto const& a = A_;
        auto const& aa = a*a;
        auto const& aaa = a * aa;

        auto const& P0 = c0[0] * CI + c0[1] * a + c0[2] * aa + c0[3] * aaa;
        auto const& P1 = c1[0] * CI + c1[1] * a + c1[2] * aa + c1[3] * aaa;
        auto const& P2 = c2[0] * CI + c2[1] * a + c2[2] * aa + c2[3] * aaa;

        CI = P0 * P1 * P2;

#endif

    for ( unsigned long k = 0; k != i; ++k )
        CI *= CI;

    auto D = CI - S;
    double rmax = 0.0;
    double imax = 0.0;

    for ( unsigned long r = 0; r != D.row(); ++r )
        for ( unsigned long c = 0; c != D.col(); ++c )
        {
            double const ar = std::abs( std::real( D[r][c] ) );
            double const ai = std::abs( std::imag( D[r][c] ) );
            if ( ar > rmax ) rmax = ar;
            if ( ai > imax ) imax = ai;
        }

    std::cout << "\nthe max diff for real is " << rmax << "\n";
    std::cout << "\nthe max diff for imag is " << imax << "\n";

    return 0;
}

