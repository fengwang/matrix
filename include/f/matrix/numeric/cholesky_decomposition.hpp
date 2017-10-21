#ifndef MCHOLDESKY_DECOMPOSITON_HPP_INCLUDED_FSDOIU4098UFS098U4098USF098409SF09UVX098UVCX098XV098XV09XV809XV8X0V9XV09C0X9
#define MCHOLDESKY_DECOMPOSITON_HPP_INCLUDED_FSDOIU4098UFS098U4098USF098409SF09UVX098UVCX098XV098XV09XV809XV8X0V9XV09C0X9

namespace f
{
    // M = A A^T
    template<typename Matrix1, typename Matrix2>
    void
    cholesky_decomposition( const Matrix1& m, Matrix2& a )
    {
        typedef typename Matrix1::value_type value_type;
        assert( m.row() == m.col() );
        a = m;
        const std::size_t n = m.row();

        for ( std::size_t i = 0; i < n; ++i )
            for ( std::size_t j = i; j < n; ++j )
            {
                const value_type sum = a[i][j] - std::inner_product( a.row_begin( i ), a.row_begin( i ) + i, a.row_begin( j ), value_type( 0 ) );
                a[j][i] = ( i == j ) ? std::sqrt( sum ) : ( sum / a[i][i] );
            }

        for ( std::size_t i = 1; i < n; ++i )
            std::fill( a.upper_diag_begin( i ), a.upper_diag_end( i ), value_type() );
    }

    /*
        struct cholesky_decomposition
        {
            template< typename Matrix_Type >
            const Matrix_Type
            operator()( const Matrix_Type& m ) const
            {
                typedef typename Matrix_Type::value_type value_type;
                assert( m.row() == m.col() );
                const std::size_t n = m.row();
                Matrix_Type ans( m );

                for ( std::size_t i = 0; i < n; ++i )
                    for ( std::size_t j = i; j < n; ++j )
                    {
                        const value_type sum = ans[i][j] - std::inner_product( ans.row_begin( i ), ans.row_begin( i ) + i, ans.row_begin( j ), value_type( 0 ) );
                        ans[j][i] = ( i == j ) ? std::sqrt( sum ) : ( sum / ans[i][i] );
                    }
                for ( std::size_t i = 1; i < n; ++i )
                    std::fill( ans.upper_diag_begin(i), ans.upper_diag_end(i), value_type() );

                return ans;
            }
        };
    */

}//namespace f

#endif//_CHOLDESKY_DECOMPOSITON_HPP_INCLUDED_FSDOIU4098UFS098U4098USF098409SF09UVX098UVCX098XV098XV09XV809XV8X0V9XV09C0X9

