#ifndef MPLUS_EXPRESSION_HPP_INCLUDED_SDFPOIJ4098YHFASDKLJVCN908Y4ELKSJFDNVBJDFS
#define MPLUS_EXPRESSION_HPP_INCLUDED_SDFPOIJ4098YHFASDKLJVCN908Y4ELKSJFDNVBJDFS

#include "./expression.hpp"

namespace f
{
    template<typename L_Expression, typename R_Expression, typename Type, typename Allocator>
    struct crtp_plus_expression : public crtp_expression< crtp_plus_expression<L_Expression, R_Expression, Type, Allocator>, Type, Allocator >
    {
        typedef crtp_typedef<Type, Allocator>        type_proxy_type;
        typedef typename type_proxy_type::value_type          value_type;
        typedef typename type_proxy_type::size_type           size_type;

        L_Expression const& lhs;
        R_Expression const& rhs;

        crtp_plus_expression( const L_Expression& l, const R_Expression& r ) : lhs( l ), rhs( r ) {}

        value_type operator()( const size_type r, const size_type c ) const
        {
            return lhs( r, c ) + rhs( r, c );
        }

    };//struct

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_plus_operator
    {
        template<typename L_Expression, typename R_Expression>
        friend crtp_plus_expression<L_Expression, R_Expression, Type, Allocator>  const operator + ( const L_Expression& l, const R_Expression& r )
        {
            return  crtp_plus_expression<L_Expression, R_Expression, Type, Allocator>( l, r );
        }
    };

}

#endif//_PLUS_EXPRESSION_HPP_INCLUDED_SDFPOIJ4098YHFASDKLJVCN908Y4ELKSJFDNVBJDFS

