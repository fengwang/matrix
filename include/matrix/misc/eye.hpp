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

}//namespace feng

#endif//_EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

