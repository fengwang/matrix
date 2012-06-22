#ifndef _HOUSEHOLDER_HPP_INCLUDED_SOIJ43ULDSAKJVCKLJAKLJALKJCSDFUI4UHSDKJAF4HTKJFLAJKSFDLKJFDVNAJKSD
#define _HOUSEHOLDER_HPP_INCLUDED_SOIJ43ULDSAKJVCKLJAKLJALKJCSDFUI4UHSDKJAF4HTKJFLAJKSFDLKJFDVNAJKSD

#include <matrix/matrix.hpp>

#include <cmath>
#include <cassert>
#include <valarray>

namespace feng
{

    // A = Q D Q^T
    template< typename Matrix1, typename Matrix2, typename Matrix3 >
    void householder( const Matrix1& A, Matrix2& Q, Matrix3& D )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type  size_type;

        assert( A.row() == A.col() );
        size_type const n = A.row();

        Q = eye<value_type>(n);
        D = A;

        if ( n < 3 ) { return; }

#if 0
        Matrix1 v( n, 1 );
        Matrix1 P( n, n );

        for ( size_type k = 0; k != n; ++k )
        {
            auto const alpha = std::copysign( std::sqrt( std::inner_product(D.col_begin(k)+k+1, D.col_end(k), D.col_begin(k)+k+1, value_type()) ), D[k+1][k] );

            auto const r = std::sqrt(value_type(0.5) * ( alpha*(alpha + D[k+1][k]) ));
            if ( value_type(0) == r ) continue;

            std::fill( v.begin(), v.begin()+k+1, value_type() );
            std::copy( D.col_begin(k)+k+1, D.col_end(k), v.begin()+k+1 );
            v[k+1][0] += alpha;
            v /= r+r;

            P = value_type(1) - value_type(2) * v * v.transpose();

            D= P * D * P;

            Q *= P;
        }
#endif
#if 1
        Matrix1 u( n, 1 );
        Matrix1 p( n, 1 );
        Matrix1 q( n, 1 );

        for ( size_type m = 0; m != n-2; ++m )
        {
            auto const i = n - m - 1;
            
            auto const delta = std::inner_product( D.row_begin(i), D.row_begin(i)+i, D.row_begin(i), value_type() );
            
            std::copy( D.row_begin(i), D.row_begin(i)+i, u.begin() );
            std::fill( u.begin()+i, u.end(), value_type() );
            if ( D[i][i-1] < value_type() ) u[i-1][0] += delta;
            else u[i-1][0] -= delta;

            auto const H = value_type(0.5) * std::inner_product( u.begin(), u.end(), u.begin(), value_type() ); //this line can be optimized

            p = D * u / H;

            auto const k = std::inner_product( u.begin(), u.end(), p.begin(), value_type() ) / //can be optimized
                           ( value_type(0.5) * H );

            q = p - k * u;

            D -= q * u.transpose() + u * q.transpose();

            Q -= Q * u * u.transpose() / H;
        }
#endif



        

    }//function householder

}//namespace feng

#endif//_HOUSEHOLDER_HPP_INCLUDED_SOIJ43ULDSAKJVCKLJAKLJALKJCSDFUI4UHSDKJAF4HTKJFLAJKSFDLKJFDVNAJKSD

