#ifndef MIMAG_HPP_INCLUDED_SFP9IJASFLKJ4P9HFVKLJVNA9F8Y4EKLJANSFDNMVC9P8YHSFDLKJ 
#define MIMAG_HPP_INCLUDED_SFP9IJASFLKJ4P9HFVKLJVNA9F8Y4EKLJANSFDNMVC9P8YHSFDLKJ 

#include <f/matrix/impl/matrix_impl.hpp>
#include <f/algorithm/for_each.hpp>

#include <cstddef>
#include <complex>

namespace f
{

    template<typename T, std::size_t D, typename A>
    const matrix<T,D> imag( const matrix<std::complex<T>,D,A>& m )
    {
        matrix<T,D> ans( m.row(), m.col() );

        std::for_each( m.begin(), m.end(), ans.begin(), []( const std::complex<T>& c, T& r ) { r = std::imag(c); } );

        return ans;
    }

}//namespace f

#endif//_IMAG_HPP_INCLUDED_SFP9IJASFLKJ4P9HFVKLJVNA9F8Y4EKLJANSFDNMVC9P8YHSFDLKJ 

