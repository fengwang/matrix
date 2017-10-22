#ifndef MCOL_ITERATOR_HPP_INCLUDED_SDPHADFH4KLJASFDKLJH49Y8HAFDJHSDAKLHJSD9YHU84
#define MCOL_ITERATOR_HPP_INCLUDED_SDPHADFH4KLJASFDKLJH49Y8HAFDJHSDAKLHJSD9YHU84

namespace f
{
    //TODO: noexcept attribute append
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_col_iterator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::col_type                              col_type;
        typedef typename type_proxy_type::const_col_type                        const_col_type;
        typedef typename type_proxy_type::reverse_col_type                      reverse_col_type;
        typedef typename type_proxy_type::const_reverse_col_type                const_reverse_col_type;

        col_type col_begin( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return col_type( zen.begin() + index, zen.col() );
        }

        col_type col_end( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return col_begin( index ) + zen.row();
        }

        const_col_type col_begin( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_col_type( zen.begin() + index, zen.col() );
        }

        const_col_type col_end( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return col_begin( index ) + zen.row();
        }

        const_col_type col_cbegin( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_col_type( zen.begin() + index, zen.col() );
        }

        const_col_type col_cend( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return col_begin( index ) + zen.row();
        }

        reverse_col_type col_rbegin( const size_type index = 0 )
        {
            return reverse_col_type( col_end( index ) );
        }

        reverse_col_type col_rend( const size_type index = 0 )
        {
            return reverse_col_type( col_begin( index ) );
        }

        const_reverse_col_type col_rbegin( const size_type index = 0 ) const
        {
            return const_reverse_col_type( col_end( index ) );
        }

        const_reverse_col_type col_rend( const size_type index = 0 ) const
        {
            return const_reverse_col_type( col_begin( index ) );
        }

        const_reverse_col_type col_crbegin( const size_type index = 0 ) const
        {
            return const_reverse_col_type( col_end( index ) );
        }

        const_reverse_col_type col_crend( const size_type index = 0 ) const
        {
            return const_reverse_col_type( col_begin( index ) );
        }

    };//struct crtp_col_iterator

}

#endif//_COL_ITERATOR_HPP_INCLUDED_SDPHADFH4KLJASFDKLJH49Y8HAFDJHSDAKLHJSD9YHU84

