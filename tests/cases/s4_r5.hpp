// S4-R5 (PR-6, PR-14): the stride iterators model the standard iterator concepts.
#include <algorithm>
#include <iterator>
#include <numeric>
#include <ranges>
#include <type_traits>

TEST_CASE( "S4 stride iterators satisfy the standard iterator concepts", "[S4][S4-R5]" )
{
    using it = feng::stride_iterator<double*>;
    using cit = feng::stride_iterator<double const*>;
    static_assert( std::random_access_iterator<it> );
    static_assert( std::random_access_iterator<cit> );
    static_assert( std::random_access_iterator<std::reverse_iterator<it>> );
    static_assert( std::random_access_iterator<std::reverse_iterator<cit>> );
    static_assert( std::sized_sentinel_for<it, it> );
    static_assert( std::sized_sentinel_for<cit, cit> );
    static_assert( std::sized_sentinel_for<std::reverse_iterator<it>, std::reverse_iterator<it>> );
    static_assert( std::sized_sentinel_for<std::reverse_iterator<cit>, std::reverse_iterator<cit>> );
    static_assert( std::output_iterator<it, double> );
    static_assert( std::is_convertible_v<it, cit> );

    feng::matrix<double> m{ 4, 3 };
    std::iota( m.begin(), m.end(), 0.0 );
    auto col = std::ranges::subrange( m.col_begin( 1 ), m.col_end( 1 ) );
    REQUIRE( std::ranges::size( col ) == 4 );
    REQUIRE( std::ranges::equal( col, std::vector<double>{ 1.0, 4.0, 7.0, 10.0 } ) );
    std::ranges::fill( col, -1.0 );
    REQUIRE( std::ranges::count( m, -1.0 ) == 4 );
    REQUIRE( m[2][1] == -1.0 );
    auto const& cm = m;
    auto ccol = std::ranges::subrange( cm.col_begin( 1 ), cm.col_end( 1 ) );
    REQUIRE( std::ranges::all_of( ccol, []( double x ) { return x == -1.0; } ) );
    REQUIRE( *std::ranges::max_element( std::ranges::subrange( cm.col_begin( 2 ), cm.col_end( 2 ) ) ) == 11.0 );
}

// S4-R5 (PR-6, PR-14): the view element iterator models the standard iterator concepts and both view types are
// borrowed random-access ranges; the owner is not borrowed.
TEST_CASE( "S4 view types are borrowed random-access ranges", "[S4][S4-R5]" )
{
    using A = feng::matrix<double>::allocator_type;
    using vit = feng::view_iterator<double*>;
    using cvit = feng::view_iterator<double const*>;
    using view = feng::matrix_view<double, A>;
    using mview = feng::mutable_matrix_view<double, A>;
    static_assert( std::random_access_iterator<vit> );
    static_assert( std::random_access_iterator<cvit> );
    static_assert( std::random_access_iterator<std::reverse_iterator<vit>> );
    static_assert( std::random_access_iterator<std::reverse_iterator<cvit>> );
    static_assert( std::sized_sentinel_for<vit, vit> );
    static_assert( std::sized_sentinel_for<cvit, cvit> );
    static_assert( std::sized_sentinel_for<std::reverse_iterator<vit>, std::reverse_iterator<vit>> );
    static_assert( std::sized_sentinel_for<std::reverse_iterator<cvit>, std::reverse_iterator<cvit>> );
    static_assert( std::output_iterator<vit, double> );
    static_assert( std::is_convertible_v<vit, cvit> );
    static_assert( std::ranges::random_access_range<view> );
    static_assert( std::ranges::random_access_range<mview> );
    static_assert( std::ranges::borrowed_range<view> );
    static_assert( std::ranges::borrowed_range<mview> );
    static_assert( !std::ranges::borrowed_range<feng::matrix<double>> );
    static_assert( std::is_convertible_v<mview, view> );

    feng::matrix<double> m{ 4, 5 };
    std::iota( m.begin(), m.end(), 0.0 );
    auto mv = feng::make_mutable_view( m, { 1, 3 }, { 1, 4 } );
    REQUIRE( std::ranges::size( mv ) == 6 );
    REQUIRE( std::ranges::equal( mv, std::vector<double>{ 6.0, 7.0, 8.0, 11.0, 12.0, 13.0 } ) );
    std::ranges::fill( mv, -1.0 );
    REQUIRE( std::ranges::count( m, -1.0 ) == 6 );
    REQUIRE( m( 2, 3 ) == -1.0 );
    REQUIRE( m( 2, 4 ) == 14.0 );
    view const v = mv;
    REQUIRE( std::ranges::all_of( v, []( double x ) { return x == -1.0; } ) );
    auto const& cm = m;
    auto const cv = feng::make_view( cm, { 0, 4 }, { 4, 5 } );
    REQUIRE( *std::ranges::max_element( cv ) == 19.0 );
    // a borrowed range: the iterator from a temporary view stays usable while the owner lives
    auto const it = std::ranges::find( feng::make_view( cm, { 3, 4 }, { 0, 5 } ), 17.0 );
    REQUIRE( *it == 17.0 );
    cvit const cit = mv.begin();
    REQUIRE( *cit == -1.0 );
}
