// S2-R3 (PR-4): crop/pad copies the overlapping block exactly; flips are exact (D-004 axes).
#include "./s2_death.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace s2_r3
{
    inline feng::matrix<double> distinct( std::size_t rows, std::size_t cols )
    {
        feng::matrix<double> m{ rows, cols };
        for ( std::size_t r = 0; r != rows; ++r )
            for ( std::size_t c = 0; c != cols; ++c )
                m[r][c] = static_cast<double>( r * cols + c + 1 );
        return m;
    }

    inline bool same( feng::matrix<double> const& a, feng::matrix<double> const& b )
    {
        if ( a.row() != b.row() || a.col() != b.col() ) return false;
        for ( std::size_t r = 0; r != a.row(); ++r )
            for ( std::size_t c = 0; c != a.col(); ++c )
                if ( a[r][c] != b[r][c] ) return false;
        return true;
    }

    inline std::vector< std::pair< std::size_t, std::size_t > > flip_shapes()
    {
        return { { 3, 5 }, { 5, 3 }, { 4, 4 }, { 1, 1 }, { 0, 0 }, { 0, 3 }, { 3, 0 } };
    }

    inline void check_shrink( std::size_t r0, std::size_t c0, std::size_t r1, std::size_t c1 )
    {
        feng::matrix<double> const original = distinct( r0, c0 );
        feng::matrix<double> m = original;
        m.shrink_to_size( r1, c1 );
        REQUIRE( m.row() == r1 );
        REQUIRE( m.col() == c1 );
        for ( std::size_t r = 0; r != r1; ++r )
            for ( std::size_t c = 0; c != c1; ++c )
            {
                INFO( r0 << "x" << c0 << " -> " << r1 << "x" << c1 << ", r = " << r << ", c = " << c );
                double const expected = ( r < r0 && c < c0 ) ? original[r][c] : 0.0;
                REQUIRE( m[r][c] == expected );
            }
    }
}

TEST_CASE( "S2 shrink_to_size 5x5 to 5x3 is exact", "[S2][S2-R3]" )
{
    feng::matrix<double> m{ 5, 5 };
    for ( std::size_t r = 0; r != 5; ++r )
        for ( std::size_t c = 0; c != 5; ++c )
            m[r][c] = static_cast<double>( r * 5 + c );
    m.shrink_to_size( 5, 3 );
    REQUIRE( m.row() == 5 );
    REQUIRE( m.col() == 3 );
    for ( std::size_t r = 0; r != 5; ++r )
        for ( std::size_t c = 0; c != 3; ++c )
        {
            INFO( "r = " << r << ", c = " << c );
            REQUIRE( m[r][c] == static_cast<double>( r * 5 + c ) );
        }
}

TEST_CASE( "S2 shrink_to_size crops and pads exactly", "[S2][S2-R3]" )
{
    s2_r3::check_shrink( 5, 5, 5, 3 );
    s2_r3::check_shrink( 3, 10, 5, 2 );
    s2_r3::check_shrink( 2, 3, 4, 5 );
}

TEST_CASE( "S2 shrink_to_size aborts on a zero extent", "[S2][S2-R3]" )
{
    S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 2, 2 }; m.shrink_to_size( 0, 2 ); }, "shrink_to_size" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 2, 2 }; m.shrink_to_size( 2, 0 ); }, "shrink_to_size" );
}

TEST_CASE( "S2 flipdim 1 reverses the rows", "[S2][S2-R3]" )
{
    for ( auto [rows, cols] : s2_r3::flip_shapes() )
    {
        INFO( rows << "x" << cols );
        feng::matrix<double> const m = s2_r3::distinct( rows, cols );
        feng::matrix<double> const f = feng::flipdim( m, 1 );
        REQUIRE( f.row() == rows );
        REQUIRE( f.col() == cols );
        for ( std::size_t r = 0; r != rows; ++r )
            for ( std::size_t c = 0; c != cols; ++c )
                REQUIRE( f[r][c] == m[rows - 1 - r][c] );
    }
}

TEST_CASE( "S2 flipdim 2 reverses the columns", "[S2][S2-R3]" )
{
    for ( auto [rows, cols] : s2_r3::flip_shapes() )
    {
        INFO( rows << "x" << cols );
        feng::matrix<double> const m = s2_r3::distinct( rows, cols );
        feng::matrix<double> const f = feng::flipdim( m, 2 );
        REQUIRE( f.row() == rows );
        REQUIRE( f.col() == cols );
        for ( std::size_t r = 0; r != rows; ++r )
            for ( std::size_t c = 0; c != cols; ++c )
                REQUIRE( f[r][c] == m[r][cols - 1 - c] );
    }
}

TEST_CASE( "S2 flipdim twice restores the input", "[S2][S2-R3]" )
{
    for ( auto [rows, cols] : s2_r3::flip_shapes() )
    {
        INFO( rows << "x" << cols );
        feng::matrix<double> const m = s2_r3::distinct( rows, cols );
        REQUIRE( s2_r3::same( feng::flipdim( feng::flipdim( m, 1 ), 1 ), m ) );
        REQUIRE( s2_r3::same( feng::flipdim( feng::flipdim( m, 2 ), 2 ), m ) );
    }
}

TEST_CASE( "S2 fliplr flips columns and flipud flips rows", "[S2][S2-R3]" )
{
    for ( auto [rows, cols] : s2_r3::flip_shapes() )
    {
        INFO( rows << "x" << cols );
        feng::matrix<double> const m = s2_r3::distinct( rows, cols );
        REQUIRE( s2_r3::same( feng::fliplr( m ), feng::flipdim( m, 2 ) ) );
        REQUIRE( s2_r3::same( feng::flipud( m ), feng::flipdim( m, 1 ) ) );
    }
    feng::matrix<double> const m = s2_r3::distinct( 2, 3 );
    feng::matrix<double> const lr = feng::fliplr( m );
    feng::matrix<double> const ud = feng::flipud( m );
    REQUIRE( lr[0][0] == 3.0 );
    REQUIRE( lr[1][2] == 4.0 );
    REQUIRE( ud[0][0] == 4.0 );
    REQUIRE( ud[1][2] == 3.0 );
}

TEST_CASE( "S2 flipdim aborts on another dim", "[S2][S2-R3]" )
{
    S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 2, 2 }; (void)feng::flipdim( m, 0 ); }, "flipdim" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 2, 2 }; (void)feng::flipdim( m, 3 ); }, "flipdim" );
}
