#ifndef MIS_NAN_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU
#define MIS_NAN_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

#include "../details/algorithm/for_each.hpp"

namespace f
{

    template< typename T, typename A >
    matrix<bool> is_nan( const matrix<T, A>& m )
    {
        matrix<bool> ans( m.row(), m.col() );
        for_each( m.begin(), m.end(), ans.begin(), []( const T & v, bool & a )
        {
            a = std::isnan( v ) ? true : false;
        } );
        return ans;
    }

    template< typename T, typename A >
    matrix<bool> isnan( const matrix<T, A>& m )
    {
        return is_nan( m );
    }

}//namespace f

#endif//_IS_NAN_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

