//this file is to be removed

#ifndef MMATRIX_NUMERIC_ALGORITHM_HPP_INCLUDED_SODNSALK4O9UANDSFKJ4HIUHADSFKJH43
#define MMATRIX_NUMERIC_ALGORITHM_HPP_INCLUDED_SODNSALK4O9UANDSFKJ4HIUHADSFKJH43

namespace f
{

    //
    // Solve A x = b
    //
    // return :
    //          1   --  Failed
    //          0   --  Success
    template<   typename T1, typename A1,
                typename T2, typename A2,
                typename T3, typename A3 >
    int gauss_jordan_elimination( const matrix<T1, A1>& A, matrix<T2, A2>& x, const matrix<T3, A3>& b )
    {
        typedef matrix<T1, A1>                      matrix_type;
        typedef typename matrix_type::size_type     size_type;
        typedef typename matrix_type::value_type    value_type;
        typedef typename matrix_type::range_type    range_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        matrix_type a( A || b ); // a -- [A:b]
        const size_type n = b.row();
        const size_type m = b.col();
        struct abs_compare
        {
            bool operator()( value_type x, value_type y ) const
            {
                return std::abs( x ) < std::abs( y );
            }
        };
        struct scale_by
        {
            value_type factor;
            scale_by( value_type f ) : factor( f ) {}
            void operator()( value_type& x ) const
            {
                x /= factor;
            }
        };
        struct ratio_by
        {
            value_type ratio;
            ratio_by( value_type r ) : ratio( r ) {}
            value_type operator()( value_type x, value_type y ) const
            {
                return x - y * ratio;
            }
        };

        for ( size_type i = 0; i < n; ++i )
        {
            //find max element
            const size_type
            //p = std::distance( a.col_begin( i ), std::max_element( a.col_begin( i ) + i, a.col_end( i ), []( value_type x, value_type y ) { return std::abs( x ) < std::abs( y );} ) );
            p = std::distance( a.col_begin( i ), std::max_element( a.col_begin( i ) + i, a.col_end( i ), abs_compare() ) );

            //swap row i and row p
            if ( p != i )
            {
                std::swap_ranges( a.row_begin( i ) + i, a.row_end( i ), a.row_begin( p ) + i );
            }

            const value_type factor = a[i][i];

            if ( factor == value_type() ) return 1; // fail to solve

            //eliminate
            //std::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, [factor]( value_type & x ) { x /= factor; } );
            std::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, scale_by( factor ) );

            for ( size_type j = 0; j < n; ++j )
            {
                if ( i == j )
                {
                    continue;
                }

                const value_type ratio = a[j][i];
                //std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), [ratio]( value_type x, value_type y ) { return x - y * ratio; } );
                std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), ratio_by( ratio ) );
            }
        }

        x = matrix_type( a, range_type( 0, n ), range_type( n, m + n ) );
        return 0;
    }

}//namespace f

#endif

