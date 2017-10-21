#ifndef MMATRIX_ABS_EXPRESSION_HPP_INCLUDED_SDFPOIJA3OIASFLDK3PIAFDLK4JKFDAKJSFN
#define MMATRIX_ABS_EXPRESSION_HPP_INCLUDED_SDFPOIJA3OIASFLDK3PIAFDLK4JKFDAKJSFN

#include <f/matrix/impl/crtp/matrix_expression.hpp>

#include <cmath>
#include <cassert>

namespace f
{
    //needs specialization for complex?
    template< typename Expression>
    struct crtp_matrix_abs_expression : matrix_expression< crtp_matrix_abs_expression< Expression > >
    {
        typedef typename Expression::value_type     value_type;
        typedef typename Expression::size_type      size_type;

        crtp_matrix_abs_expression( Expression const& expression_ ) : expression(expression_) {}

        Expression const&      expression;

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
            return std::abs( expression( r, c ) );
        }
    };

    template< typename Expression >
    crtp_matrix_abs_expression<Expression> const abs( const matrix_expression<Expression>& expression )
    {
        return crtp_matrix_abs_expression<Expression>( expression );
    }

}

#endif//_MATRIX_ABS_EXPRESSION_HPP_INCLUDED_SDFPOIJA3OIASFLDK3PIAFDLK4JKFDAKJSFN

