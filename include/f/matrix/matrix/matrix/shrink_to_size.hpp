#ifndef TILVRAWFEKOGUQEEWGEMBQWAYLDIJSWSLEXQDJLUYWTKJCVNUYUIFLKAVGWAVMSIKQMKHBIJK
#define TILVRAWFEKOGUQEEWGEMBQWAYLDIJSWSLEXQDJLUYWTKJCVNUYUIFLKAVGWAVMSIKQMKHBIJK

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_shrink_to_size
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::value_type                            value_type;

#if 0
        zen_type& shrink_to_size( const size_type new_row, const size_type new_col = 1 )
        {
            assert( new_row && new_col );
            zen_type& zen = static_cast<zen_type&>( *this );

            if ( new_row == zen.row() && new_col == zen.col() )
                return zen;

            assert( new_row <= zen.row() && new_col <= zen.col() );
            zen_type other{ new_row, new_col };

            if ( new_col >= new_row )
                for ( size_type r = 0; r != new_row; ++r )
                    std::copy( zen.row_begin( r ), zen.row_begin( r ) + new_col, other.row_begin( r ) );
            else
                for ( size_type c = 0; c != new_col; ++c )
                    std::copy( zen.col_begin( c ), zen.col_begin( c ) + new_row, other.col_begin( c ) );

            zen.swap( other );
            return zen;
        }
#endif

        zen_type& shrink_to_size( const size_type new_row, const size_type new_col )
        {
            assert( new_row && new_col );
            zen_type& zen = static_cast<zen_type&>( *this );

            if ( new_row == zen.row() && new_col == zen.col() )
                return zen;

            zen_type other{ new_row, new_col };
            std::fill( other.begin(), other.end(), value_type{} );
            size_type const the_rows_to_copy = std::min( zen.row(), new_row );
            size_type const the_cols_to_copy = std::min( zen.col(), new_col );

            for ( size_type r = 0; r != the_rows_to_copy; ++r )
                std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_rows_to_copy, other.row_begin( r ) );

            zen.swap( other );
            return zen;
        }



#if 0
        //TODO:
        //          impl a local method to impl all these shink methods ...
        //
        zen_type& shrink_to_upper_left( const size_type new_row, const size_type new_col = 1 )
        {
        }

        zen_type& shrink_to_upper_right( const size_type new_row, const size_type new_col = 1 )
        {
        }

        zen_type& shrink_to_lower_left( const size_type new_row, const size_type new_col = 1 )
        {
        }

        zen_type& shrink_to_lower_right( const size_type new_row, const size_type new_col = 1 )
        {
        }
#endif
    };//struct crtp_shrink_to_size

}

#endif//TILVRAWFEKOGUQEEWGEMBQWAYLDIJSWSLEXQDJLUYWTKJCVNUYUIFLKAVGWAVMSIKQMKHBIJK

