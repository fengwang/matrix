#ifndef _SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI
#define _SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI

#include <matrix/matrix.hpp>

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <valarray>
#include <vector>

namespace feng
{

    // A = U W V^{T}
    // Example
    // matrix<double> m( 13, 12 );
    // singular_value_decomposition<matrix<double> > svd( m );
    // auto u = svd.u();
    // auto v = svd.v();
    // auto w = svd.w();

    template < typename Matrix_Type, template<class> class Array = std::valarray >
    struct singular_value_decomposition
    {
        typedef Matrix_Type                         matrix_type;
        typedef typename matrix_type::size_type     size_type;
        typedef typename matrix_type::value_type    value_type;
        typedef Array<value_type>                   array_type;

        singular_value_decomposition( const matrix_type& a, const size_type max_iteration = 30 )
            : a_( a ), max_iteration_( max_iteration )
        { do_decomposition(); }

        const matrix_type
        u() const
        { return a_; }

        const matrix_type
        v() const
        { return v_; }

        const array_type
        w() const
        { return w_; }

    private:

        matrix_type a_;
        matrix_type v_;
        array_type w_;

        size_type max_iteration_;

    private:
        value_type
        sign( const value_type a, const value_type b )
        {
            return b > value_type( 0 ) ? std::fabs( a ) : std::fabs( b );
        }

