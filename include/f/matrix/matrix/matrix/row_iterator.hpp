#ifndef MROW_ITERATOR_HPP_INCLUDED_DSNOASD3298YALKJHPUAFDJKH587YHAFSDKLJHSAFDIA
#define MROW_ITERATOR_HPP_INCLUDED_DSNOASD3298YALKJHPUAFDJKH587YHAFSDKLJHSAFDIA

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_row_iterator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::row_type                              row_type;
        typedef typename type_proxy_type::const_row_type                        const_row_type;
        typedef typename type_proxy_type::reverse_row_type                      reverse_row_type;
        typedef typename type_proxy_type::const_reverse_row_type                const_reverse_row_type;

        row_type row_begin( const size_type index = 0 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            //return row_type( zen.begin() + index * zen.col(), 1 );
            return row_type( zen.begin() + index * zen.col() );
        }

        row_type row_end( const size_type index = 0 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return row_begin( index ) + zen.col();
        }

        const_row_type row_begin( const size_type index = 0 ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            //return const_row_type( zen.begin() + index * zen.col(), 1 );
            return const_row_type( zen.begin() + index * zen.col() );
        }

        const_row_type row_end( const size_type index = 0 ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return row_begin( index ) + zen.col();
        }

        const_row_type row_cbegin( const size_type index = 0 ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            //return const_row_type( zen.begin() + index * zen.col(), 1 );
            return const_row_type( zen.begin() + index * zen.col() );
        }

        const_row_type row_cend( const size_type index = 0 ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return row_begin( index ) + zen.col();
        }

        reverse_row_type row_rbegin( const size_type index = 0 )
        {
            return reverse_row_type( row_end( index ) );
        }

        reverse_row_type row_rend( const size_type index = 0 )
        {
            return reverse_row_type( row_begin( index ) );
        }

        const_reverse_row_type row_rbegin( const size_type index = 0 ) const
        {
            return const_reverse_row_type( row_end( index ) );
        }

        const_reverse_row_type row_rend( const size_type index = 0 ) const
        {
            return const_reverse_row_type( row_begin( index ) );
        }

        const_reverse_row_type row_crbegin( const size_type index = 0 ) const
        {
            return const_reverse_row_type( row_end( index ) );
        }

        const_reverse_row_type row_crend( const size_type index = 0 ) const
        {
            return const_reverse_row_type( row_begin( index ) );
        }

    };//struct

}

#endif//_ROW_ITERATOR_HPP_INCLUDED_DSNOASD3298YALKJHPUAFDJKH587YHAFSDKLJHSAFDIA

