#ifndef MPINVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH
#define MPINVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH

#include "../numeric/singular_value_decomposition.hpp"

namespace f
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
        return pinverse( m );
    }

}//namespace f

#endif//_PINVERSE_HPP_INCLUDED_DSOIJ498UASFDKLJHVJNSAFIUH49UHSFDKJHSJKNMVCKJHAFDSKJHAUIHASDKJVKJHSFUH
