#ifndef _EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU
#define _EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

#include <matrix/matrix.hpp>
#include <matrix/sparse_matrix.hpp>
#include <matrix/misc/is_symmetric.hpp>

#include <cassert>
#include <cstddef>
#include <cmath>
#include <vector>
#include <valarray>

namespace feng
{
    /* Input:
     *          A       Symmetric dense matrix
     *          V       Eigen Vectors
     *          Lambda  Eigen Values
     * Function:
     *          calculate the eigen vectors V and eigen values lambda using Jacobi method
     * Returns:
     *          the iterations used
     */
    template<typename Matrix1, typename Matrix2, typename T = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector<T>& Lambda, const T eps = T(1.0e-5) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin() );
    }

    template<typename Matrix1, typename Matrix2, typename T = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray<T>& Lambda, const T eps = T(1.0e-5) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin() );
    }

    // A = V * diag{lambda...} * V^T
    template< typename Matrix1, typename Matrix2, typename Otor, typename T = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, const T eps = T(1.0e-5) )
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

            // a quick explaination here:
            // the sparse matrix P is 
            // [ 1  0  0  0 ....  0  0  0  0 ]
            // [ 0  1  0  0 ....  0  0  0  0 ]
            // [ 0  0  c  0 ....  0  s  0  0 ]  <- pth row
            // [ 0  0  0  1 ....  0  0  0  0 ]
            // [ .  .  .  . ....  .  .  .  . ]
            // [ 0  0  0  0 ....  1  0  0  0 ]
            // [ 0  0 -s  0 ....  0  c  0  0 ]  <- qth row
            // [ 0  0  0  0 ....  0  0  1  0 ]
            // [ 0  0  0  0 ....  0  0  0  1 ]
            //        pth           qth
            //
            // is reduced to P = I + P_, where I is unit matrix and P_ is 
            // [ 0  0  0  0 ....  0  0  0  0 ]
            // [ 0  0  0  0 ....  0  0  0  0 ]
            // [ 0  0 c-1 0 ....  0  s  0  0 ]  <- pth row
            // [ 0  0  0  0 ....  0  0  0  0 ]
            // [ .  .  .  . ....  .  .  .  . ]
            // [ 0  0  0  0 ....  0  0  0  0 ]
            // [ 0  0 -s  0 ....  0 c-1 0  0 ]  <- qth row
            // [ 0  0  0  0 ....  0  0  0  0 ]
            // [ 0  0  0  0 ....  0  0  0  0 ]
            //        pth           qth
            //
            // this way, the iteratiron of V, V*P = V * (I+P_) = V + V*P_, i.e. V += V*P_
            // and the iteration of a, a = P^T * a * P = (I+P_^T) * a * (I+P_) 
            //                           = ( a + P_^T * a ) * (I + P_)
            //                       i.e., a += P_^T * a; a += a * P_;
            sparse_matrix<value_type> P_(n,n);
            P_(p,p) = c - one;
            P_(p,q) = s;
            P_(q,q) = c - one;
            P_(q,p) = -s;

            a += P_.transpose() * a;
            a += a * P_;
            V += V * P_;

        }//end for
        
        // @just to kill warnings, should never reach here
        return size_type(-1);
    }//eigen_jacobi

}//namespace feng

#endif//_EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

