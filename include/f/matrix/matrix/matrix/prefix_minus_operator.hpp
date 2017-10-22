#ifndef MPREFIX_MINUX_OPERATOR_HPP_INCLUDED_SDPI9J4LKSFALKJ4H98AHSFKLJH4E98AYHFF
#define MPREFIX_MINUX_OPERATOR_HPP_INCLUDED_SDPI9J4LKSFALKJ4H98AHSFKLJH4E98AYHFF

#include "./matrix_expression.hpp"

namespace f
{
#if 0
    template< typename Expression >
    struct crtp_prefix_minus_expression : matrix_expression< crtp_prefix_minus_expression<Expression>>
    {
        typedef typename Expression::value_type     value_type;
        typedef typename Expression::size_type      size_type;

        crtp_prefix_minus_expression( Expression const& expression_ ) : expression( expression_ ) {}

        Expression const&   expression;

        size_type row() const
        {
            return expression.row();
        }

        size_type col() const
        {
            return expression.col();
        }

        value_type operator()( const size_type r, const size_type c ) const
        {
            return - expression( r, c );
        }
    };

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_prefix_minus_operator
    {
        template< typename Expression >
        friend crtp_prefix_minus_expression<Expression> const operator - ( const matrix_expression<Expression>& expression )
        {
            return crtp_prefix_minus_expression<Expression>( expression );
        }
    };

#endif

    template< typename Matrix, typename Type, typename Allocator >
    struct crtp_prefix_minus
    {
        typedef Matrix                                      zen_type;
        typedef crtp_typedef<Type, Allocator>               type_proxy_type;
        typedef typename type_proxy_type::size_type         size_type;
        typedef typename type_proxy_type::value_type        value_type;

        const zen_type operator -() const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            zen_type ans( zen );
            std::transform( ans.begin(), ans.end(), ans.begin(), []( value_type x )
            {
                return -x;
            } );
            return ans;
        }
    };

}

#endif//_PREFIX_MINUX_OPERATOR_HPP_INCLUDED_SDPI9J4LKSFALKJ4H98AHSFKLJH4E98AYHFF

