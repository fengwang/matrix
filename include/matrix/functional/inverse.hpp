#ifndef _INVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH
#define _INVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename Matrix>
    Matrix const inverse( const Matrix& m )
    {
        return m.inverse();
    }

    template<typename Matrix>
    Matrix const inv( const Matrix& m )
    {
        return m.inverse();
    }

}//namespace feng

#endif//_INVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH
