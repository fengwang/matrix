#ifndef _DISPLAY_HPP_INCLUDED_SOFDIJW4EOIJASFLD0JKSAF9LKDJSAFLKJSFDOIJSLKJASFODIJASFOIJWREOIJSAFL3JD
#define _DISPLAY_HPP_INCLUDED_SOFDIJW4EOIJASFLD0JKSAF9LKDJSAFLKJSFDOIJSLKJASFODIJASFOIJWREOIJSAFL3JD

#include <matrix/matrix.hpp>

#include <iostream>

namespace feng
{
    template<typename T, std::size_t D, typename A>
    void display( const matrix<T,D,A>& m )
    {
        std::cout << m << std::endl;
    }

    template<typename T, std::size_t D, typename A>
    void disp( const matrix<T,D,A>& m )
    {
        display( m );
    }

}//namespace feng

#endif//_DISPLAY_HPP_INCLUDED_SOFDIJW4EOIJASFLD0JKSAF9LKDJSAFLKJSFDOIJSLKJASFODIJASFOIJWREOIJSAFL3JD

