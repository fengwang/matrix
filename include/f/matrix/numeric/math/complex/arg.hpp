#ifndef MARG_HPP_INCLUDED_DPOSIJASDWJISALDKF4VMN94E8YFDKLJS1VMNA498SAFDKLJASFUHF 
#define MARG_HPP_INCLUDED_DPOSIJASDWJISALDKF4VMN94E8YFDKLJS1VMNA498SAFDKLJASFUHF 

#include <f/matrix/impl/matrix_impl.hpp>
#include <f/algorithm/for_each.hpp>

#include <cstddef>
#include <complex>

namespace f
{

    template<typename T, std::size_t D, typename A>
    const matrix<T,D> arg( const matrix<std::complex<T>,D,A>& m )
    {
        matrix<T,D> ans( m.row(), m.col() );

        std::for_each( m.begin(), m.end(), ans.begin(), []( const std::complex<T>& c, T& r ) { r = std::arg(c); } );

        return ans;
    }

}//namespace f

#endif//_ARG_HPP_INCLUDED_DPOSIJASDWJISALDKF4VMN94E8YFDKLJS1VMNA498SAFDKLJASFUHF 

