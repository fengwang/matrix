#ifndef _CTRANSPOSE_HPP_INCLUDED_SDIOJ4EIJFJIEIORJOFIWJVKLJJKSFDNVNNNNNNNNNNNNNNNNNNNNNNNNSFDKJHASJDO
#define _CTRANSPOSE_HPP_INCLUDED_SDIOJ4EIJFJIEIORJOFIWJVKLJJKSFDNVNNNNNNNNNNNNNNNNNNNNNNNNSFDKJHASJDO

#include <matrix/matrix.hpp>
#include <matrix/numeric/math.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T, std::size_t D, typename A>
    matrix<std::complex<T>,D,A> const ctranspose( const matrix<std::complex<T>,D,A>& m )
    {
        return conj( m.transpose() );
    }

}//namespace feng

#endif//_CTRANSPOSE_HPP_INCLUDED_SDIOJ4EIJFJIEIORJOFIWJVKLJJKSFDNVNNNNNNNNNNNNNNNNNNNNNNNNSFDKJHASJDO

