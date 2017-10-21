#ifndef MIS_ROWXXX_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF
#define MIS_ROWXXX_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

namespace f
{

    template< typename T, typename A >
    bool is_row( const matrix<T, A>& m )
    {
        return m.row() == 1;
    }

    template< typename T, typename A >
    bool is_row_matrix( const matrix<T, A>& m )
    {
        return is_row( m );
    }

    template< typename T, typename A >
    bool isrow( const matrix<T, A>& m )
    {
        return is_row( m );
    }
}//namespace f

#endif//_IS_ROWXXX_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

