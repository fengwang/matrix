#ifndef _ZEROS_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR
#define _ZEROS_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const zeros( const std::size_t r, const std::size_t c )
    {
        matrix<T> ans{ r, c, T(0) };
        return ans;
    }

    template<typename T,
             std::size_t D = 256,
             typename A = std::allocator<typename remove_const<typename remove_reference<T>::result_type>::result_type> >
    matrix<T,D,A> const zeros( const std::size_t n )
    {
        return zeros<T,D,A>( n, n );
    }

    template<typename T, std::size_t D, typename A >
    matrix<T,D,A> const zeros( const matrix<T,D,A>& m )
    {
        return zeros<T,D,A>( m.row(), m.col() );
    }

    template<typename T, std::size_t D, typename A >
    matrix<T,D,A> const zeros( const matrix<T,D,A>& m, const std::size_t r, const std::size_t c )
    {
        return zeros<T,D,A>( r, c );
    }

}//namespace feng

#endif//_ZEROS_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR

