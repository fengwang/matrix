// S1-R4 (D-015): the F16 templates (matrix power, valarray products, clone overloads).
#include <cmath>
#include <cstddef>
#include <valarray>

namespace s1_f16
{
    // A small, well-conditioned matrix whose powers stay bounded (spectral radius below 1).
    inline feng::matrix<double> sample_square()
    {
        feng::matrix<double> a{ 3, 3 };
        double const v[3][3] = { { 0.50, 0.10, -0.20 }, { 0.05, 0.40, 0.10 }, { -0.10, 0.20, 0.30 } };
        for ( std::size_t r = 0; r != 3; ++r )
            for ( std::size_t c = 0; c != 3; ++c )
                a[r][c] = v[r][c];
        return a;
    }

    inline feng::matrix<double> sample_rect( std::size_t rows, std::size_t cols )
    {
        feng::matrix<double> a{ rows, cols };
        for ( std::size_t r = 0; r != rows; ++r )
            for ( std::size_t c = 0; c != cols; ++c )
                a[r][c] = static_cast<double>( 10 * r + c ) + 0.5;
        return a;
    }

    inline bool near( feng::matrix<double> const& x, feng::matrix<double> const& y, double tol = 1.0e-12 )
    {
        if ( x.row() != y.row() || x.col() != y.col() )
            return false;
        for ( std::size_t r = 0; r != x.row(); ++r )
            for ( std::size_t c = 0; c != x.col(); ++c )
                if ( std::abs( x[r][c] - y[r][c] ) > tol * ( 1.0 + std::abs( y[r][c] ) ) )
                    return false;
        return true;
    }

    inline bool equal_block( feng::matrix<double> const& block, feng::matrix<double> const& src,
                             std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1 )
    {
        if ( block.row() != r1 - r0 || block.col() != c1 - c0 )
            return false;
        for ( std::size_t r = r0; r != r1; ++r )
            for ( std::size_t c = c0; c != c1; ++c )
                if ( block[r - r0][c - c0] != src[r][c] )
                    return false;
        return true;
    }
} // namespace s1_f16

TEST_CASE( "S1 matrix power equals repeated multiplication", "[S1][S1-R4]" )
{
    auto const a = s1_f16::sample_square();
    auto const id = feng::eye<double>( 3, 3 );

    REQUIRE( s1_f16::near( a ^ 0, id ) );

    feng::matrix<double> expected{ id };
    for ( unsigned n = 1; n <= 13; ++n )
    {
        expected = expected * a;
        if ( n == 1 || n == 2 || n == 3 || n == 13 )
        {
            INFO( "n = " << n );
            REQUIRE( s1_f16::near( a ^ n, expected ) );
        }
    }
}

TEST_CASE( "S1 valarray products match explicit sums", "[S1][S1-R4]" )
{
    std::size_t const rows = 3, cols = 4;
    auto const m = s1_f16::sample_rect( rows, cols );

    // valarray x matrix: (1 x rows) * (rows x cols) -> 1 x cols
    std::valarray<double> const left = { 1.5, -2.0, 0.25 };
    auto const lm = left * m;
    REQUIRE( lm.row() == 1 );
    REQUIRE( lm.col() == cols );
    for ( std::size_t c = 0; c != cols; ++c )
    {
        double s = 0.0;
        for ( std::size_t r = 0; r != rows; ++r )
            s += left[r] * m[r][c];
        REQUIRE( std::abs( lm[0][c] - s ) < 1.0e-12 * ( 1.0 + std::abs( s ) ) );
    }

    // matrix x valarray: (rows x cols) * (cols x 1) -> rows x 1
    std::valarray<double> const right = { 0.5, 1.0, -1.0, 2.0 };
    auto const mr = m * right;
    REQUIRE( mr.row() == rows );
    REQUIRE( mr.col() == 1 );
    for ( std::size_t r = 0; r != rows; ++r )
    {
        double s = 0.0;
        for ( std::size_t c = 0; c != cols; ++c )
            s += m[r][c] * right[c];
        REQUIRE( std::abs( mr[r][0] - s ) < 1.0e-12 * ( 1.0 + std::abs( s ) ) );
    }
}

TEST_CASE( "S1 every clone overload returns the selected block", "[S1][S1-R4]" )
{
    auto const src = s1_f16::sample_rect( 5, 6 );
    std::size_t const r0 = 1, r1 = 4, c0 = 2, c1 = 5;

    feng::matrix<double> by_braces;
    by_braces.clone( src, { r0, r1 }, { c0, c1 } );
    REQUIRE( s1_f16::equal_block( by_braces, src, r0, r1, c0, c1 ) );

    feng::matrix<double> by_indices;
    by_indices.clone( src, r0, r1, c0, c1 );
    REQUIRE( s1_f16::equal_block( by_indices, src, r0, r1, c0, c1 ) );

    auto const const_braces = src.clone( { r0, r1 }, { c0, c1 } );
    REQUIRE( s1_f16::equal_block( const_braces, src, r0, r1, c0, c1 ) );

    auto const const_indices = src.clone( r0, r1, c0, c1 );
    REQUIRE( s1_f16::equal_block( const_indices, src, r0, r1, c0, c1 ) );
}
