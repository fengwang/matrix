#ifndef _IS_COLUMN_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF
#define _IS_COLUMN_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

#include <matrix/matrix.hpp>

namespace feng
{

    template< typename T, std::size_t N, typename A >
    bool is_column( const matrix<T,N,A>& m )
    {
        return m.col() == 1;
    }

    template< typename T, std::size_t N, typename A >
    bool is_column_matrix( const matrix<T,N,A>& m )
    {
        return is_column( m );
    }

    template< typename T, std::size_t N, typename A >
    bool iscolumn( const matrix<T,N,A>& m )
    {
        return is_column( m );
    }
}//namespace feng

#endif//_IS_COLUMN_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

