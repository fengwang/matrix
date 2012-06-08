#ifndef _BICONJUGATE_GRADIENT_STABLIZED_METHOD_INCLUDED_FSDOJIIOSFADKLJOIRSDFKJSFOIJSDFOI489UVJKFUFF
#define _BICONJUGATE_GRADIENT_STABLIZED_METHOD_INCLUDED_FSDOJIIOSFADKLJOIRSDFKJSFOIJSDFOI489UVJKFUFF

#include <matrix/matrix.hpp>
#include <matrix/sparse_matrix.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cassert>

namespace feng
{

    namespace bicgstab_private
    {
        template<typename T, typename It1, typename It2>
        T included_angle( It1 i1_, It1 _i1, It2 i2_, T v )
        {
            auto const n = std::distance( i1_, _i1 );
            auto const ab = std::inner_product( i1_, _i1, i2_, v );
            auto const aa = std::inner_product( i1_, _i1, i1_, v );
            auto const bb = std::inner_product( i2_, i2_+n, i2_, v );
            return std::acos(ab/(std::sqrt(aa)*std::sqrt(bb)));
        }
    }//namespace bicgstab_private 

    //
    // Comment:
    //          Using Biconjugate Gradient Stablized Method to solve equation
    //                  A x = b
    // Inputs:
    //          A       :       square matrix A [n,]
    //          x       :       unknown vector x [n,1]
    //          b       :       vector b [n,1]
    //          loops   :       max iterations, default is 50
    //          eps     :       precision control, default is 1.0e-5
    // Returns:
    //          iterations consumed
    //
    template<typename Matrix, typename T, std::size_t D, typename A_>
    std::size_t biconjugate_gradient_stablized_method( const Matrix& A, matrix<T,D,A_>& x, const matrix<T,D,A_>& b, std::size_t const loops = 50, T const eps = T(1.0e-5) )
    {
        typedef matrix<T,D,A_> matrix_type;
        typedef T value_type;

        assert( A.row() == A.col() );
        assert( A.col() == b.row() );
        assert( b.col() == 1 );
        
        T const one = value_type(1);
        T const zero = value_type(0);

        auto const n = A.row();
        // x_0 = b
        x = b;
        // r_0 = b - A x_0
        matrix_type r = b - A * x;
        // r'_0 = r_0
        matrix_type const r_ = r; 
        // rho = alpha = omega = 1
        auto rho = one;
        auto alpha = one;
        auto omega = one;
        //v_0 = p_0 = {0,0...,0}
        auto v  = matrix_type{n,1};
        auto p  = matrix_type{n,1};
        auto s  = matrix_type{n,1};
        auto t  = matrix_type{n,1};
        auto b_ = matrix_type{n,1};
        //loops i = 1, 2, ..., n
        for ( std::size_t i = 0; i != loops; ++i )
        {
           // rho_i = ( r'_0, r_{i-1} 
            auto const rho_new = std::inner_product( r_.begin(), r_.end(), r.begin(), zero );
            // beta = rho_i / rho_{i-1}  * alpha / omega_{i-1}
            auto const beta = rho_new * alpha / ( rho * omega );
            rho = rho_new;
            // p_i = r_{r-1} + beta ( p_{i-1} - omega v_{i-1} )
            p = r + beta * ( p - omega * v );
            // v_i = A p_i
            v = A * p;
            // alpha = rho / ( r'_0, v_i)
            alpha = rho / std::inner_product( r_.begin(), r_.end(), v.begin(), zero );
            // s = r_{i-1} - alpha v_{i}
            s = r - alpha * v;
            // t = A s
            t = A * s;
            // omega = (t,s)/(t,t)
            omega = std::inner_product( t.begin(), t.end(), s.begin(), zero ) / std::inner_product( t.begin(), t.end(), t.begin(), zero );
            // x_i = x_{i-1} + alpha p_i + omega s
            x += alpha * p + omega * s;
            // if x_i is accurate enough, then quit
            b_ = A * x;
            if ( bicgstab_private::included_angle( b_.begin(), b_.end(), b.begin(), zero ) <= eps )
                return i;
            // r_i = s - omega t
            r = s - omega * t;
        }
        
        return loops;
    }

    //
    // Comment:
    //          Using Biconjugate Gradient Stablized Method to solve equation
    //                  A x = b
    // Inputs:
    //          A       :       square matrix A [n,]
    //          iix_    :       first position of unknown vector x 
    //          iib_    :       first position of vector b 
    //          loops   :       max iterations, default is 50
    //          eps     :       precision control, default is 1.0e-5
    // Returns:
    //          iterations consumed
    //
    template<typename Matrix, typename II1, typename II2, typename T=double>
    std::size_t biconjugate_gradient_stablized_method( const Matrix& A, II1 iix_, II2 IIb_, std::size_t const loops = 50, T const eps = T(1.0e-5) )
    {
        auto const n = A.row();
        matrix<T> x{ n, 1 };
        matrix<T> b{ n, 1, IIb_, IIb_+n };
        auto ans = biconjugate_gradient_stablized_method( A, x, b, loops, eps );
        std::copy( x.begin(), x.end(), iix_ );
        return ans;
    }

    template<typename Matrix, typename T, std::size_t D, typename A_>
    std::size_t bicgstab( const Matrix& A, matrix<T,D,A_>& x, const matrix<T,D,A_>& b, std::size_t const loops = 50, T const eps = T(1.0e-5) )
    {
        return biconjugate_gradient_stablized_method( A, x, b, loops, eps );
    }

    template<typename Matrix, typename II1, typename II2, typename T=double>
    std::size_t bicgstab( const Matrix& A, II1 iix_, II2 IIb_, std::size_t const loops = 50, T const eps = T(1.0e-5) )
    {
        return biconjugate_gradient_stablized_method( A, iix_, IIb_, loops, eps );
    }

}//namespace feng

#endif//_BICONJUGATE_GRADIENT_STABLIZED_METHOD_INCLUDED_FSDOJIIOSFADKLJOIRSDFKJSFOIJSDFOI489UVJKFUFF

