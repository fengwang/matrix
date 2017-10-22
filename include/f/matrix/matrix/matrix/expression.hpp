#ifndef MEXPRESSION_HPP_INCLUDED_SDFPOISADF0I3098YUAFDSLJKSFDNJMV98Y4HELKJSFDVJK
#define MEXPRESSION_HPP_INCLUDED_SDFPOISADF0I3098YUAFDSLJKSFDNJMV98Y4HELKJSFDVJK

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_expression
    {
        typedef Matrix                                        zen_type;
        typedef crtp_typedef<Type, Allocator>        type_proxy_type;
        typedef typename type_proxy_type::value_type          value_type;
        typedef typename type_proxy_type::size_type           size_type;

        value_type& operator()( const size_type r, const size_type c )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return *( zen.row_begin( r ) + c );
        }

        value_type operator()( const size_type r, const size_type c ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return *( zen.row_cbegin( r ) + c );
        }

        operator zen_type& ()
        {
            return static_cast<zen_type&>( *this );
        }

        operator zen_type const& () const
        {
            return static_cast<zen_type const&>( *this );
        }

    };//struct

}

#endif//_EXPRESSION_HPP_INCLUDED_SDFPOISADF0I3098YUAFDSLJKSFDNJMV98Y4HELKJSFDVJK

