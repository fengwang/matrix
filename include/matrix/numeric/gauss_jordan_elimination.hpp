//this file is to be removed

#ifndef _MATRIX_NUMERIC_ALGORITHM_HPP_INCLUDED
#define _MATRIX_NUMERIC_ALGORITHM_HPP_INCLUDED

#include <matrix/matrix.hpp>

#include <cassert>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cstring>

namespace feng
{

    //
    // Solve Equation Ax=b for A[n:n], x[n,m], b[n,m]
    // Example:
    //    matrix<double> A(n,n);
    //    matrix<double> b(n,m);
    //
    //    auto x = gaussian_elimination(A,b);
    //
    template< typename Matrix1, typename Matrix2 >
    const Matrix1
    gauss_jordan_elimination( const Matrix1& A, const Matrix2& b )
    {
        typedef Matrix1                             matrix_type;
        typedef typename matrix_type::size_type     size_type;
        typedef typename matrix_type::value_type    value_type;
        typedef typename matrix_type::range_type    range_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        matrix_type a( A || b ); // a -- [A:b]
        const size_type n = b.row();
        const size_type m = b.col();

        for ( size_type i = 0; i < n; ++i )
        {
            //find max element
            const size_type
            p = std::distance( a.col_begin( i ), 
                               std::max_element( a.col_begin( i ) + i, a.col_end( i ), []( value_type x, value_type y ) { return std::abs( x ) < std::abs( y );} ) );

            //swap row i and row p
            if ( p != i )
            { std::swap_ranges( a.row_begin( i ) + i, a.row_end( i ), a.row_begin( p ) + i ); }

            const value_type factor = a[i][i];
            assert( factor != value_type() );
            //eliminate
            std::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, [factor]( value_type & x ) { x /= factor; } );

            for ( size_type j = 0; j < n; ++j )
            {
                if ( i == j )
                { continue; }

                const value_type ratio = a[j][i];
                std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), 
                                [ratio]( value_type x, value_type y ) { return x - y * ratio; } );
            }
        }

        return matrix_type( a, range_type( 0, n ), range_type( n, m + n ) );
    }

    //
    // Solve Equation Ax=b for A[n:n], x[n], b[n]
    // Example:
    //    matrix<double> A(n,n); // A is known
    //    vector<double> b(n);   // b is known
    //    vector<double> x(n);   // x is unknown
    //    ..............
    //    gaussian_jordan_elimination(A, b.begin(), b.end(), x.begin() );
    //
    template< typename Matrix, typename II, typename OI >
    OI
    gauss_jordan_elimination( const Matrix& A, II b_, II _b, OI o_ )
    {
        const std::size_t size = std::distance( b_, _b );
        assert( A.row() == size );
        Matrix b( size, 1 );
        std::copy( b_, _b, b.begin() );
        auto x = gauss_jordan_elimination( A, b );
        return std::copy( x.begin(), x.end(), o_ );
    }

}//namespace feng

#endif

