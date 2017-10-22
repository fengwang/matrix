#ifndef MVALUE_MATRIX_MINUS_EXPRESSION_HPP_INCLUDED_SDPIONAS3HI9AFDKLJ4098AFDLKD
#define MVALUE_MATRIX_MINUS_EXPRESSION_HPP_INCLUDED_SDPIONAS3HI9AFDKLJ4098AFDLKD

#include "./matrix_expression.hpp"

namespace f
{
    template< typename Expression, typename Type >
    struct crtp_value_matrix_minus_expression : matrix_expression< crtp_value_matrix_minus_expression< Expression, Type >>
    {
        typedef typename Expression::value_type     value_type;
        typedef typename Expression::size_type      size_type;

        crtp_value_matrix_minus_expression( Expression const& expression_, Type const& value_ ) : expression( expression_ ), value( value_ ) {}

        Expression const&   expression;
        Type const&         value;

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
            return value - expression( r, c );
        }
    };

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_value_matrix_minus_operator
    {
        template< typename Right_Expression >
        friend crtp_value_matrix_minus_expression< Right_Expression, Type > const operator - ( const Type& l_value, const matrix_expression<Right_Expression>& r_expression )
        {
            return crtp_value_matrix_minus_expression< Right_Expression, Type>( r_expression, l_value );
        }
    };

}

#endif//_VALUE_MATRIX_MINUS_EXPRESSION_HPP_INCLUDED_SDPIONAS3HI9AFDKLJ4098AFDLKD

