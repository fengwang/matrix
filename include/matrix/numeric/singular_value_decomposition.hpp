#ifndef _SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI
#define _SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI

#include <matrix/matrix.hpp>

#include <cstddef>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <valarray>
#include <vector>

namespace feng
{

    // A = U W V^{T}
    // Example:
    // matrix<double> A( 13, 12 );
    // matrix<double> u;
    // matrix<double> w;
    // matrix<double> v;
    // singular_value_decomposition( A, u, w, v );

    template< typename Matrix1, typename Matrix2, typename Matrix3, typename Matrix4 >
    void singular_value_decomposition(  const Matrix1& A,
                                        Matrix2& u,
                                        Matrix3& w,
                                        Matrix4& v )
    {
        typedef typename Matrix1::value_type value_type;

        const value_type zero(0);
        const value_type one(1);
        const std::size_t max_its(30);
        std::size_t m = A.row();
        std::size_t n = A.col();
        u = A;
        w.resize( n, n );
        v.resize( n, n );
        int i, j, jj, k, l;
        value_type c, f, h;
        std::vector<value_type> rv1(n);
        value_type g = zero;
        value_type s = zero;
        value_type scale = zero;
        value_type anorm = zero;

        for ( i = 0; i < n; ++i )
        {
            l = i + 2;
            rv1[i] = scale * g;
            g = zero;
            s = zero;
            scale = zero;

            if ( i < m )
            {
                scale = std::accumulate( u.col_begin(i)+i, u.col_end(i), value_type(0), []( value_type v1, value_type v2 ) { return v1 + std::abs(v2); } );

                if ( scale != zero )
                {
                    std::for_each( u.col_begin(i)+i, u.col_end(i), [scale](value_type& v){ v/=scale; } );
                    const value_type tmp_s = std::inner_product( u.col_begin(i)+i, u.col_end(i), u.col_begin(i)+i, value_type(0) );

                    g = (u[i][i] >= zero) ? -std::sqrt(tmp_s) : std::sqrt(tmp_s);
                    const value_type tmp_h = u[i][i] * g - tmp_s;
                    u[i][i] -= g;

                    for ( j = l-1; j < n; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.col_begin(i)+i, u.col_end(i), u.col_begin(j)+i, value_type(0) );
                        std::transform( u.col_begin(j)+i, u.col_end(j), u.col_begin(i)+i, u.col_begin(j)+i, [tmp_ss, tmp_h]( value_type v1, value_type v2 ) { return v1 + tmp_ss * v2 / tmp_h; } );
                    }

                    std::for_each( u.col_begin(i)+i, u.col_end(i), [scale](value_type& v){ v*=scale; } );
                } // if scale != zero
            } // if i < m

            w[i][i] = scale * g;
            g = zero;
            s = zero;
            scale = zero;

            if ( i + 1 <= m && i != n )
            {
                scale = std::accumulate( u.row_begin(i)+l-1, u.row_end(i), value_type(0), [](value_type v1, value_type v2 ) { return v1 + std::abs(v2); } );

                if ( scale != zero )
                {
                    std::for_each( u.row_begin(i)+l-1, u.row_end(i), [scale](value_type& v){ v/=scale; } );
                    auto const tmp_s = std::inner_product( u.row_begin(i)+l-1, u.row_end(i), u.row_begin(i)+l-1, value_type(0) );

                    g = (u[i][l-1] >= zero) ? -std::sqrt(tmp_s) : std::sqrt(tmp_s);
                    auto const tmp_h = u[i][l-1] * g - tmp_s;
                    u[i][l-1] -=  g;

                    std::transform( u.row_begin(i)+l-1, u.row_end(i), rv1.begin()+l-1, [tmp_h](value_type v){ return v/tmp_h; } );

                    for ( j = l-1; j < m; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.row_begin(j)+l-1, u.row_end(j), u.row_begin(i)+l-1, value_type(0) );
                        std::transform( u.row_begin(j)+l-1, u.row_end(j), rv1.begin()+l-1, u.row_begin(j)+l-1, [tmp_ss](value_type v1, value_type v2){ return v1 + tmp_ss * v2; } );
                    } // j loop

                    std::for_each( u.row_begin(i)+l-1, u.row_end(i), [scale](value_type& v) { v *= scale; } );
                } // if i+1 != m && ...
            }

