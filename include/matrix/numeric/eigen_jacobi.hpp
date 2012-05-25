#ifndef _EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU
#define _EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

#include <matrix/matrix.hpp>
#include <matrix/sparse_matrix.hpp>
#include <matrix/misc/is_symmetric.hpp>

#include <cassert>
#include <cstddef>
#include <cmath>

namespace feng
{

    template< typename Matrix1, typename Matrix2, typename Otor, typename T = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, const T eps = double(1.0e-5) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        assert( A.row() == A.col() );
        assert( is_symmetric( A ) );
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type(1);
        auto const zero = value_type(0);
        
        // @V = diag{1, 1, ..., 1}
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );

        //the sparse matrix
        sparse_matrix<value_type> P(n,n);
        for (size_type i = 0; i != n; ++i )
            P(i,i) = one;

        for ( size_type i = 0; i != size_type(-1); ++i )
        {

            // @find max non-diag value in A
            size_type p = 0;
            size_type q = 1;
            value_type current_max = std::abs(a[p][q]);

            for ( size_type ip = 0; ip != n; ++ip )
                for ( size_type iq = ip+1; iq != n; ++iq )
                {
                    auto const tmp = std::abs(a[ip][iq]);
                    if ( current_max > tmp ) continue;
                    current_max = tmp;
                    p           = ip;
                    q           = iq;
                }

            // @if all non-diag value small, then break iteration with success 
            if ( current_max < eps ) 
            {
                std::copy( a.diag_begin(), a.diag_end(), o );
                return i;
            }

            // @calc c and s
            auto const theta = ( a[q][q] - a[p][p] ) / ( a[p][q] + a[p][q] );
            auto const t     = std::copysign( one / ( std::abs(theta) + std::sqrt(theta*theta + one ) ), theta );
            auto const c     = one / std::sqrt(t*t + one);
            auto const s     = t * c;

            // @generate 4 special elements for sparse matrix P
            P(p,p) =  c;
            P(q,q) =  c;
            P(p,q) =  s;
            P(q,p) = -s;

            // @Jacobi iteration
            a = P.transpose() * a * P;
            V = V * P;

            // @recover sparse matrix for next iteration
            P(p,p) = one;
            P(q,q) = one;
            P(p,q) = zero;
            P(q,p) = zero;

        }//end for
        
        // @just to kill warnings, should never reach here
        return size_type(-1);
    }//eigen_jacobi

}//namespace feng

#endif//_EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

