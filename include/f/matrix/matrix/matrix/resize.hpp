#ifndef MRESIZE_HPP_INCLUDED_FDPONSF2398ASDFKLV9U8YH3KLJAFSD893YHALFIJHCVNJSIUFH
#define MRESIZE_HPP_INCLUDED_FDPONSF2398ASDFKLV9U8YH3KLJAFSD893YHALFIJHCVNJSIUFH

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_resize
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::value_type                            value_type;

        zen_type& resize( const size_type new_row, const size_type new_col, const value_type v )
        {
            zen_type& zen = static_cast<zen_type&>( *this );

            if ( ( zen.row() == new_row ) && ( zen.col() == new_col ) )
            {
                return zen;
            }

            zen_type ans( new_row, new_col, v );
            zen.swap( ans );
            return zen;
        }

        zen_type& resize( const size_type new_row, const size_type new_col )
        {
            zen_type& zen = static_cast<zen_type&>( *this );

            if ( ( zen.row() == new_row ) && ( zen.col() == new_col ) )
            {
                return zen;
            }

            if ( zen.size() == new_row * new_col )
            {
                zen.row_ = new_row;
                zen.col_ = new_col;
                return zen;
            }

            zen_type ans( new_row, new_col );
            zen.swap( ans );
            return zen;
        }

    };//struct crtp_resize

}

#endif//_RESIZE_HPP_INCLUDED_FDPONSF2398ASDFKLV9U8YH3KLJAFSD893YHALFIJHCVNJSIUFH

