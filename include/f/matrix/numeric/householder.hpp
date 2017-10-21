#ifndef MHOUSEHOLDER_HPP_INCLUDED_SOIJ43ULDSAKJVCKLJAKLJALKJCSDFUI4UHSDKJAF4HTKJFLAJKSFDLKJFDVNAJKSD
#define MHOUSEHOLDER_HPP_INCLUDED_SOIJ43ULDSAKJVCKLJAKLJALKJCSDFUI4UHSDKJAF4HTKJFLAJKSFDLKJFDVNAJKSD

namespace f
{

    // HouseHolder Transform
    // reduct a real, symmetric matrix A to its tridiagonal form D, using Householder matrix Q
    // Input:
    //          A       --      the symmetric real matrix
    //          Q       --      the transformation matrix
    //          D       --      the tridigonal form reduced from matrix A
    //Comment:
    //          Q is othogonal and symmetirc,
    //              Q * Q^T = I
    //          D is tridigonal
    //          and
    //          A = Q D Q
    //
    template< typename Matrix1, typename Matrix2, typename Matrix3 >
    void householder( const Matrix1& A, Matrix2& Q, Matrix3& D )
    {
        typedef Matrix1                             matrix_type;
        typedef typename matrix_type::value_type    value_type;
        typedef typename matrix_type::size_type     size_type;
        assert( A.row() == A.col() );
        size_type const n = A.row();
        value_type const zero = value_type( 0 );
        value_type const one = value_type( 1 );
        value_type const two = value_type( 2 );
        Matrix1 const I = eye( n, A );
        Q = eye<value_type>( n );
        D = A;
        Matrix1 x( n, 1 );
        Matrix1 P( n, n );

        if ( n < 3 )
        {
            return;
        }

        // for step i,
        // 1) vector x is set to be the ith column of matrix A'(or D), without first n+1 th elements,
        //      eg.  x = {0,0,A[2][1], A[3][1], ..., A[3][n]}
        // 2) calculate the length of vector x to get delta
        // 3) add or sub delta to x[i+1] (to generate vector u)
        // 4) calcuate the common factor H, which is |u|/2
        // 5) calculate matrix P, which is I - u u^T / H
        // 6) D(or A) = P * D * P
        // 7) Q = Q*P;
        for ( size_type i = 0; i != n - 1; ++i )
        {
            //1)
            std::fill( x.begin(), x.begin() + i + 1, zero );
            std::copy( D.col_begin( i ) + i + 1, D.col_end( i ), x.begin() + i + 1 );
            //2)
            value_type const delta = std::sqrt( std::inner_product( x.begin(), x.end(), x.begin(), zero ) );

            if ( zero == delta ) continue; //fix for zero lines

            //3)
            if ( x[i + 1][0] > zero ) x[i + 1][0] += delta;
            else x[i + 1][0] -= delta;

            //4)
            value_type const H = std::inner_product( x.begin(), x.end(), x.begin(), zero ) / two;
            //5)
            P = I - ( x * x.transpose() ) / H;

            if ( zero == H ) continue; //fix for zero lines

            //6)
            //D = P * D * P;
            //optimized version of D - P*D*P;
            matrix_type const p = D * x / H;
            value_type  const k = std::inner_product( x.begin(), x.end(), p.begin(), zero ) / ( H + H );
            matrix_type const q = p - k * x;
            D -= q * x.transpose() + x * q.transpose();
            //7)
            //!!Q *= P;
            //Optimized version of Q=Q*P
            Q -= Q * x * x.transpose() / H;
        }
    }//function householder

}//namespace f

#endif//_HOUSEHOLDER_HPP_INCLUDED_SOIJ43ULDSAKJVCKLJAKLJALKJCSDFUI4UHSDKJAF4HTKJFLAJKSFDLKJFDVNAJKSD

