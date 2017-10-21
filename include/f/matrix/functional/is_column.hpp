#ifndef MIS_COLUMN_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF
#define MIS_COLUMN_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

namespace f
{

    template< typename T, typename A >
    bool is_column( const matrix<T, A>& m )
    {
        return m.col() == 1;
    }

    template< typename T, typename A >
    bool is_column_matrix( const matrix<T, A>& m )
    {
        return is_column( m );
    }

    template< typename T, typename A >
    bool iscolumn( const matrix<T, A>& m )
    {
        return is_column( m );
    }
}//namespace f

#endif//_IS_COLUMN_HPP_INCLUDED_SOIJASOYU849HSKFJHSF98UYVC98UYFD4JSFUIHRLEUHGGGGUHFFFFFFFFFFFFFFFFFF

