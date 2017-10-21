#ifndef MCONJUGATE_GRADIENT_SQUARED_HPP_INCLUDED_SODJI489ASIODJHSALKJCLVKJZXLVKJU4098USLFJKOIFOIRISV
#define MCONJUGATE_GRADIENT_SQUARED_HPP_INCLUDED_SODJI489ASIODJHSALKJCLVKJZXLVKJU4098USLFJKOIFOIRISV

namespace f
{
    //solving   Ax = b
    //
    //return
    //          0    --     success
    //          1    --     failed
    template<typename T1, typename A1, typename T2, typename A2, typename T3, typename A3>
    int
    conjugate_gradient_squared( const matrix<T1, A1>&     A, //A[n][n]
                                matrix<T2, A2>&           x, //x[n]
                                const matrix<T3, A3>&     b, //b[n]
                                const std::size_t           max_loops = 100,
                                const T1                    eps = 1.0e-10 )
    {
        typedef matrix<T1, A1>                            matrix_type;
        typedef typename matrix_type::value_type            value_type;
        typedef typename matrix_type::size_type             size_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        //initialize x
        size_type const n = A.row();
        x.resize( n, 1 );

        if ( dot( x, x ) == value_type() )
            x = b;

        //arbitrary vector r_
        matrix_type const r_ = b - A * x;
        matrix_type r = r_;
        matrix_type p = r_;
        matrix_type u = r_;
        matrix_type ap = r_;
        matrix_type q = r_;
        matrix_type new_r = r_;
        matrix_type uq = r_;
        matrix_type rem = r_;
        value_type const EPS = n * eps * eps;

        if ( dot( r_, r_ ) < EPS ) return 0;

        for ( std::size_t i = 0; i != max_loops; ++i )
        {
            ap = A * p;
            value_type const alpha = dot( r, r_ ) / dot( ap, r_ );

            if ( std::isinf( alpha ) || std::isnan( alpha ) )
            {
                return 1;
            }

            q = u - alpha * ap;
            uq = u + q;
            x += alpha * uq;
            rem = A * x - b;

            if ( dot( rem, rem ) < EPS ) return 0;

            new_r = r - alpha * A * uq;
            value_type const beta = dot( new_r, r_ ) / dot( r, r_ );

            if ( std::isinf( beta ) || std::isnan( beta ) )
            {
                return 1;
            }

            r = new_r;
            u = r + beta * q;
            p = u + beta * ( q + beta * p );
        }

        return 0;
    }

    template<typename T1, typename A1, typename T2, typename A2, typename T3, typename A3>
    int
    cgs( const matrix<T1, A1>&     A, //A[n][n]
         matrix<T2, A2>&           x, //x[n]
         const matrix<T3, A3>&     b, //b[n]
         const std::size_t           max_loops = 100,
         const T1                    eps = 1.0e-10 )
    {
        return conjugate_gradient_squared( A, x, b, max_loops, eps );
    }


}//namespace f

#endif//_CONJUGATE_GRADIENT_SQUARED_HPP_INCLUDED_SODJI489ASIODJHSALKJCLVKJZXLVKJU4098USLFJKOIFOIRISV

