// S1-R4 link pair, part A (with main): instantiates the same matrix templates as link_b.cc so the two objects
// carry the same inline and template definitions; linking them checks the header for ODR and duplicate-symbol
// defects. Exits 0 when both parts compute the same results.
#include "../../matrix.hpp"

#include <complex>

double link_b_double();
std::complex<double> link_b_complex();

namespace
{
    template < typename T >
    T compute()
    {
        feng::matrix<T> a = feng::eye<T>( 4, 4 );
        a = a * T{ 2 } + a;
        auto const p = a ^ 3;
        auto const blk = p.clone( { 0, 2 }, { 0, 2 } );
        return blk[1][1] + p[3][3];
    }
} // namespace

int main()
{
    if ( compute<double>() != link_b_double() )
        return 1;
    if ( compute<std::complex<double>>() != link_b_complex() )
        return 2;
    if ( compute<double>() != 54.0 )
        return 3;
    return 0;
}
