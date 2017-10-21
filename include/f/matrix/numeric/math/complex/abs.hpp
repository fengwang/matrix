#ifndef MABS_HPP_INCLUDED_FDSPONSA3PIOJAFASFSDKAJ34KLAFJD0OIJHAFDKLJSAFDOJFDEFSD 
#define MABS_HPP_INCLUDED_FDSPONSA3PIOJAFASFSDKAJ34KLAFJD0OIJHAFDKLJSAFDOJFDEFSD 

#include <f/matrix/impl/matrix_impl.hpp>
#include <f/algorithm/for_each.hpp>

#include <cstddef>
#include <complex>

namespace f
{

    template<typename T, std::size_t D, typename A>
    const matrix<T,D> abs( const matrix<std::complex<T>,D,A>& m )
    {
        matrix<T,D> ans( m.row(), m.col() );

        std::for_each( m.begin(), m.end(), ans.begin(), []( const std::complex<T>& c, T& r ) { r = std::abs(c); } );

        return ans;
    }

}//namespace f

#endif//_ABS_HPP_INCLUDED_FDSPONSA3PIOJAFASFSDKAJ34KLAFJD0OIJHAFDKLJSAFDOJFDEFSD 

