#ifndef MIS_INF_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU
#define MIS_INF_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

#include "../details/algorithm/for_each.hpp"

namespace f
{

    template< typename T, typename A >
    matrix<bool> is_inf( const matrix<T, A>& m )
    {
        matrix<bool> ans( m.row(), m.col() );
        for_each( m.begin(), m.end(), ans.begin(), []( const T & v, bool & a )
        {
            a = std::isinf( v ) ? true : false;
        } );
        return ans;
    }

    template< typename T, typename A >
    matrix<bool> isinf( const matrix<T, A>& m )
    {
        return is_inf( m );
    }

}//namespace f

#endif//_IS_INF_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

