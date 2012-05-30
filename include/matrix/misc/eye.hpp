#ifndef _EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH
#define _EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T>
    matrix<T> const eye( const std::size_t r, const std::size_t c )
    {
        matrix<T> ans{ r, c };
        std::fill( ans.diag_begin(), ans.diag_end(), T(1) );
        return ans;
    }

}//namespace feng

#endif//_EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

