#ifndef MSTREAM_OPERATOR_HPP_INCLUDED_SDPONASPOIJ3P9HASFDLH4P9HIASFDLHI4P9H8AUFS
#define MSTREAM_OPERATOR_HPP_INCLUDED_SDPONASPOIJ3P9HASFDLH4P9HIASFDLHI4P9H8AUFS

namespace f
{
    //TODO: precision control for different Types
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_stream_operator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::value_type                            value_type;

        friend std::ostream& operator << ( std::ostream& lhs, zen_type const& rhs )
        {
            lhs.precision( 20 );

            for ( size_type i = 0; i < rhs.row(); ++i )
            {
                std::copy( rhs.row_begin( i ), rhs.row_end( i ), std::ostream_iterator<value_type> ( lhs, "\t" ) );
                lhs << "\n";
            }

            return lhs;
        }

        friend std::istream& operator >> ( std::istream& is, zen_type& rhs )
        {
            std::vector<std::string> row_element;
            std::string string_line;

            while ( std::getline( is, string_line, '\n' ) )
                row_element.push_back( string_line );

            size_type const row = row_element.size();
            size_type const col = std::count_if( row_element[0].begin(), row_element[0].end(), []( char ch )
            {
                return '\t' == ch;
            } );

            //error check
            if ( row == 0 || col == 0 )
            {
                is.setstate( std::ios::failbit );
                return is;
            }

            rhs.resize( row, col );

            for ( size_type r = 0; r != row; ++r )
            {
                std::istringstream the_row( row_element[r] );
                std::copy( std::istream_iterator<value_type>( the_row ), std::istream_iterator<value_type>(), rhs.row_begin( r ) );
            }

            return is;
        }

    };//struct

}

#endif//_STREAM_OPERATOR_HPP_INCLUDED_SDPONASPOIJ3P9HASFDLH4P9HIASFDLHI4P9H8AUFS

