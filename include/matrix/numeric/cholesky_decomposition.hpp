#ifndef _CHOLDESKY_DECOMPOSITON_HPP_INCLUDED_FSDOIU4098UFS098U4098USF098409SF09UVX098UVCX098XV098XV09XV809XV8X0V9XV09C0X9
#define _CHOLDESKY_DECOMPOSITON_HPP_INCLUDED_FSDOIU4098UFS098U4098USF098409SF09UVX098UVCX098XV098XV09XV809XV8X0V9XV09C0X9

#include <matrix/matrix.hpp>

#include <cstddef>
#include <cassert>
#include <cmath>
#include <numeric>
#include <algorithm>

namespace feng
{
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
}//namespace feng

#endif//_CHOLDESKY_DECOMPOSITON_HPP_INCLUDED_FSDOIU4098UFS098U4098USF098409SF09UVX098UVCX098XV098XV09XV809XV8X0V9XV09C0X9

