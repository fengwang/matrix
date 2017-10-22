#ifndef MDIAG_ITERATOR_HPP_INCLUDED_DFDS9HI3HILJUASFDKJVKJNDASJKHFSDIOHSF98Y4FSD
#define MDIAG_ITERATOR_HPP_INCLUDED_DFDS9HI3HILJUASFDKJVKJNDASJKHFSDIOHSF98Y4FSD

namespace f
{
    //TODO: noexcept attribute
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_diag_iterator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::difference_type                       difference_type;
        typedef typename type_proxy_type::diag_type                             diag_type;
        typedef typename type_proxy_type::const_diag_type                       const_diag_type;
        typedef typename type_proxy_type::reverse_upper_diag_type               reverse_upper_diag_type;
        typedef typename type_proxy_type::const_reverse_upper_diag_type         const_reverse_upper_diag_type;
        typedef typename type_proxy_type::reverse_lower_diag_type               reverse_lower_diag_type;
        typedef typename type_proxy_type::const_reverse_lower_diag_type         const_reverse_lower_diag_type;
        typedef typename type_proxy_type::reverse_diag_type                     reverse_diag_type;
        typedef typename type_proxy_type::const_reverse_diag_type               const_reverse_diag_type;

        diag_type upper_diag_begin( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return diag_type( zen.begin() + index, zen.col() + 1 );
        }

        diag_type upper_diag_end( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return diag_type( upper_diag_begin( index ) + depth );
        }

        const_diag_type upper_diag_begin( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_diag_type( zen.begin() + index, zen.col() + 1 );
        }

        const_diag_type upper_diag_end( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return upper_diag_begin( index ) + depth;
        }

        const_diag_type upper_diag_cbegin( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_diag_type( zen.cbegin() + index, zen.col() + 1 );
        }

        const_diag_type upper_diag_cend( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return upper_diag_cbegin( index ) + depth;
        }

        reverse_upper_diag_type
        upper_diag_rbegin( const size_type index = 0 )
        {
            return reverse_upper_diag_type( upper_diag_end( index ) );
        }

        reverse_upper_diag_type
        upper_diag_rend( const size_type index = 0 )
        {
            return reverse_upper_diag_type( upper_diag_begin( index ) );
        }

        const_reverse_upper_diag_type
        upper_diag_rbegin( const size_type index = 0 ) const
        {
            return const_reverse_upper_diag_type( upper_diag_end( index ) );
        }

        const_reverse_upper_diag_type
        upper_diag_rend( const size_type index = 0 ) const
        {
            return const_reverse_upper_diag_type( upper_diag_begin( index ) );
        }

        const_reverse_upper_diag_type
        upper_diag_crbegin( const size_type index = 0 ) const
        {
            return const_reverse_upper_diag_type( upper_diag_end( index ) );
        }

        const_reverse_upper_diag_type
        upper_diag_crend( const size_type index = 0 ) const
        {
            return const_reverse_upper_diag_type( upper_diag_begin( index ) );
        }


        diag_type lower_diag_begin( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }

        diag_type lower_diag_end( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_diag_begin( index ) + depth;
        }

        const_diag_type lower_diag_begin( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }

        const_diag_type lower_diag_end( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_diag_begin( index ) + depth;
        }

        const_diag_type lower_diag_cbegin( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return const_diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }

        const_diag_type lower_diag_cend( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_diag_begin( index ) + depth;
        }

        reverse_lower_diag_type lower_diag_rbegin( const size_type index = 0 )
        {
            return reverse_lower_diag_type( lower_diag_end( index ) );
        }

        reverse_lower_diag_type lower_diag_rend( const size_type index = 0 )
        {
            return reverse_lower_diag_type( lower_diag_begin( index ) );
        }

        const_reverse_lower_diag_type lower_diag_rbegin( const size_type index = 0 ) const
        {
            return const_reverse_lower_diag_type( lower_diag_end( index ) );
        }

        const_reverse_lower_diag_type lower_diag_rend( const size_type index = 0 ) const
        {
            return const_reverse_lower_diag_type( lower_diag_begin( index ) );
        }

        const_reverse_lower_diag_type lower_diag_crbegin( const size_type index = 0 ) const
        {
            return const_reverse_lower_diag_type( lower_diag_end( index ) );
        }

        const_reverse_lower_diag_type lower_diag_crend( const size_type index = 0 ) const
        {
            return const_reverse_lower_diag_type( lower_diag_begin( index ) );
        }

        diag_type diag_begin( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_diag_begin( index );
            }

            return lower_diag_begin( -index );
        }

        diag_type diag_end( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_diag_end( index );
            }

            return lower_diag_end( -index );
        }

        const_diag_type diag_begin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_begin( index );
            }

            return lower_diag_begin( -index );
        }

        const_diag_type diag_end( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_end( index );
            }

            return lower_diag_end( -index );
        }

        const_diag_type diag_cbegin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_cbegin( index );
            }

            return lower_diag_cbegin( -index );
        }

        const_diag_type diag_cend( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_cend( index );
            }

            return lower_diag_cend( -index );
        }


        reverse_diag_type diag_rbegin( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_diag_rbegin( index );
            }

            return lower_diag_rbegin( -index );
        }

        reverse_diag_type diag_rend( const difference_type index = 0 )
        {
            if ( index > 0 )
            {
                return upper_diag_rend( index );
            }

            return lower_diag_rend( -index );
        }

        const_reverse_diag_type diag_rbegin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_rbegin( index );
            }

            return lower_diag_rbegin( -index );
        }

        const_reverse_diag_type diag_rend( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_rend( index );
            }

            return lower_diag_rend( -index );
        }

        const_reverse_diag_type diag_crbegin( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_crbegin( index );
            }

            return lower_diag_crbegin( -index );
        }

        const_reverse_diag_type diag_crend( const difference_type index = 0 ) const
        {
            if ( index > 0 )
            {
                return upper_diag_crend( index );
            }

            return lower_diag_crend( -index );
        }

    };//struct crtp_diag_iterator

}

#endif//_DIAG_ITERATOR_HPP_INCLUDED_DFDS9HI3HILJUASFDKJVKJNDASJKHFSDIOHSF98Y4FSD

