#ifndef _EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH
#define _EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const eye( const std::size_t r, const std::size_t c )
    {
        matrix<T> ans{ r, c };
        std::fill( ans.diag_begin(), ans.diag_end(), T(1) );
        return ans;
    }

    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const eye( const std::size_t n )
    {
        return eye<T,D,A>( n, n );
    }

    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const eye( const matrix<T,D,A>& m )
    {
        return eye<T,D,A>( m.row(), m.col() );
    }

    template<typename Matrix>
    Matrix const eye( const std::size_t r, const std::size_t c, const Matrix& )
    {
        Matrix m( r, c );
        std::fill( m.diag_begin(), m.diag_end(), typename Matrix::value_type(1) );
        return m;
    }

    template<typename Matrix>
    Matrix const eye( const std::size_t n, const Matrix& m )
    {
        return eye(n, n, m);
    }

}//namespace feng

#endif//_EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