    private:
        void do_decomposition()
        {
            const size_type m = a_.row();
            const size_type n = a_.col();
            assert( m >= n );
            v_.resize( n, n );
            w_.resize( n );
            size_type i, flag, its, j, jj, k, l, nm;
            value_type c, f, h, s, x, y, z;
            const value_type v0( 0 );
            const value_type v1( 1 );
            value_type anorm = v0;
            value_type g = v0;
            value_type scale = v0;
            value_type rv1[n];

            for ( i = 1; i <= n; i++ )
            {
                l = i + 1;
                rv1[i] = scale * g;
                g = s = scale = v0;

                if ( i < m )
                {
                    //scale += std::accumulate( a_.col_begin(i)+i, a_.col_end(i), value_type(), []( value_type x, value_type y ){ return x + std::abs(y); } );
                    for ( k = i; k <= m; k++ )
                        { scale += std::fabs( a_[k][i] ); }

                    if ( scale )
                    {
                        for ( k = i; k <= m; k++ )
                        {
                            a_[k][i] /= scale;
                            s += a_[k][i] * a_[k][i];
                        }

                        f = a_[i][i];
                        g = -sign( std::sqrt( s ), f );
                        h = f * g - s;
                        a_[i][i] = f - g;

                        if ( i != n )
                        {
                            for ( j = l; j <= n; j++ )
                            {
                                for ( s = v0, k = i; k <= m; k++ )
                                    { s += a_[k][i] * a_[k][j]; }

                                f = s / h;

                                for ( k = i; k <= m; k++ )
                                    { a_[k][j] += f * a_[k][i]; }
                            }
                        }

                        for ( k = i; k <= m; k++ )
                            { a_[k][i] *= scale; }
                    }
                }

                w_[i] = scale * g;
                g = s = scale = v0;

                if ( i <= m && i != n )
                {
                    for ( k = l; k <= n; k++ )
                        { scale += std::fabs( a_[i][k] ); }

                    if ( scale )
                    {
                        for ( k = l; k <= n; k++ )
                        {
                            a_[i][k] /= scale;
                            s += a_[i][k] * a_[i][k];
                        }

                        f = a_[i][l];
                        g = -sign( std::sqrt( s ), f );
                        h = f * g - s;
                        a_[i][l] = f - g;

                        for ( k = l; k <= n; k++ )
                            { rv1[k] = a_[i][k] / h; }

                        if ( i != m )
                        {
                            for ( j = l; j <= m; j++ )
                            {
                                for ( s = v0, k = l; k <= n; k++ )
                                    { s += a_[j][k] * a_[i][k]; }

                                for ( k = l; k <= n; k++ )
                                    { a_[j][k] += s * rv1[k]; }
                            }
                        }

                        for ( k = l; k <= n; k++ )
                            { a_[i][k] *= scale; }
                    }
                }

                anorm = std::max( anorm, ( std::fabs( w_[i] ) + std::fabs( rv1[i] ) ) );
            }

            for ( i = n; i >= 1; i-- )
            {
                if ( i < n )
                {
                    if ( g )
                    {
                        for ( j = l; j <= n; j++ )
                            { v_[j][i] = ( a_[i][j] / a_[i][l] ) / g; }

                        for ( j = l; j <= n; j++ )
                        {
                            for ( s = v0, k = l; k <= n; k++ )
                                { s += a_[i][k] * v_[k][j]; }

                            for ( k = l; k <= n; k++ )
                                { v_[k][j] += s * v_[k][i]; }
                        }
                    }

                    for ( j = l; j <= n; j++ )
                        { v_[i][j] = v_[j][i] = v0; }
                }

                v_[i][i] = v1;
                g = rv1[i];
                l = i;
            }

            for ( i = n; i >= 1; i-- )
            {
                l = i + 1;
                g = w_[i];

                if ( i < n )
                    for ( j = l; j <= n; j++ )
                        { a_[i][j] = v0; }

                if ( g )
                {
                    g = v1 / g;

                    if ( i != n )
                    {
                        for ( j = l; j <= n; j++ )
                        {
                            for ( s = v0, k = l; k <= m; k++ )
                                { s += a_[k][i] * a_[k][j]; }

                            f = ( s / a_[i][i] ) * g;

                            for ( k = i; k <= m; k++ )
                                { a_[k][j] += f * a_[k][i]; }
                        }
                    }

                    for ( j = i; j <= m; j++ )
                        { a_[j][i] *= g; }
                }
                else
                {
                    for ( j = i; j <= m; j++ )
                        { a_[j][i] = v0; }
                }

                ++a_[i][i];
            }

            for ( k = n; k >= 1; k-- )
            {
                for ( its = 1; its <= max_iteration_; its++ )
                {
                    flag = 1;

                    for ( l = k; l >= 1; l-- )
                    {
                        nm = l - 1;

                        if ( std::fabs( rv1[l] ) + anorm == anorm )
                        {
                            flag = 0;
                            break;
                        }

                        if ( std::fabs( w_[nm] ) + anorm == anorm )
                            { break; }
                    }

                    if ( flag )
                    {
                        c = v0;
                        s = v1;

                        for ( i = l; i <= k; i++ )
                        {
                            f = s * rv1[i];

                            if ( std::fabs( f ) + anorm != anorm )
                            {
                                g = w_[i];
                                h = hypot( f, g );
                                w_[i] = h;
                                h = v1 / h;
                                c = g * h;
                                s = ( -f * h );

                                for ( j = 1; j <= m; j++ )
                                {
                                    y = a_[j][nm];
                                    z = a_[j][i];
                                    a_[j][nm] = y * c + z * s;
                                    a_[j][i] = z * c - y * s;
                                }
                            }
                        }
                    }

                    z = w_[k];

                    if ( l == k )
                    {
                        if ( z < v0 )
                        {
                            w_[k] = -z;

                            for ( j = 1; j <= n; j++ )
                                { v_[j][k] = ( -v_[j][k] ); }
                        }

                        break;
                    }

                    if ( its == max_iteration_ )
                        { assert( !"No convergence in max_iteration_ SVDCMP iterations" ); }

                    x = w_[l];
                    nm = k - 1;
                    y = w_[nm];
                    g = rv1[nm];
                    h = rv1[k];
                    f = ( ( y - z ) * ( y + z ) + ( g - h ) * ( g + h ) ) / ( 2.0 * h * y );
                    g = hypot( f, v1 );
                    f = ( ( x - z ) * ( x + z ) + h * ( ( y / ( f + sign( g, f ) ) ) - h ) ) / x;
                    c = s = v1;

                    for ( j = l; j <= nm; j++ )
                    {
                        i = j + 1;
                        g = rv1[i];
                        y = w_[i];
                        h = s * g;
                        g = c * g;
                        z = hypot( f, h );
                        rv1[j] = z;
                        c = f / z;
                        s = h / z;
                        f = x * c + g * s;
                        g = g * c - x * s;
                        h = y * s;
                        y = y * c;

                        for ( jj = 1; jj <= n; jj++ )
                        {
                            x = v_[jj][j];
                            z = v_[jj][i];
                            v_[jj][j] = x * c + z * s;
                            v_[jj][i] = z * c - x * s;
                        }

                        z = hypot( f, h );
                        w_[j] = z;

                        if ( z )
                        {
                            z = v1 / z;
                            c = f * z;
                            s = h * z;
                        }

                        f = ( c * g ) + ( s * y );
                        x = ( c * y ) - ( s * g );

                        for ( jj = 1; jj <= m; jj++ )
                        {
                            y = a_[jj][j];
                            z = a_[jj][i];
                            a_[jj][j] = y * c + z * s;
                            a_[jj][i] = z * c - y * s;
                        }
                    }

                    rv1[l] = v0;
                    rv1[k] = f;
                    w_[k] = x;
                }
            }
        }
    };


}//namespace sm


#endif//_SVD_HPP_INCLUDED_OFSIDHJLKJ34OAHFD98HU398UHYAFOSOIUH3IUHD98U4EIJDFIJSDI

