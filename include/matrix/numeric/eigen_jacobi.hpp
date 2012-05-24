#ifndef _EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU
#define _EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

#include <matrix/matrix.hpp>
#include <matrix/sparse_matrix.hpp>

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
        auto a          = A;
        auto _a         = a;
        auto const n    = a.row();
        auto const one  = value_type(1);
        auto const zero = value_type(0);
        /*
        auto P          = A;
        */
        
        // @V = diag{1, 1, ..., 1}
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );
        auto _v          = V;

        for ( size_type i = 0; i != size_type(-1); ++i )
        {
            //_a = a; //just a working copy

            // @find max non-diag value in A
            size_type p = 0;
            size_type q = 1;
            value_type current_max = std::abs(_a[p][q]);

            for ( size_type ip = 0; ip != n; ++ip )
                for ( size_type iq = ip+1; iq != n; ++iq )
                {
                    auto const tmp = std::abs(_a[ip][iq]);
                    if ( current_max > tmp ) continue;
                    current_max = tmp;
                    p           = ip;
                    q           = iq;
                }

            // @if all non-diag value is small, then break the algorithm
            if ( current_max < eps ) 
            {
                std::copy( _a.diag_begin(), _a.diag_end(), o );
                return i;
            }

            // @calc c and s
            auto const theta = ( _a[q][q] - _a[p][p] ) / ( _a[p][q] + _a[p][q] );
            auto const t     = std::copysign( one / ( std::abs(theta) + std::sqrt(theta*theta + one ) ), theta );
            auto const c     = one / std::sqrt(t*t + one);
            auto const s     = t * c;
#if 0
            auto const cc    = c*c;
            auto const ss    = s*s;
            auto const sscc  = value_type(2) * s*c;
            // @update pth colume of _a
            std::transform( a.col_begin(p), a.col_end(p), a.col_begin(q), _a.col_begin(p), [c,s](value_type const cp, value_type const cq) { return c*cp - s*cq; } );
            // @update qth column of _a
            std::transform( a.col_begin(q), a.col_end(q), a.col_begin(p), _a.col_begin(q), [c,s](value_type const cq, value_type const cp) { return c*cq + s*cp; } );
            // @copy pth column of _a to pth row of _a
            std::copy( _a.col_begin(p), _a.col_end(p), _a.row_begin(p) );
            // @copy qth column of _a to qth row of _a
            std::copy( _a.col_begin(q), _a.col_end(q), _a.row_begin(q) );
            // @set _a[p][q] and _a[q][p] to zero 
            _a[p][q] = zero;
            _a[q][p] = zero;
            // @set _a[p][p]
            _a[p][p] = cc * a[p][p] - ss * a[q][q] - sscc * a[p][q];
            // @set _a[q][q] 
            _a[q][q] = ss * a[p][p] + cc * a[q][q] + sscc * a[p][q];
            // @update a 
            a = _a;

            //_v = V; //a working copy
            // @change pth column of _v
            std::transform( V.col_begin(p), V.col_end(p), V.col_begin(q), _v.col_begin(p), [c,s](value_type const cp, value_type const cq) { return c*cp - s*cq; } );
            // @update qth column of _v
            std::transform( V.col_begin(q), V.col_end(q), V.col_begin(p), _v.col_begin(q), [c,s](value_type const cq, value_type const cp) { return c*cq + s*cp; } );
            // @update V
            V = _v;
#endif
            /*
            P = zero;
            std::fill( P.diag_begin(), P.diag_end(), one );

            P[p][p] =  c;
            P[q][q] =  c;
            P[p][q] =  s;
            P[q][p] = -s;
            */
            sparse_matrix<value_type> P(n,n);
            for (size_type i = 0; i != n; ++i )
                P(i,i) = one;
            P(p,p) =  c;
            P(q,q) =  c;
            P(p,q) =  s;
            P(q,p) = -s;

            _a = P.transpose() * _a * P;
            V = V * P;
            a = _a;

        }//end for
        return size_type(-1);
    }//eigen_jacobi

}//namespace feng

#endif//_EIGEN_JACOBI_HPP_INCLUDED_SDF3IUJFA89S04390SFIUH3IUHSFJKDH43QU9ISKJHIDUHASIUHFDIUHTKJHFSDIU

