// S3-R4 (PR-5): matrix from a view copies exactly the viewed rectangle (F03).
#include <cstddef>

namespace s3_r4
{
    inline feng::matrix<double> distinct( std::size_t r, std::size_t c )
    {
        feng::matrix<double> m{ r, c };
        for ( std::size_t i = 0; i != r; ++i )
            for ( std::size_t j = 0; j != c; ++j )
                m[i][j] = static_cast<double>( 100 * i + j ) + 0.5;
        return m;
    }

    inline void require_rectangle( feng::matrix<double> const& m, std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1 )
    {
        auto const view = feng::make_view( m, { r0, r1 }, { c0, c1 } );
        feng::matrix<double> const out{ view };
        REQUIRE( out.row() == r1 - r0 );
        REQUIRE( out.col() == c1 - c0 );
        REQUIRE( out.size() == out.row() * out.col() );
        REQUIRE( static_cast< std::size_t >( out.end() - out.begin() ) == out.size() );
        auto it = out.begin();
        for ( std::size_t i = r0; i != r1; ++i )
            for ( std::size_t j = c0; j != c1; ++j )
                REQUIRE( *it++ == m[i][j] );
    }
}

TEST_CASE( "S3 matrix from an offset view copies exactly the rectangle", "[S3][S3-R4]" )
{
    auto const m = s3_r4::distinct( 5, 6 );
    SECTION( "2x3 view at offset (1, 2)" )
    {
        auto const view = feng::make_view( m, { 1UL, 3UL }, { 2UL, 5UL } );
        feng::matrix<double> const out{ view };
        REQUIRE( out.row() == 2 );
        REQUIRE( out.col() == 3 );
        REQUIRE( out.size() == 6 );
        double const expected[] = { 102.5, 103.5, 104.5, 202.5, 203.5, 204.5 };
        std::size_t k = 0;
        for ( auto const& v : out ) REQUIRE( v == expected[k++] );
        REQUIRE( k == 6 );
    }
    SECTION( "1x1 view" ) { s3_r4::require_rectangle( m, 3, 4, 4, 5 ); }
    SECTION( "full-size view" ) { s3_r4::require_rectangle( m, 0, 5, 0, 6 ); }
    SECTION( "last-column view" ) { s3_r4::require_rectangle( m, 0, 5, 5, 6 ); }
    SECTION( "first-row view" ) { s3_r4::require_rectangle( m, 0, 1, 0, 6 ); }
    SECTION( "interior 1xN view" ) { s3_r4::require_rectangle( m, 4, 5, 1, 5 ); }
    SECTION( "interior Nx1 view" ) { s3_r4::require_rectangle( m, 1, 4, 0, 1 ); }
}
