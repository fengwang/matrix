#ifndef _MAGIC_HPP_INCLUDED_ODSFIJ498USFDLIJ498YUFSDLIJHSFDLKJHVSFIUDY4987YFSDKHSFDKJHSFKUHIURWEKSFJHKSJFDHKSJFHKJSFHFHJD
#define _MAGIC_HPP_INCLUDED_ODSFIJ498USFDLIJ498YUFSDLIJHSFDLKJHVSFIUDY4987YFSDKHSFDKJHSFKUHIURWEKSFJHKSJFDHKSJFHKJSFHFHJD

#include <matrix/matrix.hpp>

#include <cstddef>

namespace feng
{
    const matrix<std::size_t>
    magic( const std::size_t n )
    {
        matrix<std::size_t> ans( n, n );
        for(std::size_t i=0; i<n; ++i)
            for(std::size_t j=0; j<n; ++j)
                ans[((n-1)/2+i-j+n)%n][(3*n-1+j-2*i)%n]=i*n+j+1;
        return ans;
    }

}//namespace feng

#endif//_MAGIC_HPP_INCLUDED_ODSFIJ498USFDLIJ498YUFSDLIJHSFDLKJHVSFIUDY4987YFSDKHSFDKJHSFKUHIURWEKSFJHKSJFDHKSJFHKJSFHFHJD

