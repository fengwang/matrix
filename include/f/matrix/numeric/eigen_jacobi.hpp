#ifndef MEIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU
#define MEIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

#include "./math.hpp"

#include <cassert>
#include <cstddef>
#include <cmath>
#include <vector>
#include <valarray>

namespace f
{
    namespace eigen_jacobi_private
    {
        template< typename Matrix >
        typename Matrix::value_type norm( const Matrix& A )
        {
            typedef typename Matrix::value_type value_type;
            //typedef typename Matrix::size_type  size_type;
            auto A_ = abs( A );
            std::fill( A_.diag_begin(), A_.diag_end(), value_type( 0 ) ); //diag element set to 0
            auto const max_elem = *( std::max_element( A_.cbegin(), A_.cend() ) ); //find the max element

            if ( value_type( 0 ) == max_elem ) return value_type( 0 ); //return 0 if all elements are 0

            A_ /= max_elem; //in case of round-off error
            auto const sum = std::inner_product( A_.cbegin(), A_.cend(), A_.cbegin(), value_type( 0 ) ); //\sum A_{i,j}^2
            return std::sqrt( sum ) * max_elem;
        }

        template< typename Matrix1, typename Matrix2 >
        void rotate( Matrix1& A, Matrix2& V, const std::size_t p, const std::size_t q )
        {
            typedef typename Matrix1::value_type value_type;
            auto const one  = value_type( 1 );
            auto const n    = A.row();
            // @calc c and s
            auto const theta = ( A[q][q] - A[p][p] ) / ( A[p][q] + A[p][q] );
            auto const t     = std::copysign( one / ( std::abs( theta ) + std::hypot( theta, one ) ), theta );
            //auto const t     = std::copysign( one / ( std::abs(theta) + std::sqrt(theta*theta + one ) ), theta );
            auto const c     = one / std::hypot( t, one );
            //auto const c     = one / std::sqrt(t*t + one);
            auto const s     = t * c;
            // a quick explaination here:
            // the sparse matrix P, Givens matrix, is
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
            // and the iteration of A, A = P^T * A * P = (I+P_^T) * A * (I+P_)
            //                           = ( A + P_^T * A ) * (I + P_)
            //                       i.e., A += P_^T * A; A += A * P_;
            // in another word, here we avoid the multiplications between two dense matrices
#if 0
            //original version using sparse matrix
            sparse_matrix<value_type> P_( n, n );
            P_( p, p ) = c - one;
            P_( p, q ) = s;
            P_( q, q ) = c - one;
            P_( q, p ) = -s;
            V += V * P_; // this means V's pth col and qth col changed
            A += P_.transpose() * A; //this means A's pth row and qth row changed
            A += A * P_;
#endif

            //optimized version
            for ( std::size_t i = 0; i != n; ++i )
            {
                auto const vip = V[i][p] * c - V[i][q] * s;
                auto const viq = V[i][q] * c + V[i][p] * s;
                V[i][p] = vip;
                V[i][q] = viq;
                auto const api = c * A[p][i] - s * A[q][i];
                auto const aqi = c * A[q][i] + s * A[p][i];
                A[p][i] = api;
                A[q][i] = aqi;
            }

            for ( std::size_t i = 0; i != n; ++i )
            {
                auto const aip = A[i][p] * c - A[i][q] * s;
                auto const aiq = A[i][q] * c + A[i][p] * s;
                A[i][p] = aip;
                A[i][q] = aiq;
            }
        } //void rotate

    }//namespace eigen_jacobi_private

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
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector<T>& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }

    template<typename Matrix1, typename Matrix2, typename T = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray<T>& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }

    template<typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, matrix<T, A_>& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_jacobi( A, V, Lambda.diag_begin(), eps );
    }

    // A = V * diag{lambda...} * V^T
    template< typename Matrix1, typename Matrix2, typename Otor, typename T = double>
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        assert( A.row() == A.col() );
        //assert( is_symmetric( A ) );
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type( 1 );
        auto const zero = value_type( 0 );
        // @V = diag{1, 1, ..., 1}
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );
#if 1

        for ( size_type i = 0; i != size_type( -1 ); ++i )
        {
            // @find max non-diag value in A
            size_type p = 0;
            size_type q = 1;
            value_type current_max = std::abs( a[p][q] );

            for ( size_type ip = 0; ip != n; ++ip )
                for ( size_type iq = ip + 1; iq != n; ++iq )
                {
                    auto const tmp = std::abs( a[ip][iq] );

                    if ( current_max > tmp )
                        continue;

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

            // a and V iterations
            eigen_jacobi_private::rotate( a, V, p, q );
        }//end for

#endif
        // @just to kill warnings, should never reach here
        return size_type( -1 );
    }//eigen_jacobi

    template< typename Matrix1, typename Matrix2, typename Otor, typename T = double>
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, std::size_t max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        auto const compare_func = [eps]( const value_type lhs, const value_type rhs )
        {
            return std::abs( lhs - rhs ) < eps;
        };
        assert( A.row() == A.col() );
        //assert( is_symmetric( A, compare_func ) );
        size_type i     = 0;
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type( 1 );
        auto const zero = value_type( 0 );
        // @V = diag{1, 1, ..., 1}
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );

        for ( ; i != max_rot; ++i )
        {
            if ( !( i & 7 ) && eigen_jacobi_private::norm( a ) == zero )
            {
                break;
            }

            for ( size_type p = 0; p != n; ++p )
                for ( size_type q = p + 1; q != n; ++q )
                    eigen_jacobi_private::rotate( a, V, p, q );
        }

        std::copy( a.diag_begin(), a.diag_end(), o );
        return i * n * n;
    }

    template<typename Matrix1, typename Matrix2, typename T = double>
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector<T>& Lambda, std::size_t const max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }

    template<typename Matrix1, typename Matrix2, typename T = double>
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray<T>& Lambda, std::size_t const max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }

    template<typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double>
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, matrix<T, A_>& Lambda, std::size_t const max_rot = 80, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return cyclic_eigen_jacobi( A, V, Lambda.diag_begin(), max_rot, eps );
    }


}//namespace f

#endif//_EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

