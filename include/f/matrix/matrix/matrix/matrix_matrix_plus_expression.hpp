#ifndef MMATRIX_MATRIX_PLUS_EXPRESSION_HPP_INCLUDED_DAOJI3LJKFDNAJAFDSFDAFSD43FD
#define MMATRIX_MATRIX_PLUS_EXPRESSION_HPP_INCLUDED_DAOJI3LJKFDNAJAFDSFDAFSD43FD

namespace f
{
    template< typename Left_Expression, typename Right_Expression>
    struct crtp_matrix_matrix_plus_expression : matrix_expression< crtp_matrix_matrix_plus_expression< Left_Expression, Right_Expression >>
    {
        typedef typename Left_Expression::value_type     value_type;
        typedef typename Left_Expression::size_type      size_type;

        crtp_matrix_matrix_plus_expression( Left_Expression const& l_expression_, Right_Expression const& r_expression_ ) : l_expression( l_expression_ ), r_expression( r_expression_ ) {}

        Left_Expression const&      l_expression;
        Right_Expression const&     r_expression;

        size_type row() const
        {
            assert( l_expression.row() == r_expression.row() );
            return l_expression.row();
        }

        size_type col() const
        {
            assert( l_expression.col() == r_expression.col() );
            return l_expression.col();
        }

        value_type operator()( const size_type r, const size_type c ) const
        {
            return l_expression( r, c ) + r_expression( r, c );
        }
    };

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_matrix_matrix_plus_operator
    {
        template< typename Left_Expression, typename Right_Expression >
        friend crtp_matrix_matrix_plus_expression<Left_Expression, Right_Expression> const  operator + ( const matrix_expression<Left_Expression>& l_expression, const matrix_expression<Right_Expression>& r_expression )
        {
            return crtp_matrix_matrix_plus_expression<Left_Expression, Right_Expression>( l_expression, r_expression );
        }
    };
}

#endif//_MATRIX_MATRIX_PLUS_EXPRESSION_HPP_INCLUDED_DAOJI3LJKFDNAJAFDSFDAFSD43FD

