// S9-R4 (PR-12, D-032): span access to the storage and rows; std::mdspan and std::submdspan adapters where the
// library has them. In C++20 (macros unset) the gated adapters are not declared, and make_view /
// make_mutable_view plus row_span give the same extents and elements.
#include "./s2_death.hpp"

#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>
#include <version>

namespace s9_r4
{
    template < typename M >
    concept has_to_mdspan = requires( M& m ) { to_mdspan( m ); };

    template < typename M >
    concept has_submdspan = requires( M& m ) { submdspan( m, { 0, 1 }, { 0, 1 } ); };

    // a 3x4 matrix holding 10*r + c
    inline feng::matrix< double > grid()
    {
        feng::matrix< double > m{ 3, 4 };
        for ( std::size_t r = 0; r != 3; ++r )
            for ( std::size_t c = 0; c != 4; ++c )
                m[r][c] = static_cast< double >( 10 * r + c );
        return m;
    }
}

TEST_CASE( "S9-R4 span adapters alias the storage and rows", "[S9][S9-R4]" )
{
    auto m = s9_r4::grid();
    auto const& cm = m;

    static_assert( std::is_same_v< decltype( feng::as_span( m ) ), std::span< double > > );
    static_assert( std::is_same_v< decltype( feng::as_span( cm ) ), std::span< double const > > );
    static_assert( std::is_same_v< decltype( feng::row_span( m, 0 ) ), std::span< double > > );
    static_assert( std::is_same_v< decltype( feng::row_span( cm, 0 ) ), std::span< double const > > );

    auto const all = feng::as_span( m );
    REQUIRE( all.data() == m.data() );
    REQUIRE( all.size() == m.size() );
    REQUIRE( feng::as_span( cm ).data() == m.data() );
    for ( std::size_t r = 0; r != 3; ++r )
    {
        auto const row = feng::row_span( cm, r );
        REQUIRE( row.size() == 4 );
        REQUIRE( row.data() == m.data() + r * 4 );
        for ( std::size_t c = 0; c != 4; ++c )
            REQUIRE( row[c] == m[r][c] );
    }
    feng::row_span( m, 1 )[2] = -1.0;
    REQUIRE( m[1][2] == -1.0 );
    all[0] = -2.0;
    REQUIRE( m[0][0] == -2.0 );

    feng::matrix< double > empty;
    REQUIRE( feng::as_span( empty ).empty() );

    SECTION( "row_span outside the matrix aborts with one message" )
    {
        S2_REQUIRE_DEATH( [&] { auto s = feng::row_span( cm, 3 ); (void)s; }, "row_span: row 3 outside a matrix with 3 rows" );
    }
}

#if defined( __cpp_lib_mdspan )
TEST_CASE( "S9-R4 to_mdspan has the matrix extents, layout_right strides and aliases it", "[S9][S9-R4]" )
{
    auto m = s9_r4::grid();
    auto const& cm = m;
    static_assert( s9_r4::has_to_mdspan< feng::matrix< double > > );
    static_assert( std::is_same_v< decltype( feng::to_mdspan( m ) ), std::mdspan< double, std::dextents< std::size_t, 2 > > > );
    static_assert( std::is_same_v< decltype( feng::to_mdspan( cm ) ), std::mdspan< double const, std::dextents< std::size_t, 2 > > > );
    auto const md = feng::to_mdspan( m );
    REQUIRE( md.extent( 0 ) == 3 );
    REQUIRE( md.extent( 1 ) == 4 );
    REQUIRE( md.stride( 0 ) == 4 );
    REQUIRE( md.stride( 1 ) == 1 );
    REQUIRE( md.data_handle() == m.data() );
    for ( std::size_t r = 0; r != 3; ++r )
        for ( std::size_t c = 0; c != 4; ++c )
            REQUIRE( &md[r, c] == &m[r][c] );
    md[2, 3] = -5.0;
    REQUIRE( m[2][3] == -5.0 );
    REQUIRE( feng::to_mdspan( cm )[2, 3] == -5.0 );
}
#endif

#if defined( __cpp_lib_submdspan )
TEST_CASE( "S9-R4 submdspan slices the matrix after the view validation", "[S9][S9-R4]" )
{
    auto m = s9_r4::grid();
    auto const& cm = m;
    static_assert( s9_r4::has_submdspan< feng::matrix< double > > );
    auto const s = feng::submdspan( m, { 1, 3 }, { 1, 4 } );
    REQUIRE( s.extent( 0 ) == 2 );
    REQUIRE( s.extent( 1 ) == 3 );
    REQUIRE( s.stride( 0 ) == 4 );
    REQUIRE( s.stride( 1 ) == 1 );
    for ( std::size_t r = 0; r != 2; ++r )
        for ( std::size_t c = 0; c != 3; ++c )
            REQUIRE( &s[r, c] == &m[r + 1][c + 1] );
    auto const cs = feng::submdspan( cm, { 0, 0 }, { 0, 4 } );
    static_assert( std::is_const_v< std::remove_reference_t< decltype( cs[0, 0] ) > > );
    REQUIRE( cs.extent( 0 ) == 0 );
    REQUIRE( cs.extent( 1 ) == 4 );

    SECTION( "an invalid slice aborts" )
    {
        S2_REQUIRE_DEATH( [&] { auto x = feng::submdspan( cm, { 0, 4 }, { 0, 1 } ); (void)x; }, "matrix view: row range [0, 4)" );
        S2_REQUIRE_DEATH( [&] { auto x = feng::submdspan( cm, { 2, 1 }, { 0, 1 } ); (void)x; }, "matrix view: row range [2, 1)" );
        S2_REQUIRE_DEATH( [&] { auto x = feng::submdspan( cm, { 0, 1 }, { 0, 5 } ); (void)x; }, "matrix view: column range [0, 5)" );
    }
}
#endif

#if !defined( __cpp_lib_mdspan ) || !defined( __cpp_lib_submdspan )
TEST_CASE( "S9-R4 C++20 fallback: views and row_span give the same extents and elements", "[S9][S9-R4]" )
{
#if !defined( __cpp_lib_mdspan )
    static_assert( !s9_r4::has_to_mdspan< feng::matrix< double > > );
#endif
    static_assert( !s9_r4::has_submdspan< feng::matrix< double > > );
    auto m = s9_r4::grid();
    auto const& cm = m;

    auto const v = feng::make_view( cm, { 0, 3 }, { 0, 4 } ); // the whole matrix, as to_mdspan( m )
    REQUIRE( v.row() == 3 );
    REQUIRE( v.col() == 4 );
    for ( std::size_t r = 0; r != 3; ++r )
    {
        auto const row = feng::row_span( cm, r );
        for ( std::size_t c = 0; c != 4; ++c )
        {
            REQUIRE( v( r, c ) == m[r][c] );
            REQUIRE( row[c] == v( r, c ) );
        }
    }

    auto const mv = feng::make_mutable_view( m, { 1, 3 }, { 1, 4 } ); // as submdspan( m, {1, 3}, {1, 4} )
    REQUIRE( mv.row() == 2 );
    REQUIRE( mv.col() == 3 );
    for ( std::size_t r = 0; r != 2; ++r )
        for ( std::size_t c = 0; c != 3; ++c )
            REQUIRE( &mv( r, c ) == &feng::row_span( m, r + 1 )[c + 1] );
}
#endif
