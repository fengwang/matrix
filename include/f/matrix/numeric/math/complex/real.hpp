#ifndef MREAL_HPP_INCLUDED_SDPOINASFLKJ4EPOINAFDLJKFANDKLJSFDKLJSFASFIUNFDSAFDDF
#define MREAL_HPP_INCLUDED_SDPOINASFLKJ4EPOINAFDLJKFANDKLJSFDKLJSFASFIUNFDSAFDDF

#include <f/matrix/impl/matrix_impl.hpp>
#include <f/algorithm/for_each.hpp>

#include <cstddef>
#include <complex>

namespace f
{

    template<typename T, std::size_t D, typename A>
    const matrix<T,D> real( const matrix<std::complex<T>,D,A>& m )
    {
        matrix<T,D> ans( m.row(), m.col() );

        std::for_each( m.begin(), m.end(), ans.begin(), []( const std::complex<T>& c, T& r ) { r = std::real(c); } );

        return ans;
    }

}//namespace f

#endif//_REAL_HPP_INCLUDED_SDPOINASFLKJ4EPOINAFDLJKFANDKLJSFDKLJSFASFIUNFDSAFDDF

