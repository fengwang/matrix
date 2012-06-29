#ifndef _CONJUGATE_GRADIENT_SQUARED_HPP_INCLUDED_SODJI489ASIODJHSALKJCLVKJZXLVKJU4098USLFJKOIFOIRISV
#define _CONJUGATE_GRADIENT_SQUARED_HPP_INCLUDED_SODJI489ASIODJHSALKJCLVKJZXLVKJU4098USLFJKOIFOIRISV

#include <matrix/matrix.hpp>
#include <matrix/functional.hpp>

#include <cstddef>

namespace feng
{
    //solving linear equations Ax=b using conjugate gradient squared method,
    //returns the iterations used

    template<typename Matrix1, typename Matrix2, typename Matrix3>
    std::size_t conjugate_gradient_squared( const Matrix1& A, Matrix2& x, const Matrix3& b, const std::size_t max_loops = 100, const typename Matrix1::value_type eps = 1.0e-10 )
    {
        typedef typename Matrix1::value_type value_type; 

        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );

        //initialize x
        auto const n = A.row();
        x.resize( n, 1 );
        if ( dot(x, x) == value_type() )
            x = b;

        //arbitrary vector r_
        auto const r_ = b - A * x;
        auto r = r_;
        auto p = r_;
        auto u = r_;

        auto ap = r_;
        auto q = r_;
        auto new_r = r_;
        auto uq = r_;

        auto rem = r_;
        auto const EPS = n * eps;

        for ( std::size_t i = 0; i != max_loops; ++i )
        {
            ap = A * p;
            auto const alpha = dot(r, r_) / dot(ap, r_);
            q = u - alpha * ap;
            uq = u + q; 
            x += alpha * uq;
            
            rem = A*x-b;
            if ( dot(rem, rem) < EPS ) return i;

            new_r = r - alpha * A * uq;
            auto const beta = dot(new_r, r_)/dot(r, r_);
            r = new_r;
            u = r + beta * q;
            p = u + beta * ( q + beta * p);

        }

        return max_loops;
    }

    template<typename Matrix1, typename Matrix2, typename Matrix3>
    std::size_t cgs( const Matrix1& A, Matrix2& x, const Matrix3& b, const std::size_t max_loops = 100, const typename Matrix1::value_type eps = 1.0e-10 )
    {
        return conjugate_gradient_squared( A, x, b, max_loops, eps ); 
    }


}//namespace feng

#endif//_CONJUGATE_GRADIENT_SQUARED_HPP_INCLUDED_SODJI489ASIODJHSALKJCLVKJZXLVKJU4098USLFJKOIFOIRISV

