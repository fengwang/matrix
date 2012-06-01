#ifndef _RAND_HPP_INCOUDED_SOFI4398UAFSDIJSLFDKJLVCKXJSAFDLSFDIJ4EIULASFDIJOISFDJAOSFIDJOSFDIU4EUSAL
#define _RAND_HPP_INCOUDED_SOFI4398UAFSDIJSLFDKJLVCKXJSAFDLSFDIJ4EIULASFDIJOISFDJAOSFIDJOSFDIU4EUSAL

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>
#include <ctime>
#include <cstdlib>

namespace feng
{
    template<typename T = double,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const rand( const std::size_t r, const std::size_t c )
    {
        matrix<T> ans{ r, c };
        std::srand(std::time(0) + (std::size_t)(&ans));
        auto const generator = [](){ return T(std::rand())/T(RAND_MAX); };
        //std::fill( ans.diag_begin(), ans.diag_end(), T(1) );
        std::generate( ans.begin(), ans.end(), generator );
        return ans;
    }

    template<typename T = double,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const rand( const std::size_t n )
    {
        return rand<T,D,A>( n, n );
    }

}//namespace feng

#endif//_RAND_HPP_INCOUDED_SOFI4398UAFSDIJSLFDKJLVCKXJSAFDLSFDIJ4EIULASFDIJOISFDJAOSFIDJOSFDIU4EUSAL


