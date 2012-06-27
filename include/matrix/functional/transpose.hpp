#ifndef _TRANSPOSE_HPP_INCLUDED_SDIOJ4EIJFJIEIORJOFIWJVKLJJKSFDNVNNNNNNNNNNNNNNNNNNNNNNNNSFDKJHASJDO
#define _TRANSPOSE_HPP_INCLUDED_SDIOJ4EIJFJIEIORJOFIWJVKLJJKSFDNVNNNNNNNNNNNNNNNNNNNNNNNNSFDKJHASJDO

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const transpose( const matrix<T,D,A>& m )
    {
        return m.transpose();
    }

}//namespace feng

#endif//_TRANSPOSE_HPP_INCLUDED_SDIOJ4EIJFJIEIORJOFIWJVKLJJKSFDNVNNNNNNNNNNNNNNNNNNNNNNNNSFDKJHASJDO

