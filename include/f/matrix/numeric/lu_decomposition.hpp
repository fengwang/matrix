#ifndef MLU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81
#define MLU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81

namespace f
{

    // LU decomposition
    // return
    //          0       --       success
    //          1       --       failed
    template<typename T1, typename A1,
             typename T2, typename A2,
             typename T3, typename A3>
    int lu_decomposition( const matrix<T1, A1>& A, matrix<T2, A2>& L, matrix<T3, A3>& U )
    {
        typedef T1 value_type;
        assert( A.row() == A.col() );
        const std::size_t n = A.row();
        L.resize( n, n );
        U.resize( n, n );
        std::fill( L.diag_begin(), L.diag_end(), value_type( 1 ) );

        for ( std::size_t j = 0; j < n; ++j )
        {
            for ( std::size_t i = 0; i < j + 1; ++i )
            {
                U[i][j] = A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + i, U.col_begin( j ), value_type() );
            }

            for ( std::size_t i = j + 1; i < n; ++i )
            {
                L[i][j] = ( A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + j, U.col_begin( j ), value_type() ) ) / U[j][j];

                if ( std::isinf( L[i][j] ) || std::isnan( L[i][j] ) ) return 1;
            }
        }

        return 0;
    }

}//namespace f

#endif//_LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81

