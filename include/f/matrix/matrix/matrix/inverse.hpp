#ifndef MINVERSE_HPP_INCLUDED_DSPHJSAFD3OUHASFKLJN43NIOAFSLJK4HOHIAFSDLJKN4SFDJH
#define MINVERSE_HPP_INCLUDED_DSPHJSAFD3OUHASFKLJN43NIOAFSLJK4HOHIAFSDLJKN4SFDJH

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_inverse
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::value_type                            value_type;
        typedef typename type_proxy_type::range_type                            range_type;

        const zen_type inverse() const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            assert( zen.row() == zen.col() );
            size_type const n = zen.row();
            zen_type a( n, n + n, value_type( 0 ) );

            for ( size_type i = 0; i != n; ++i )
            {
                std::copy( zen.row_begin( i ), zen.row_end( i ), a.row_begin( i ) );
            }

            std::fill( a.upper_diag_begin( n ), a.upper_diag_end( n ), value_type( 1 ) );

            for ( size_type i = 0; i < n; ++i )
            {
                //find max element
                const size_type p = std::distance( a.col_begin( i ), std::max_element( a.col_begin( i ) + i, a.col_end( i ), []( value_type x, value_type y )
                {
                    return std::abs( x ) < std::abs( y );
                } ) );

                //swap row i and row p
                if ( p != i )
                {
                    std::swap_ranges( a.row_begin( i ) + i, a.row_end( i ), a.row_begin( p ) + i );
                }

                const value_type factor = a[i][i];
                assert( factor != value_type() );
                //eliminate
                std::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, [factor]( value_type & x )
                {
                    x /= factor;
                } );

                for ( size_type j = 0; j < n; ++j )
                {
                    if ( i == j )
                    {
                        continue;
                    }

                    const value_type ratio = a[j][i];
                    std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), [ratio]( value_type x, value_type y )
                    {
                        return x - y * ratio;
                    } );
                }
            }

            return zen_type( a, range_type( 0, n ), range_type( n, n + n ) );
        }

    };//struct crtp_inverse

}

#endif//_INVERSE_HPP_INCLUDED_DSPHJSAFD3OUHASFKLJN43NIOAFSLJK4HOHIAFSDLJKN4SFDJH

