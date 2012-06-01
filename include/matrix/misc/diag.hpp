#ifndef _DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF
#define _DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const diag( const matrix<T,D,A>& m )
    {
        matrix<T,D,A> ans{m.row(), m.col()};
        std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin() );
        return ans;
    }

}//namespace feng

#endif//_DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

