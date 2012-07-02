#ifndef _DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF
#define _DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{

    template<typename Matrix>
    Matrix const diag( const Matrix& m )
    {
        Matrix ans{m.row(), m.col()};
        std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin() );
        return ans;
    }

}//namespace feng

#endif//_DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

