#ifndef _TOEPLITZ_HPP_INCLUDED_DSOIFJ4ID9UIF498OSDIFAKLSFDJVNVCXNKSFDJAHSFDAOI4HOSDIFT4UISFDIOU489YJ
#define _TOEPLITZ_HPP_INCLUDED_DSOIFJ4ID9UIF498OSDIFAKLSFDJVNVCXNKSFDJAHSFDAOI4HOSDIFT4UISFDIOU489YJ

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>
#include <ctime>
#include <cstdlib>

namespace feng
{
    template<typename Itor1,
             typename Itor2,
             typename T = double,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const toeplitz( Itor1 i1_, Itor1 _i1, Itor2 i2_, Itor2 _i2 )
    {
        std::size_t r = std::distance( i1_, _i1 );
        std::size_t c = std::distance( i2_, _i2 );
        matrix<T,D,A> m( r, c );
        for ( std::size_t i = 0; i != r; ++i )
            std::fill( m.lower_diag_begin(i), m.lower_diag_end(i), *(i1_+i) );
        for ( std::size_t i = 1; i != c; ++i )
            std::fill( m.upper_diag_begin(i), m.upper_diag_end(i), *(i2_+i) );

        return m;
    }

}//namespace feng

#endif//_TOEPLITZ_HPP_INCLUDED_DSOIFJ4ID9UIF498OSDIFAKLSFDJVNVCXNKSFDJAHSFDAOI4HOSDIFT4UISFDIOU489YJ

