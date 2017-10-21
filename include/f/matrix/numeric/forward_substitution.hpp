#ifndef MFORWARD_SUBSTITUTION_HPP_INCLUDED_SFDON4OIJAFOJIHTUHLSAFKJHALFJKHASFIOUHASFLKJHNSAFKJHSFDJK
#define MFORWARD_SUBSTITUTION_HPP_INCLUDED_SFDON4OIJAFOJIHTUHLSAFKJHALFJKHASFIOUHASFLKJHNSAFKJHSFDJK

namespace f
{
    //solving   Ax = b      -- A is a lower triangular matrix
    //
    //return
    //          0    --     success
    //          1    --     failed
    template<typename T1, typename A1, typename T2, typename A2, typename T3, typename A3>
    int
    forward_substitution( const matrix<T1, A1>&           A, //A[n][n] is a lower triangular matrix
                          matrix<T2, A2>&                 x, //x[n][1]
                          const matrix<T3, A3>&           b ) //b[n][1]
    {
        typedef matrix<T1, A1>                 matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type  size_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        size_type const n = A.row();
        //initialize x
        x.resize( n, 1 );
        std::fill( x.begin(), x.end(), value_type( 0 ) );

        for ( size_type i = 0; i != n; ++i )
        {
            value_type sum = std::inner_product( x.begin(), x.begin() + i, A.row_begin( i ), value_type( 0 ) );
            x[i][0] = ( b[i][0] - sum ) / A[i][i];

            if ( std::isinf( x[i][0] ) || std::isnan( x[i][0] ) ) return 1;
        }

        return 0;
    }

}//namespace f

#endif//_FORWARD_SUBSTITUTION_HPP_INCLUDED_SFDON4OIJAFOJIHTUHLSAFKJHALFJKHASFIOUHASFLKJHNSAFKJHSFDJK

