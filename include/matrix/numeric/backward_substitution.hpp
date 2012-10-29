#ifndef _BACKWARD_SUBSTITUTION_HPP_INCLUDED_SFDON4OIJAFOJIHTUHLSAFKJHALFJKHASFIOUHASFLKJHNSAFKJHSFDJK
#define _BACKWARD_SUBSTITUTION_HPP_INCLUDED_SFDON4OIJAFOJIHTUHLSAFKJHALFJKHASFIOUHASFLKJHNSAFKJHSFDJK

#include <matrix/matrix.hpp>
#include <matrix/functional.hpp>

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <cmath>

namespace feng
{
    //solving   Ax = b      -- A is an upper triangular matrix
    //
    //return
    //          0    --     success
    //          1    --     failed
    template<typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2, typename T3, std::size_t D3, typename A3>
    int
    backward_substitution( const matrix<T1,D1,A1>&           A,  //A[n][n] is an upper triangular matrix
                           matrix<T2,D2,A2>&                 x,  //x[n]  
                           const matrix<T3,D3,A3>&           b ) //b[n]
    {
        typedef matrix<T1,D1,A1>                 matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type  size_type;

        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        size_type const n = A.row();

        //initialize x
        x.resize( n, 1 );
        std::fill( x.begin(), x.end(), value_type(0) );

        for ( size_type i = 0; i != n; ++i )
        {
            size_type const r = n-1-i;
            value_type sum = std::inner_product( x.rbegin(), x.rbegin()+i, A.row_rbegin(r), value_type(0) );
            x[r][0] = (b[r][0]-sum) / A[r][r];
            if ( std::isinf(x[r][0]) || std::isnan(x[r][0]) ) return 1;
        }

        return 0;
    }

}//namespace feng

#endif//_BACKWARD_SUBSTITUTION_HPP_INCLUDED_SFDON4OIJAFOJIHTUHLSAFKJHALFJKHASFIOUHASFLKJHNSAFKJHSFDJK

