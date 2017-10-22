#ifndef MSWAP_HPP_INCLUDED_SPDJ23OPIJAFLKJASFDOIOJSDAFKLJSAFD38ISFDAKLJHSWEOIJSA
#define MSWAP_HPP_INCLUDED_SPDJ23OPIJAFLKJASFDOIOJSDAFKLJSAFD38ISFDAKLJHSWEOIJSA 

#include <iterator>
#include <cstddef>
#include <type_traits>
#include <cassert>

namespace f
{
    template<typename T>
    void swap( T& lhs, T& rhs )
    {
        assert( std::is_move_constructible<T>::value );
        T tmp(std::move(lhs));
        lhs = std::move(rhs);
        rhs = std::move(tmp);
    }

    template<typename Iterator1, typename Iterator2>
    void swap( Iterator1 first1, Iterator1 last1, Iterator2 first2 )
    {
        if ( first1 == last1 ) return;

        swap( *first1, *first2 );

        std::advance(first1, 1);
        std::advance(first2, 1);
        swap( first1, last1, first2 );
    }

    template<typename Iterator1, typename Iterator2>
    void swap_n( Iterator1 first1, std::size_t n, Iterator2 first2 )
    {
        if ( 0 == n ) return;

        swap( *first1, *first2 );

        std::advance(first1, 1);
        std::advance(first2, 1);
        swap( first1, n-1, first2 );
    }

}//namespace f

#endif//_SWAP_HPP_INCLUDED_SPDJ23OPIJAFLKJASFDOIOJSDAFKLJSAFD38ISFDAKLJHSWEOIJSA 

