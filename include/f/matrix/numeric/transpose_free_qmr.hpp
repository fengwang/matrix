#ifndef MTRANSPOSE_FREE_QMR_HPP_INCLUDED_SIO3RI8FSDKLVMNDSSSSSSCCCSKLJNXCMNSJKFALKJSDOUI4HFJKA9CNFKJ
#define MTRANSPOSE_FREE_QMR_HPP_INCLUDED_SIO3RI8FSDKLVMNDSSSSSSCCCSKLJNXCMNSJKFALKJSDOUI4HFJKA9CNFKJ

namespace f
{
    template< typename Matrix1, typename Matrix2, typename Matrix3 >
    std::size_t transpose_free_qmr( const Matrix1& A, Matrix2& x, const Matrix3& b, const std::size_t max_loops = 1000, typename Matrix1::value_type eps = 1.0e-10 )
    {
        typedef Matrix1::value_type value_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        auto const n = A.row();
        auto const zero = value_type( 0 );
        auto const one = value_type( 1 );
        x.resize( n, 1 );

        if ( zero == dot( x, x ) )
            x = b;

        auto const r_ = b - A * x;
        auto w = r_;
        auto u = r_;
        auto r = r_;
        auto v = A * u;
        auto d = Matrix1( n, 1 );
        auto u_new = u;
        auto u_old = u;
        auto tao = std::sqrt( dot( r, r ) );
        auto theta = zero;
        auto eta = zero;
        auto rho = dot( r_, r );
        auto c = zero;
        auto alpha = zero;
        auto beta = zero;
        auto alpha_new = alpha;
        auto rho_new = rho;

        for ( std::size_t i = 0; i != max_loops; ++i )
        {
            if ( ! ( i & 1 ) )
            {
                alpha = rho / dot( v, r_ );
                alpha_new = alpha;
                u_new = u - alpha * v;
            }

            w -= alpha * A * u;
            d = u + theta * theta * eta * d / alpha;
            theta = std::sqrt( dot( w, w ) ) / tao;
            c = one / std::hypot( one, theta );
            tao *= theta * c;
            eta = c * c * alpha;
            x += eta * d;

            if ( i & 1 )
            {
                rho_new = dot( w, r_ );
                beta = rho_new / rho;
                u_new = w + beta * u;
                v_new = A * u_new + beta * ( A * u + beta * v_old );
            }

            u = u_new;
            alpha = alpha_new;
        }

        return max_loops;
    }

}//namespace f

#endif//_TRANSPOSE_FREE_QMR_HPP_INCLUDED_SIO3RI8FSDKLVMNDSSSSSSCCCSKLJNXCMNSJKFALKJSDOUI4HFJKA9CNFKJ

