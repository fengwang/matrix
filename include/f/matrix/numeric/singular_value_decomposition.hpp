#ifndef MSVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI
#define MSVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI

#include "../details/algorithm/for_each.hpp"

namespace f
{

    // Example:
    // matrix<double> A( 13, 12 );
    // matrix<double> u;
    // matrix<double> w;
    // matrix<double> v;
    // singular_value_decomposition( A, u, w, v );


    // Return
    //          1   ----    Failed to solve within given iterations
    //          0   ----    Successfully solve SVD
    // Effect  w v^{-1} u' A = I
    template< typename Matrix1, typename Matrix2, typename Matrix3, typename Matrix4 >
    std::size_t
    singular_value_decomposition(  const Matrix1& A,
                                   Matrix2& u,
                                   Matrix3& w,
                                   Matrix4& v,
                                   std::size_t const max_its = 1000
                                )
    {
        typedef typename Matrix1::value_type value_type;
        const value_type zero( 0 );
        const value_type one( 1 );
        const std::size_t m = A.row();
        const std::size_t n = A.col();
        u = A;
        w.resize( n, n );
        v.resize( n, n );
        //int i, j, k, l;
        std::size_t i{0}, l{0};
        value_type c{0}, f{0}, h{0};
        std::vector<value_type> arr( n );
        value_type g = zero;
        value_type s = zero;
        value_type scale = zero;
        value_type anorm = zero;

        for ( i = 0; i < n; ++i )
        {
            l = i + 2;
            arr[i] = scale * g;
            g = zero;
            s = zero;
            scale = zero;

            if ( i < m )
            {
                scale = std::accumulate( u.col_begin( i ) + i, u.col_end( i ), value_type( 0 ), []( value_type v1, value_type v2 )
                {
                    return v1 + std::abs( v2 );
                } );

                if ( scale != zero )
                {
                    std::for_each( u.col_begin( i ) + i, u.col_end( i ), [scale]( value_type & v )
                    {
                        v /= scale;
                    } );
                    const value_type tmp_s = std::inner_product( u.col_begin( i ) + i, u.col_end( i ), u.col_begin( i ) + i, value_type( 0 ) );
                    g = ( u[i][i] >= zero ) ? -std::sqrt( tmp_s ) : std::sqrt( tmp_s );
                    const value_type tmp_h = u[i][i] * g - tmp_s;
                    u[i][i] -= g;

                    for ( std::size_t j = l - 1; j < n; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.col_begin( i ) + i, u.col_end( i ), u.col_begin( j ) + i, value_type( 0 ) );
                        std::transform( u.col_begin( j ) + i, u.col_end( j ), u.col_begin( i ) + i, u.col_begin( j ) + i, [tmp_ss, tmp_h]( value_type v1, value_type v2 )
                        {
                            return v1 + tmp_ss * v2 / tmp_h;
                        } );
                    }

                    std::for_each( u.col_begin( i ) + i, u.col_end( i ), [scale]( value_type & v )
                    {
                        v *= scale;
                    } );
                } // if scale != zero
            } // if i < m

            w[i][i] = scale * g;
            g = zero;
            s = zero;
            scale = zero;

            if ( i + 1 <= m && i != n )
            {
                scale = std::accumulate( u.row_begin( i ) + l - 1, u.row_end( i ), value_type( 0 ), []( value_type v1, value_type v2 )
                {
                    return v1 + std::abs( v2 );
                } );

                if ( scale != zero )
                {
                    std::for_each( u.row_begin( i ) + l - 1, u.row_end( i ), [scale]( value_type & v )
                    {
                        v /= scale;
                    } );
                    auto const tmp_s = std::inner_product( u.row_begin( i ) + l - 1, u.row_end( i ), u.row_begin( i ) + l - 1, value_type( 0 ) );
                    g = ( u[i][l - 1] >= zero ) ? -std::sqrt( tmp_s ) : std::sqrt( tmp_s );
                    auto const tmp_h = u[i][l - 1] * g - tmp_s;
                    u[i][l - 1] -=  g;
                    std::transform( u.row_begin( i ) + l - 1, u.row_end( i ), arr.begin() + l - 1, [tmp_h]( value_type v )
                    {
                        return v / tmp_h;
                    } );

                    for ( std::size_t j = l - 1; j < m; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.row_begin( j ) + l - 1, u.row_end( j ), u.row_begin( i ) + l - 1, value_type( 0 ) );
                        std::transform( u.row_begin( j ) + l - 1, u.row_end( j ), arr.begin() + l - 1, u.row_begin( j ) + l - 1, [tmp_ss]( value_type v1, value_type v2 )
                        {
                            return v1 + tmp_ss * v2;
                        } );
                    } // j loop

                    std::for_each( u.row_begin( i ) + l - 1, u.row_end( i ), [scale]( value_type & v )
                    {
                        v *= scale;
                    } );
                } // if i+1 != m && ...
            }

