// S1-R4 link pair, part B: the same templates as link_a.cc, in a second translation unit.
#include "../../matrix.hpp"

#include <complex>

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

double link_b_double() { return compute<double>(); }
std::complex<double> link_b_complex() { return compute<std::complex<double>>(); }
