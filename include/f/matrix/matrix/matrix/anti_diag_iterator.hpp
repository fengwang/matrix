#ifndef AANTI_DIAG_ITERATOR_HPP_INCLUDED_SDSAHFDKLJSD498YALFHSAF0Y8943LOHSFDKLJH
#define AANTI_DIAG_ITERATOR_HPP_INCLUDED_SDSAHFDKLJSD498YALFHSAF0Y8943LOHSFDKLJH

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_anti_diag_iterator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::difference_type                       difference_type;
        typedef typename type_proxy_type::anti_diag_type                        anti_diag_type;
        typedef typename type_proxy_type::const_anti_diag_type                  const_anti_diag_type;
        typedef typename type_proxy_type::reverse_anti_diag_type                reverse_anti_diag_type;
        typedef typename type_proxy_type::const_reverse_anti_diag_type          const_reverse_anti_diag_type;

        anti_diag_type upper_anti_diag_begin( const size_type index = 0 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return anti_diag_type( zen.begin() + zen.col() - index - 1, zen.col() - 1 );
        }

        anti_diag_type upper_anti_diag_end( const size_type index = 0 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return upper_anti_diag_begin( index ) + depth;
        }

        const_anti_diag_type upper_anti_diag_begin( const size_type index = 0 )  const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_anti_diag_type( zen.begin() + zen.col() - index - 1, zen.col() - 1 );
        }

        const_anti_diag_type upper_anti_diag_end( const size_type index = 0 )  const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return upper_anti_diag_begin( index ) + depth;
        }

        const_anti_diag_type upper_anti_diag_cbegin( const size_type index = 0 )  const
        {
            return upper_anti_diag_begin( index );
        }

        const_anti_diag_type upper_anti_diag_cend( const size_type index = 0 )  const
        {
            return upper_anti_diag_end( index );
        }

        reverse_anti_diag_type upper_anti_diag_rbegin( const size_type index = 0 )
        {
            return reverse_anti_diag_type( upper_anti_diag_end( index ) );
        }

        reverse_anti_diag_type upper_anti_diag_rend( const size_type index = 0 )
        {
            return reverse_anti_diag_type( upper_anti_diag_begin( index ) );
        }

        const_reverse_anti_diag_type upper_anti_diag_rbegin( const size_type index = 0 )  const
        {
            return const_reverse_anti_diag_type( upper_anti_diag_end( index ) );
        }

        const_reverse_anti_diag_type upper_anti_diag_rend( const size_type index = 0 )  const
        {
            return const_reverse_anti_diag_type( upper_anti_diag_begin( index ) );
        }

        const_reverse_anti_diag_type upper_anti_diag_crbegin( const size_type index = 0 )  const
        {
            return upper_anti_diag_rbegin( index );
        }

        const_reverse_anti_diag_type upper_anti_diag_crend( const size_type index = 0 )  const
        {
            return upper_anti_diag_rend( index );
        }


        anti_diag_type lower_anti_diag_begin( const size_type index = 0 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return anti_diag_type( zen.begin() + ( zen.col() * ( index + 1 ) ) - 1, zen.col() - 1 );
        }

        anti_diag_type lower_anti_diag_end( const size_type index = 0 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_anti_diag_begin( index ) + depth;
        }

        const_anti_diag_type lower_anti_diag_begin( const size_type index = 0 )  const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_anti_diag_type( zen.begin() + ( zen.col() * ( index + 1 ) ) - 1, zen.col() - 1 );
        }

        const_anti_diag_type lower_anti_diag_end( const size_type index = 0 )  const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_anti_diag_begin( index ) + depth;
        }

        const_anti_diag_type lower_anti_diag_cbegin( const size_type index = 0 )  const
        {
            return lower_anti_diag_begin( index );
        }

        const_anti_diag_type lower_anti_diag_cend( const size_type index = 0 )  const
        {
            return lower_anti_diag_end( index );
        }

        reverse_anti_diag_type lower_anti_diag_rbegin( const size_type index = 0 )
        {
            return reverse_anti_diag_type( lower_anti_diag_end( index ) );
        }

        reverse_anti_diag_type lower_anti_diag_rend( const size_type index = 0 )
        {
            return reverse_anti_diag_type( lower_anti_diag_begin( index ) );
        }

        const_reverse_anti_diag_type lower_anti_diag_rbegin( const size_type index = 0 )  const
        {
            return const_reverse_anti_diag_type( lower_anti_diag_end( index ) );
        }

        const_reverse_anti_diag_type lower_anti_diag_rend( const size_type index = 0 )  const
        {
            return const_reverse_anti_diag_type( lower_anti_diag_begin( index ) );
        }

        const_reverse_anti_diag_type lower_anti_diag_crbegin( const size_type index = 0 )  const
        {
            return lower_anti_diag_rbegin( index );
        }

        const_reverse_anti_diag_type lower_anti_diag_crend( const size_type index = 0 )  const
        {
            return lower_anti_diag_rend( index );
        }

        anti_diag_type anti_diag_begin( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_anti_diag_begin( index );
            }

            return lower_anti_diag_begin( -index );
        }

        anti_diag_type anti_diag_end( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_anti_diag_end( index );
            }

            return lower_anti_diag_end( -index );
        }

        const_anti_diag_type anti_diag_begin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_begin( index );
            }

            return lower_anti_diag_begin( -index );
        }

        const_anti_diag_type anti_diag_end( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_end( index );
            }

            return lower_anti_diag_end( -index );
        }

        const_anti_diag_type anti_diag_cbegin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_cbegin( index );
            }

            return lower_anti_diag_cbegin( -index );
        }

        const_anti_diag_type anti_diag_cend( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_cend( index );
            }

            return lower_anti_diag_cend( -index );
        }

        reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rbegin( index );
            }

            return lower_anti_diag_rbegin( -index );
        }

        reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rend( index );
            }

            return lower_anti_diag_rend( -index );
        }

        const_reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rbegin( index );
            }

            return lower_anti_diag_rbegin( -index );
        }

        const_reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rend( index );
            }

            return lower_anti_diag_rend( -index );
        }

        const_reverse_anti_diag_type anti_diag_crbegin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_crbegin( index );
            }

            return lower_anti_diag_crbegin( -index );
        }

        const_reverse_anti_diag_type anti_diag_crend( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_anti_diag_crend( index );
            }

            return lower_anti_diag_crend( -index );
        }

    };//struct  crtp_anti_diag_iterator

}

#endif//_ANTI_DIAG_ITERATOR_HPP_INCLUDED_SDSAHFDKLJSD498YALFHSAF0Y8943LOHSFDKLJH