            anorm = std::max( anorm, ( std::fabs( w[i][i] ) + std::fabs( arr[i] ) ) );
        }

        for ( i = n - 1; ; --i )
        {
            if ( i < n - 1 )
            {
                if ( g != zero )
                {
                    auto const tmp_uil = u[i][l];
                    std::transform( u.row_begin( i ) + l, u.row_end( i ), v.col_begin( i ) + l, [g, tmp_uil]( value_type val )
                    {
                        return val / ( tmp_uil * g );
                    } );

                    for ( std::size_t j = l; j < n; j++ )
                    {
                        const auto tmp_s = std::inner_product( u.row_begin( i ) + l, u.row_end( i ), v.col_begin( j ) + l, value_type( 0 ) );
                        std::transform( v.col_begin( j ) + l, v.col_end( j ), v.col_begin( i ) + l, v.col_begin( j ) + l, [tmp_s]( value_type v1, value_type v2 )
                        {
                            return v1 + v2 * tmp_s;
                        } );
                    }
                } // if g != zero

                std::fill( v.row_begin( i ) + l, v.row_end( i ), zero );
                std::fill( v.col_begin( i ) + l, v.col_end( i ), zero );
            } // if i < n-1

            v[i][i] = one;
            g = arr[i];
            l = i;

            if ( !i ) break;
        } // i loop

        for ( i = std::min( m, n ) - 1; ; --i )
        {
            auto const tmp_l = i + 1;
            auto const tmp_g = w[i][i];
            std::fill( u.row_begin( i ) + tmp_l, u.row_end( i ), zero );

            if ( tmp_g != zero )
            {
                for ( std::size_t j = tmp_l; j < n; j++ )
                {
                    auto const tmp_s = std::inner_product( u.col_begin( i ) + tmp_l, u.col_end( i ), u.col_begin( j ) + tmp_l, value_type( 0 ) );
                    auto const tmp_f = tmp_s / ( u[i][i] * tmp_g );
                    std::transform( u.col_begin( j ) + i, u.col_end( j ), u.col_begin( i ) + i, u.col_begin( j ) + i, [tmp_f]( value_type v1, value_type v2 )
                    {
                        return v1 + tmp_f * v2;
                    } );
                } // j loop

                std::for_each( u.col_begin( i ) + i, u.col_end( i ), [tmp_g]( value_type & v )
                {
                    v /= tmp_g;
                } );
            }
            else
                std::fill( u.col_begin( i ) + i, u.col_end( i ), zero );

            ++u[i][i];

            if ( !i ) break;
        } // i loop

        for ( std::size_t k = n - 1; ; --k )
        {
            for ( std::size_t its = 0; its < max_its; its++ )
            {
                bool flag = true;
                std::size_t tmp_nm = 0;

                for ( l = k; ; l-- )
                {
                    tmp_nm = l - 1;

                    if ( std::fabs( arr[l] ) + anorm == anorm )
                    {
                        flag = false;
                        break;
                    }

                    if ( std::fabs( w[l - 1][l - 1] ) + anorm == anorm )
                    {
                        break;
                    }

                    if ( l == 0 )
                        break;
                } // l loop

                if ( flag )
                {
                    c = zero;
                    s = one;

                    for ( i = l - 1; i < k + 1; ++i )
                    {
                        f = s * arr[i];
                        arr[i] = c * arr[i];

                        if ( std::fabs( f ) + anorm == anorm )
                        {
                            break;
                        }

                        g = w[i][i];
                        h = std::hypot( f, g );
                        w[i][i] = h;
                        h = one / h;
                        c = g * h;
                        s = -f * h;

                        for ( std::size_t j = 0; j < m; ++j )
                        {
                            value_type y = u[j][tmp_nm];
                            value_type z = u[j][i];
                            u[j][tmp_nm] = y * c + z * s;
                            u[j][i] = z * c - y * s;
                        } // j loop
                    } // i loop
                } // if flag

                value_type z = w[k][i];

                if ( l == k )
                {
                    if ( z < zero )
                    {
                        w[k][k] = -z;
                        std::for_each( v.col_begin( k ), v.col_end( k ), []( value_type & v )
                        {
                            v = -v;
                        } );
                    }

                    break;
                } // if l == k

                if ( ( its + 1 ) == max_its )
                    return 1;

                //{ assert( !"no convergence in 100 singular_value_decomposition iterations" ); }
                value_type x = w[l][l];
                value_type y = w[k - 1][k - 1];
                g = arr[k - 1];
                h = arr[k];
                f = ( ( y - z ) * ( y + z ) + ( g - h ) * ( g + h ) ) / ( 2.0 * h * y );
                g = std::hypot( f, one );
                g = ( f >= zero ) ? g : -g;
                f = ( ( x - z ) * ( x + z ) + h * ( ( y / ( f + g ) ) ) - h ) / x;
                c = s = one;

                for ( std::size_t j = l; j <= k - 1; j++ )
                {
                    g = arr[j + 1];
                    y = w[j + 1][j + 1];
                    h = s * g;
                    g = c * g;
                    z = std::hypot( f, h );
                    arr[j] = z;
                    c = f / z;
                    s = h / z;
                    f = x * c + g * s;
                    g = g * c - x * s;
                    h = y * s;
                    y *= c;
                    for_each( v.col_begin( j ), v.col_end( j ), v.col_begin( j + 1 ), [c, s]( value_type & v1, value_type & v2 )
                    {
                        const auto vv1( v1 );
                        const auto vv2( v2 );
                        v1 = vv1 * c + vv2 * s;
                        v2 = vv2 * c - vv1 * s;
                    } );
                    w[j][j] = std::hypot( f, h );

                    if ( value_type( 0 ) != w[j][j] )
                    {
                        c = f / w[j][j];
                        s = h / w[j][j];
                    } // if z

                    //defined in misc.hpp
                    for_each( u.col_begin( j ), u.col_end( j ), u.col_begin( j + 1 ), [c, s]( value_type & v1, value_type & v2 )
                    {
                        const auto vv1( v1 );
                        const auto vv2( v2 );
                        v1 = vv1 * c + vv2 * s;
                        v2 = vv2 * c - vv1 * s;
                    } );
                    f = c * g + s * y;
                    x = c * y - s * g;
                } // j loop

                arr[l] = zero;
                arr[k] = f;
                w[k][k] = x;
                i = k;
            }//its loop

            if ( !k ) break;
        } // k loop

        return 0;//return with success
    }//singular_value_decomposition


    // for an input matrix of n,m this will output a persudo inverse matrix m,n
    template< typename Matrix >
    Matrix const svd_inverse( Matrix const& A )
    {
        Matrix u;
        Matrix w;
        Matrix v;
        singular_value_decomposition( A, u, v, w );
        std::for_each( v.begin(), v.end(), []( auto & val )
        {
            if ( std::abs( val ) > 1.0e-10 ) val = 1.0 / val;
        } );
        return w * v.transpose() * u.transpose();
    }

}//namespace sm


#endif//_SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI

