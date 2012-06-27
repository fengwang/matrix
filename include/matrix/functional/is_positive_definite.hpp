#ifndef _IS_POSITIVE_DEFINITE_HPP_INCLUDED_SFDOIJDLFKJDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD3248IUSFLIJ
#define _IS_POSITIVE_DEFINITE_HPP_INCLUDED_SFDOIJDLFKJDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD3248IUSFLIJ

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>
#include <cassert>

namespace feng
{
    template< typename T, std::size_t N, typename A >
    bool is_positive_definite( const matrix<T,N,A>& m )
    {
        typedef matrix<T,N,A> matrix_type;
        typedef typename matrix_type::range_type range_type;

        if ( m.row() != m.col() ) return false;
        
        for ( std::size_t i = 1; i != m.row(); ++i )
        {
            const matrix_type a{ m, range_type{0,i}, range_type{0,i} };
            if ( a.det() <= T(0) ) return false;
        }

       return true; 
    }

}//namespace feng

#endif//_IS_POSITIVE_DEFINITE_HPP_INCLUDED_SFDOIJDLFKJDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD3248IUSFLIJ

