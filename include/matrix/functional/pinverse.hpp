#ifndef _PINVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH
#define _PINVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH

#include <matrix/matrix.hpp>
#include <matrix/numeric/singular_value_decomposition.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename Matrix>
    Matrix const pinverse( const Matrix& m )
    {
        //non-square matrix situations need considering
        Matrix u, w, v;
        singular_value_decomposition( m, u, w, v );

        return v * w * u.transpose();
    }

    template<typename Matrix>
    Matrix const pinv( const Matrix& m )
    {
        return pinverse(m);
    }

}//namespace feng

#endif//_PINVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH
