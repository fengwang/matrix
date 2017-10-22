#ifndef MPREFIX_PLUS_OPERATOR_HPP_INCLUDED_SDPON3POIHJAFSDLKJ49HAFDKLJNVIAUFH43F
#define MPREFIX_PLUS_OPERATOR_HPP_INCLUDED_SDPON3POIHJAFSDLKJ49HAFDKLJNVIAUFH43F

namespace f
{

#if 0
    template< typename Expression >
    struct crtp_prefix_plus_expression : matrix_expression< crtp_prefix_plus_expression<Expression>>
    {
        typedef typename Expression::value_type     value_type;
        typedef typename Expression::size_type      size_type;

        crtp_prefix_plus_expression( Expression const& expression_ ) : expression( expression_ ) {}

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
            return expression( r, c );
        }
    };

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_prefix_plus_operator
    {
        template< typename Expression >
        friend crtp_prefix_plus_expression<Expression> const operator + ( const matrix_expression<Expression>& expression )
        {
            return crtp_prefix_plus_expression<Expression>( expression );
        }
    };
#endif

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_prefix_plus
    {
        typedef Matrix                                              zen_type;
        typedef crtp_typedef<Type, Allocator>              type_proxy_type;
        typedef typename type_proxy_type::value_type                value_type;
        typedef typename type_proxy_type::size_type                 size_type;

        const zen_type operator +() const
        {
            return static_cast<zen_type const&>( *this );
        }

    };

}

#endif//_PREFIX_PLUS_OPERATOR_HPP_INCLUDED_SDPON3POIHJAFSDLKJ49HAFDKLJNVIAUFH43F

