#ifndef _IS_ROWXXX_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF
#define _IS_ROWXXX_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

#include <matrix/matrix.hpp>

namespace feng
{

    template< typename T, std::size_t N, typename A >
    bool is_row( const matrix<T,N,A>& m )
    {
        return m.row() == 1;
    }

    template< typename T, std::size_t N, typename A >
    bool is_row_matrix( const matrix<T,N,A>& m )
    {
        return is_row(m);
    }

    template< typename T, std::size_t N, typename A >
    bool isrow( const matrix<T,N,A>& m )
    {
        return is_row( m );
    }
}//namespace feng

#endif//_IS_ROWXXX_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

