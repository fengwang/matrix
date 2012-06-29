#ifndef _BICONJUGATE_GRADIENT_STABLIZED_METHOD_INCLUDED_FSDOJIIOSFADKLJOIRSDFKJSFOIJSDFOI489UVJKFUFF
#define _BICONJUGATE_GRADIENT_STABLIZED_METHOD_INCLUDED_FSDOJIIOSFADKLJOIRSDFKJSFOIJSDFOI489UVJKFUFF

#include <matrix/matrix.hpp>
#include <matrix/functional.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cassert>

namespace feng
{

    //
    // Comment:
    //          Using Biconjugate Gradient Stablized Method to solve equation
    //                  A x = b
    // Inputs:
    //          A       :       square matrix A [n,]
    //          x       :       unknown vector x [n,1]
    //          b       :       vector b [n,1]
    //          loops   :       max iterations, default is 100
    //          eps     :       precision control, default is 1.0e-5
    // Returns:
    //          iterations consumed
    //
    // TODO: opmtimize this algorithm to get rid of round-offs, and test it with more data
    template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2, typename T3, std::size_t D3, typename A3>
    std::size_t
    biconjugate_gradient_stablized_method( const matrix<T1, D1, A1>&     A,
                                           matrix<T2, D2, A2>&           x,
                                           const matrix<T3, D3, A3>&     b,
                                           const std::size_t           max_loops = 100,
                                           const T1                    eps = 1.0e-10 )
    {
        typedef matrix<T1, D1, A1>     matrix_type;
        typedef T1                   value_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        auto const n     = A.row();

        //if no initial guess of x, then set its initial guess to b
        if ( ( n != x.row() ) || ( 1 != x.col() ) || ( 0 == std::count_if( x.begin(), x.end(), []( T2 const v ) { return v != T2( 0 ); } ) ) )
            x = b;
        auto r = b - A * x;
        auto const r_ = r;
        auto p = r;
        auto s = p;
        auto ap = p;
        auto as = p;
        auto new_r = r;
        auto rem = r;
        auto const EPS = eps * n;

        for ( std::size_t i = 0; i != max_loops; ++i )
        {
            ap = A * p;
            auto const alpha = dot(r, r_) / dot( ap, r_ );
            if ( std::isinf(alpha) || std::isnan(alpha) ) return i;

            s = r - alpha * ap;
            as = A * s;
            auto const omega = dot( as, s) / dot( as, as );
            if ( std::isinf(omega) || std::isnan(omega) ) return i;

            x += alpha * p + omega * s;
            new_r = s - omega * as;
            auto const beta = dot( new_r, r_ ) * alpha / dot(r, r_) / omega;
            if ( std::isinf(beta) || std::isnan(beta) ) return i;
            
            r = new_r;
            p = r + beta * ( p - omega * ap );

            rem = A * x - b;
            if ( dot(rem, rem) <= EPS )
                return i;
        }

        return max_loops;
    }

    //
    // Comment:
    //          Using Biconjugate Gradient Stablized Method to solve equation
    //                  A x = b
    // Inputs:
    //          A       :       square matrix A [n,]
    //          iix_    :       first position of unknown vector x
    //          iib_    :       first position of vector b
    //          loops   :       max iterations, default is 100
    //          eps     :       precision control, default is 1.0e-5
    // Returns:
    //          iterations consumed
    //
    template<typename Matrix, typename II1, typename II2, typename T = double>
    std::size_t biconjugate_gradient_stablized_method( const Matrix& A, II1 iix_, II2 IIb_, std::size_t const loops = 100, T const eps = T( 1.0e-15 ) )
    {
        auto const n = A.row();
        matrix<T> x { n, 1 };
        matrix<T> b { n, 1, IIb_, IIb_ + n };
        auto ans = biconjugate_gradient_stablized_method( A, x, b, loops, eps );
        std::copy( x.begin(), x.end(), iix_ );
        return ans;
    }

    template<typename Matrix, typename T, std::size_t D, typename A_>
    std::size_t bicgstab( const Matrix& A, matrix<T, D, A_>& x, const matrix<T, D, A_>& b, std::size_t const loops = 100, T const eps = T( 1.0e-15 ) )
    {
        return biconjugate_gradient_stablized_method( A, x, b, loops, eps );
    }

    template<typename Matrix, typename II1, typename II2, typename T = double>
    std::size_t bicgstab( const Matrix& A, II1 iix_, II2 IIb_, std::size_t const loops = 100, T const eps = T( 1.0e-15 ) )
    {
        return biconjugate_gradient_stablized_method( A, iix_, IIb_, loops, eps );
    }

}//namespace feng

#endif//_BICONJUGATE_GRADIENT_STABLIZED_METHOD_INCLUDED_FSDOJIIOSFADKLJOIRSDFKJSFOIJSDFOI489UVJKFUFF

