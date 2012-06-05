#ifndef _HILB_HPP_INCLUDED_SFIOJ8398FDVJKZXM3984444444444444444444444444444444444444444444444444HHHH
#define _HILB_HPP_INCLUDED_SFIOJ8398FDVJKZXM3984444444444444444444444444444444444444444444444444HHHH
#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{

    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const hilb( const std::size_t n )
    {
        matrix<T,D,A> ans( n, n );
        for ( std::size_t i = 0; i < n; ++i )
            for ( std::size_t j = i; j < n; ++j )
            {
                ans[i][j] = T(1) /(i+j+1);
                ans[j][i] = ans[i][j];
            }

        return ans;
    }

    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const hilbert( const std::size_t n )
    {
        return hilb(n);
    }

}//namespace feng

#endif//_HILB_HPP_INCLUDED_SFIOJ8398FDVJKZXM3984444444444444444444444444444444444444444444444444HHHH

