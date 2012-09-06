#ifndef _IS_NAN_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU
#define _IS_NAN_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

#include <matrix/matrix.hpp>
#include <algorithm/for_each.hpp>

#include <cmath>

namespace feng
{

    template< typename T, std::size_t N, typename A >
    matrix<bool> is_nan( const matrix<T,N,A>& m )
    {
        matrix<bool> ans( m.row(), m.col() );
        for_each( m.begin(), m.end(), ans.begin(), []( const T& v, bool& a ) { a = std::isnan( v ) ? true : false; } );
        return ans;
    }

    template< typename T, std::size_t N, typename A >
    matrix<bool> isnan( const matrix<T,N,A>& m )
    {
        return is_nan( m );
    }

}//namespace feng

#endif//_IS_NAN_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

