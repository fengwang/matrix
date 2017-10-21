#ifndef WCERHYYDAFTEOLIILJBKFLYQGEUXUNPOQNYCRSWEAALLRFJORRSKKDMIKPSEGGFVFSRDTDYVR
#define WCERHYYDAFTEOLIILJBKFLYQGEUXUNPOQNYCRSWEAALLRFJORRSKKDMIKPSEGGFVFSRDTDYVR

#include <f/matrix/impl/matrix_impl.hpp>
#include <f/algorithm/for_each.hpp>

#include <cstddef>
#include <complex>

namespace f
{

    template<typename T, std::size_t D, typename A>
    // TODO:
    //      use rebind to generate return type's allocator
    const matrix<T,D> norm( const matrix<std::complex<T>,D,A>& m )
    {
        matrix<T,D> ans( m.row(), m.col() );

        std::for_each( m.begin(), m.end(), ans.begin(), []( const std::complex<T>& c, T& r ) { r = std::norm(c); } );

        return ans;
    }

}//namespace f

#endif//WCERHYYDAFTEOLIILJBKFLYQGEUXUNPOQNYCRSWEAALLRFJORRSKKDMIKPSEGGFVFSRDTDYVR

