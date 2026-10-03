// S4-R3 (PR-6): view ranges are validated in every build, never normalized (F06).
#include "./s2_death.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

TEST_CASE( "S4 view ranges outside the owner abort", "[S4][S4-R3]" )
{
    feng::matrix<double> m{ 4, 5 };
    auto const& cm = m;

    SECTION( "make_view aborts on a range outside the owner" )
    {
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 5 }, { 0, 1 } ); (void)v; }, "matrix view:" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 3, 2 }, { 0, 1 } ); (void)v; }, "matrix view:" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 1 }, { 0, 6 } ); (void)v; }, "matrix view:" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 1, 2 }, { 0, 1 } ); (void)v; }, "matrix view:" );
    }
    SECTION( "make_mutable_view aborts on a range outside the owner" )
    {
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 5 }, { 0, 1 } ); (void)v; }, "matrix view:" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 3, 2 }, { 0, 1 } ); (void)v; }, "matrix view:" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 1 }, { 0, 6 } ); (void)v; }, "matrix view:" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 1 }, { 0, 1, 2 } ); (void)v; }, "matrix view:" );
    }
    SECTION( "an empty row range gives an empty view" )
    {
        auto const v = feng::make_view( cm, { 2, 2 }, { 0, 5 } );
        REQUIRE( v.row() == 0 );
        REQUIRE( v.col() == 5 );
        REQUIRE( v.size() == 0 );
        REQUIRE( v.begin() == v.end() );
        auto const w = feng::make_mutable_view( m, { 2, 2 }, { 0, 5 } );
        REQUIRE( w.row() == 0 );
        REQUIRE( w.col() == 5 );
        REQUIRE( w.begin() == w.end() );
        feng::matrix<double> const n{ v };
        REQUIRE( n.size() == 0 );
    }
}

TEST_CASE( "S4 every view factory and constructor rejects ranges outside the owner", "[S4][S4-R3]" )
{
    // R = 4 rows, C = 5 columns
    using range = std::pair<std::size_t, std::size_t>;
    using cview = feng::matrix_view<double, std::allocator<double>>;
    using mview = feng::mutable_matrix_view<double, std::allocator<double>>;
    feng::matrix<double> m{ 4, 5 };
    auto const& cm = m;

    SECTION( "make_view: r0 > r1, r1 > R, c0 > c1, c1 > C" )
    {
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 3, 2 }, { 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 5 }, { 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 4 }, { 3, 2 } ); (void)v; }, "matrix view: column range" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 4 }, { 0, 6 } ); (void)v; }, "matrix view: column range" );
    }
    SECTION( "make_mutable_view: r0 > r1, r1 > R, c0 > c1, c1 > C" )
    {
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 3, 2 }, { 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 5 }, { 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 4 }, { 3, 2 } ); (void)v; }, "matrix view: column range" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 4 }, { 0, 6 } ); (void)v; }, "matrix view: column range" );
    }
    SECTION( "matrix_view constructor: r0 > r1, r1 > R, c0 > c1, c1 > C" )
    {
        S2_REQUIRE_DEATH( [&] { cview v( cm, range{ 3, 2 }, range{ 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { cview v( cm, range{ 0, 5 }, range{ 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { cview v( cm, range{ 0, 4 }, range{ 3, 2 } ); (void)v; }, "matrix view: column range" );
        S2_REQUIRE_DEATH( [&] { cview v( cm, range{ 0, 4 }, range{ 0, 6 } ); (void)v; }, "matrix view: column range" );
    }
    SECTION( "mutable_matrix_view constructor: r0 > r1, r1 > R, c0 > c1, c1 > C" )
    {
        S2_REQUIRE_DEATH( [&] { mview v( m, range{ 3, 2 }, range{ 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { mview v( m, range{ 0, 5 }, range{ 0, 5 } ); (void)v; }, "matrix view: row range" );
        S2_REQUIRE_DEATH( [&] { mview v( m, range{ 0, 4 }, range{ 3, 2 } ); (void)v; }, "matrix view: column range" );
        S2_REQUIRE_DEATH( [&] { mview v( m, range{ 0, 4 }, range{ 0, 6 } ); (void)v; }, "matrix view: column range" );
    }
    SECTION( "factory lists of size 0, 1 and 3" )
    {
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, {}, { 0, 1 } ); (void)v; }, "matrix view: the row range needs exactly two values, got 0" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 1 }, { 0, 1 } ); (void)v; }, "matrix view: the row range needs exactly two values, got 1" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 1, 2 }, { 0, 1 } ); (void)v; }, "matrix view: the row range needs exactly two values, got 3" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 1 }, {} ); (void)v; }, "matrix view: the column range needs exactly two values, got 0" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 1 }, { 1 } ); (void)v; }, "matrix view: the column range needs exactly two values, got 1" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_view( cm, { 0, 1 }, { 0, 1, 2 } ); (void)v; }, "matrix view: the column range needs exactly two values, got 3" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, {}, { 0, 1 } ); (void)v; }, "matrix view: the row range needs exactly two values, got 0" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 1 }, { 0, 1 } ); (void)v; }, "matrix view: the row range needs exactly two values, got 1" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 1, 2 }, { 0, 1 } ); (void)v; }, "matrix view: the row range needs exactly two values, got 3" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 1 }, {} ); (void)v; }, "matrix view: the column range needs exactly two values, got 0" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 1 }, { 1 } ); (void)v; }, "matrix view: the column range needs exactly two values, got 1" );
        S2_REQUIRE_DEATH( [&] { auto v = feng::make_mutable_view( m, { 0, 1 }, { 0, 1, 2 } ); (void)v; }, "matrix view: the column range needs exactly two values, got 3" );
    }
    SECTION( "empty ranges r0 == r1 and c0 == c1 give empty views; the owner's limits are accepted" )
    {
        auto const require_empty = []( auto const& v, std::size_t rows, std::size_t cols )
        {
            REQUIRE( v.row() == rows );
            REQUIRE( v.col() == cols );
            REQUIRE( v.size() == 0 );
            REQUIRE( v.begin() == v.end() );
            REQUIRE( std::distance( v.begin(), v.end() ) == 0 );
        };
        for ( std::size_t i = 0; i <= 4; ++i )
        {
            INFO( "row range [" << i << ", " << i << ")" );
            int const ii = static_cast<int>( i );
            require_empty( feng::make_view( cm, { ii, ii }, { 0, 5 } ), 0, 5 );
            require_empty( feng::make_mutable_view( m, { ii, ii }, { 0, 5 } ), 0, 5 );
            require_empty( cview( cm, range{ i, i }, range{ 0, 5 } ), 0, 5 );
            require_empty( mview( m, range{ i, i }, range{ 0, 5 } ), 0, 5 );
        }
        for ( std::size_t j = 0; j <= 5; ++j )
        {
            INFO( "column range [" << j << ", " << j << ")" );
            int const jj = static_cast<int>( j );
            require_empty( feng::make_view( cm, { 0, 4 }, { jj, jj } ), 4, 0 );
            require_empty( feng::make_mutable_view( m, { 0, 4 }, { jj, jj } ), 4, 0 );
            require_empty( cview( cm, range{ 0, 4 }, range{ j, j } ), 4, 0 );
            require_empty( mview( m, range{ 0, 4 }, range{ j, j } ), 4, 0 );
        }
        REQUIRE( feng::make_view( cm, { 0, 4 }, { 0, 5 } ).size() == 20 );
        REQUIRE( mview( m, range{ 3, 4 }, range{ 4, 5 } ).size() == 1 );
    }
}
