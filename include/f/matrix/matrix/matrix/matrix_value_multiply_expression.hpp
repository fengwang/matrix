#ifndef MMATRIX_VALUE_MULTIPLY_EXPRESSION_HPP_INCLUDED_FDSPOIJASFD14098UFKJVN98F
#define MMATRIX_VALUE_MULTIPLY_EXPRESSION_HPP_INCLUDED_FDSPOIJASFD14098UFKJVN98F

#include "./matrix_expression.hpp"

namespace f
{
    template<typename Expression, typename Type>
    struct crtp_matrix_value_multiply_expression
        : matrix_expression<crtp_matrix_value_multiply_expression<Expression, Type>>
    {
        typedef typename Expression::value_type     value_type;
        typedef typename Expression::size_type      size_type;

        crtp_matrix_value_multiply_expression( Expression const& expression_, Type const& value_ ) : expression( expression_ ), value( value_ ) {}

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
            return expression( r, c ) * value;
        }
    };

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_matrix_value_multiply_operator
    {
        template< typename Left_Expression >
        friend crtp_matrix_value_multiply_expression<Left_Expression, Type> const
        operator * ( const matrix_expression<Left_Expression>& l_expression, const Type& r_value )
        {
            return crtp_matrix_value_multiply_expression<Left_Expression, Type>( l_expression, r_value );
        }

        template< typename Right_Expression >
        friend crtp_matrix_value_multiply_expression<Right_Expression, Type> const
        operator * ( const Type& l_value, const matrix_expression<Right_Expression>& r_expression )
        {
            return crtp_matrix_value_multiply_expression<Right_Expression, Type>( r_expression, l_value );
        }
    };

}

#endif//_MATRIX_VALUE_MULTIPLY_EXPRESSION_HPP_INCLUDED_FDSPOIJASFD14098UFKJVN98F

