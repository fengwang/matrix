#ifndef _IS_EMPTY_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF
#define _IS_EMPTY_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

#include <matrix/matrix.hpp>

namespace feng
{

    template< typename T, std::size_t N, typename A >
    bool is_empty( const matrix<T,N,A>& m )
    {
        return m.size() == 0;
    }

    template< typename T, std::size_t N, typename A >
    bool is_empty_matrix( const matrix<T,N,A>& m )
    {
        return is_empty(m);
    }

    template< typename T, std::size_t N, typename A >
    bool isempty( const matrix<T,N,A>& m )
    {
        return is_empty( m );
    }
}//namespace feng

#endif//_IS_EMPTY_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

