#ifndef MMATRIX_VALUE_PLUS_EXPRESSION_HPP_INCLUDED_DSFPOAS984YKFDLJV98GH3IUHFADI
#define MMATRIX_VALUE_PLUS_EXPRESSION_HPP_INCLUDED_DSFPOAS984YKFDLJV98GH3IUHFADI

#include "./expression.hpp"

namespace f
{
    template<typename Matrix_Expression, typename Value_Expression, typename Type, typename Allocator>
    struct crtp_matrix_value_plus_expression : public crtp_expression< crtp_matrix_value_plus_expression<Matrix_Expression, Value_Expression, Type, Allocator>, Type, Allocator >
    {
        typedef crtp_typedef<Type, Allocator>        type_proxy_type;
        typedef typename type_proxy_type::value_type          value_type;
        typedef typename type_proxy_type::size_type           size_type;

        Matrix_Expression const& matrix;
        Value_Expression const& value;

        crtp_matrix_value_plus_expression( const Matrix_Expression& m, const Value_Expression& v ) : matrix( m ), value( v ) {}

        value_type operator()( const size_type r, const size_type c ) const
        {
            return matrix( r, c ) + value( r, c );
        }

    };//struct

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_matrix_value_plus_operator
    {
        template<typename Matrix_Type, typename Value_Type>
        friend crtp_matrix_value_plus_expression const  operator + ( const Matrix_Type& m, const Value_Type& v )
    };

}

#endif//_MATRIX_VALUE_PLUS_EXPRESSION_HPP_INCLUDED_DSFPOAS984YKFDLJV98GH3IUHFADI