            anorm = std::max( anorm, ( std::fabs( w[i][i] ) + std::fabs( rv1[i] ) ) );
        }

        for ( i = n-1; i >= 0; i-- )
        {
            if ( i < n-1 )
            {
                if ( g != zero )
                {
                    auto const tmp_uil = u[i][l];
                    std::transform( u.row_begin(i)+l, u.row_end(i), v.col_begin(i)+l, [g, tmp_uil](value_type val){ return val/(tmp_uil*g); } );

                    for ( j = l; j < n; j++ )
                    {
                        const auto tmp_s = std::inner_product( u.row_begin(i)+l, u.row_end(i), v.col_begin(j)+l, value_type(0));
                        std::transform( v.col_begin(j)+l, v.col_end(j), v.col_begin(i)+l, v.col_begin(j)+l, [tmp_s](value_type v1, value_type v2){ return v1 + v2 * tmp_s; } );
                    }
                } // if g != zero

                std::fill( v.row_begin(i)+l, v.row_end(i), zero );
                std::fill( v.col_begin(i)+l, v.col_end(i), zero );

            } // if i < n-1

            v[i][i] = one;
            g = rv1[i];
            l = i;
        } // i loop

        for ( i = std::min( m, n )-1; i >= 0; --i )
        {
            auto const tmp_l = i + 1;
            auto const tmp_g = w[i][i];

            std::fill( u.row_begin(i)+tmp_l, u.row_end(i), zero );

            if ( tmp_g != zero )
            {
                for ( j = tmp_l; j < n; j++ )
                {
                    auto const tmp_s = std::inner_product( u.col_begin(i)+tmp_l, u.col_end(i), u.col_begin(j)+tmp_l, value_type(0) );
                    auto const tmp_f = tmp_s / (u[i][i] * tmp_g);
                    std::transform( u.col_begin(j)+i, u.col_end(j), u.col_begin(i)+i, u.col_begin(j)+i, [tmp_f](value_type v1, value_type v2){ return v1+tmp_f*v2; } );
                } // j loop

                std::for_each( u.col_begin(i)+i, u.col_end(i), [tmp_g](value_type& v){ v /= tmp_g; } );
            }
            else
                std::fill( u.col_begin(i)+i, u.col_end(i), zero );

            ++u[i][i];
        } // i loop

        for ( k = n-1; k >= 0; k-- )
        {
            for ( std::size_t its = 0; its < max_its; its++ )
            {
                bool flag = true;
                std::size_t tmp_nm = 0;

                for ( l = k; l >= 0; l-- )
                {
                    tmp_nm = l-1;

                    if ( std::fabs( rv1[l] ) + anorm == anorm )
                    {
                        flag = false;
                        break;
                    }

                    if ( std::fabs( w[l-1][l-1] ) + anorm == anorm )
                    { break; }
                } // l loop

                if ( flag )
                {
                    c = zero;
                    s = one;

                    for ( i = l-1; i < k + 1; i++ )
                    {
                        f = s * rv1[i];
                        rv1[i] = c * rv1[i];

                        if ( std::fabs( f ) + anorm == anorm )
                        { break; }

                        g = w[i][i];
                        h = std::hypot( f, g );
                        w[i][i] = h;
                        h = one / h;
                        c = g * h;
                        s = -f * h;

                        for ( j = 0; j < m; j++ )
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
                        std::for_each( v.col_begin(k), v.col_end(k), [](value_type& v){ v = -v; } );
                    }

                    break;
                } // if l == k

                if ( (its+1) == max_its )
                { assert( !"no convergence in 30 svdcmp iterations" ); }

                value_type x = w[l][l];
                value_type y = w[k-1][k-1];
                g = rv1[k-1];
                h = rv1[k];
                f = ( ( y - z ) * ( y + z ) + ( g - h ) * ( g + h ) ) / ( 2.0 * h * y );
                g = std::hypot( f, one );
                g = ( f >= zero ) ? g : -g;
                f = ( ( x - z ) * ( x + z ) + h * ( ( y / ( f + g ) ) ) - h ) / x;
                c = s = one;

                for ( j = l; j <= k-1; j++ )
                {
                    i = j + 1;
                    g = rv1[i];
                    y = w[i][i];
                    h = s * g;
                    g = c * g;
                    z = std::hypot( f, h );
                    rv1[j] = z;
                    c = f / z;
                    s = h / z;
                    f = x * c + g * s;
                    g = g * c - x * s;
                    h = y * s;
                    y *= c;

                    for ( jj = 0; jj < n; jj++ )
                    {
                        const value_type tmp1 = v[jj][j];
                        const value_type tmp2 = v[jj][i];
                        v[jj][j] = tmp1 * c + tmp2 * s;
                        v[jj][i] = tmp2 * c - tmp1 * s;
                    } // jj loop

                    z = std::hypot( f, h );
                    w[j][j] = z;

                    if ( z )
                    {
                        z = one / z;
                        c = f * z;
                        s = h * z;
                    } // if z

                    f = c * g + s * y;
                    x = c * y - s * g;

                    for ( jj = 0; jj < m; jj++ )
                    {
                        const value_type tmp1 = u[jj][j];
                        const value_type tmp2 = u[jj][i];
                        u[jj][j] = tmp1 * c + tmp2 * s;
                        u[jj][i] = tmp2 * c - tmp1 * s;
                    } // jj loop
                }

                rv1[l] = zero;
                rv1[k] = f;
                w[k][k] = x;
            }
        }
    }

}//namespace sm


#endif//_SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI

