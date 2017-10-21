#ifndef MIS_EQUAL_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU
#define MIS_EQUAL_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

namespace f
{

    template< typename T, typename A >
    bool is_equal( const matrix<T, A>& m1, const matrix<T, A>m2 )
    {
        return m1 == m2;
    }

    template< typename M1, typename M2, typename ... Mn >
    bool is_equal( const M1& m1, const M2& m2, const Mn& ... mn )
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }

    template< typename T, typename A >
    bool isequal( const matrix<T, A>& m1, const matrix<T, A>m2 )
    {
        return is_equal( m1, m2 );
    }

    template< typename M1, typename M2, typename ... Mn >
    bool isequal( const M1& m1, const M2& m2, const Mn& ... mn )
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }

}//namespace f

#endif//_IS_EQUAL_HPP_INCLUDED_SFDOJASFOIJSADOIJSDFO9YU8498YUASFKIHSFDUIOHVIUOHSAF978Y4UIJHASFUYH4EU

