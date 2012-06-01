#ifndef _TRIU_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF
#define _TRIU_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T, std::size_t D, typename A >
    matrix<T,D,A> const triu( const matrix<T,D,A>& m )
    {
        matrix<T,D,A> ans{m.row(), m.col()};
        for ( std::size_t i = 0; i != m.col(); ++i )
            std::copy( m.upper_diag_cbegin(i), m.upper_diag_cend(i), ans.upper_diag_begin(i) );
        return ans;
    }

}//namespace feng

#endif//_TRIU_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF

