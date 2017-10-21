#ifndef MMATRIX_VALUE_OPERATORS_HPP_INCLUDED
#define MMATRIX_VALUE_OPERATORS_HPP_INCLUDED

/* operators overloading between Matrix and Value
 *
 * Examples:
 *
 *    double d;
 *    matrix<double> m;
 *    std::size_t n;
 *
 *    matrix<double> m1 = d+m;
 *    matrix<double> m2 = m+d;
 *    matrix<double> m3 = d-m;
 *    matrix<double> m4 = m-d;
 *    matrix<double> m5 = d*m;
 *    matrix<double> m6 = m*d;
 *    m.resize( n, n ); //need square matrix
 *    matrix<double> m7 = d/m;
 *    matrix<double> m8 = m/d;
 *    matrix<double> m9 = d&&m;
 *    matrix<double> m10= m&&d;
 *    matrix<double> m11= d||m;
 *    matrix<double> m12= m||d;
 *    matrix<double> m13= m^2;
 *
 */

namespace f
{

    template< typename T, typename A>
    const matrix<T, A>
    operator + ( const matrix<T, A>& lhs, const T& rhs )
    {
        matrix<T, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return rhs + x;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<T, A>
    operator + ( const T& lhs, const matrix<T, A>& rhs )
    {
        return rhs + lhs;
    }

    template< typename T, typename A>
    const matrix<T, A>
    operator - ( const matrix<T, A>& lhs, const T& rhs )
    {
        matrix<T, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x - rhs;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<T, A>
    operator - ( const T& lhs, const matrix<T, A>& rhs )
    {
        matrix<T, A> ans( rhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [lhs]( T x )
        {
            return -lhs + x;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<T, A>
    operator * ( const matrix<T, A>& lhs, const T& rhs )
    {
        matrix<T, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x * rhs;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<T, A>
    operator * ( const T& lhs, const matrix<T, A>& rhs )
    {
        return rhs * lhs;
    }


    template< typename T, typename A>
    const matrix<T, A>
    operator / ( const matrix<T, A>& lhs, const T& rhs )
    {
        matrix<T, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x / rhs;
        } );
        return ans;
    }
    template< typename T, typename A>
    const matrix<T, A>
    operator / ( const T& lhs, const matrix<T, A>& rhs )
    {
        return lhs * rhs.inverse();
    }


    template< typename T, typename A >
    const matrix<T, A>
    operator || ( const matrix<T, A>& lhs, const T& rhs )
    {
        matrix<T, A> ans( lhs.row(), lhs.col() + 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.col_begin( lhs.col() ), ans.col_end( lhs.col() ), rhs );
        return ans;
    }

    template< typename T, typename A >
    const matrix<T, A>
    operator || ( const T& lhs, const matrix<T, A>& rhs )
    {
        matrix<T, A> ans( rhs.row(), rhs.col() + 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) + 1 );

        std::fill( ans.col_begin( 0 ), ans.col_end( 0 ), rhs );
        return ans;
    }

    template< typename T, typename A >
    const matrix<T, A>
    operator && ( const matrix<T, A>& lhs, const T& rhs )
    {
        matrix<T, A> ans( lhs.row() + 1, lhs.col() );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.row_begin( lhs.row() ), ans.row_end( lhs.row() ), rhs );
        return ans;
    }

    template< typename T, typename A >
    const matrix<T, A>
    operator && ( const T& lhs, const matrix<T, A>& rhs )
    {
        matrix<T, A> ans( rhs.row() + 1, rhs.col() );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i + 1 ) );

        std::fill( ans.row_begin( 0 ), ans.row_end( 0 ), rhs );
        return ans;
    }

    template< typename T, typename A >
    const matrix<T, A>
    operator ^ ( const matrix<T, A>& lhs, std::size_t n )
    {
        assert( lhs.row() == lhs.col() );
        auto const r = lhs.row();

        if ( 0 == n )
            return eye<T>( r, r );

        if ( 1 == n )
            return lhs;

        if ( n & 1 )
            return lhs ^ ( n - 1 ) * lhs;

        auto const lhs_2 = lhs ^ ( n >> 1 );
        return lhs_2 * lhs_2;
    }

}//namespace f

#endif//_MATRIX_VALUE_OPERATORS_HPP_INCLUDED

