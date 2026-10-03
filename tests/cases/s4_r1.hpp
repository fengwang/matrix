// S4-R1 (PR-6): stride iterators with logical ends (F05); index checks in every build, address checks in the
// checked-iterator build (FENG_MATRIX_CHECKED_ITERATORS, D-020).
#include "./s2_death.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

namespace s4_r1
{
    inline feng::matrix<double> numbered( std::size_t r, std::size_t c )
    {
        feng::matrix<double> m{ r, c };
        for ( std::size_t i = 0; i != r; ++i )
            for ( std::size_t j = 0; j != c; ++j )
                m( i, j ) = static_cast<double>( i * 10 + j );
        return m;
    }

    template < typename It >
    std::vector<double> walk( It first, It last )
    {
        std::vector<double> ans;
        for ( ; first != last; ++first ) ans.push_back( *first );
        return ans;
    }
}

TEST_CASE( "S4 one-column anti-diagonal traverses forward and backward", "[S4][S4-R1]" )
{
    SECTION( "5x1 anti-diagonals have length 1, forward and reverse" )
    {
        auto m = s4_r1::numbered( 5, 1 );
        for ( std::ptrdiff_t k = -4; k <= 0; ++k )
        {
            INFO( "k = " << k );
            REQUIRE( std::distance( m.anti_diag_begin( k ), m.anti_diag_end( k ) ) == 1 );
            REQUIRE( s4_r1::walk( m.anti_diag_begin( k ), m.anti_diag_end( k ) ) == std::vector<double>{ m( static_cast<std::size_t>( -k ), 0 ) } );
            REQUIRE( s4_r1::walk( m.anti_diag_rbegin( k ), m.anti_diag_rend( k ) ) == std::vector<double>{ m( static_cast<std::size_t>( -k ), 0 ) } );
            REQUIRE( std::distance( m.anti_diag_crbegin( k ), m.anti_diag_crend( k ) ) == 1 );
        }
    }
    SECTION( "the last column of a 3x4 matrix has distance 3, forward and reverse" )
    {
        auto m = s4_r1::numbered( 3, 4 );
        REQUIRE( std::distance( m.col_begin( 3 ), m.col_end( 3 ) ) == 3 );
        REQUIRE( s4_r1::walk( m.col_begin( 3 ), m.col_end( 3 ) ) == std::vector<double>{ 3.0, 13.0, 23.0 } );
        REQUIRE( s4_r1::walk( m.col_rbegin( 3 ), m.col_rend( 3 ) ) == std::vector<double>{ 23.0, 13.0, 3.0 } );
        auto const& cm = m;
        REQUIRE( s4_r1::walk( cm.col_cbegin( 3 ), cm.col_cend( 3 ) ) == std::vector<double>{ 3.0, 13.0, 23.0 } );
    }
    SECTION( "diagonals of a 3x4 matrix match the index oracle" )
    {
        auto m = s4_r1::numbered( 3, 4 );
        for ( std::ptrdiff_t k = -2; k <= 3; ++k )
        {
            std::vector<double> diag, anti;
            for ( std::ptrdiff_t r = 0; r != 3; ++r )
            {
                std::ptrdiff_t const c = r + k;
                if ( c >= 0 && c < 4 ) diag.push_back( m( static_cast<std::size_t>( r ), static_cast<std::size_t>( c ) ) );
            }
            // anti-diagonal k: k > 0 starts at (0, 3-k), k <= 0 at (-k, 3); each step goes one row down, one column left
            std::ptrdiff_t r = k > 0 ? 0 : -k, c = k > 0 ? 3 - k : 3;
            for ( ; r < 3 && c >= 0; ++r, --c ) anti.push_back( m( static_cast<std::size_t>( r ), static_cast<std::size_t>( c ) ) );
            INFO( "k = " << k );
            REQUIRE( s4_r1::walk( m.diag_begin( k ), m.diag_end( k ) ) == diag );
            REQUIRE( std::distance( m.diag_begin( k ), m.diag_end( k ) ) == static_cast<std::ptrdiff_t>( diag.size() ) );
            REQUIRE( s4_r1::walk( m.diag_rbegin( k ), m.diag_rend( k ) ) == std::vector<double>( diag.rbegin(), diag.rend() ) );
            REQUIRE( s4_r1::walk( m.anti_diag_begin( k ), m.anti_diag_end( k ) ) == anti );
            REQUIRE( s4_r1::walk( m.anti_diag_rbegin( k ), m.anti_diag_rend( k ) ) == std::vector<double>( anti.rbegin(), anti.rend() ) );
        }
    }
    SECTION( "diagonal 0 of an empty matrix is an empty range" )
    {
        feng::matrix<double> m;
        REQUIRE( m.diag_begin() == m.diag_end() );
        REQUIRE( m.anti_diag_begin() == m.anti_diag_end() );
    }
}

TEST_CASE( "S4 column and diagonal indices outside the matrix abort", "[S4][S4-R1]" )
{
    S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_begin( 4 ); (void)it; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.diag_begin( 4 ); (void)it; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.diag_begin( -3 ); (void)it; }, "matrix iterator" );
}

TEST_CASE( "S4 diag of an empty matrix with an offset keeps its result", "[S4][S4-R1]" )
{
    auto all_zero = []( feng::matrix<double> const& m ) { return std::all_of( m.begin(), m.end(), []( double x ) { return x == 0.0; } ); };
    feng::matrix<double> const empty;
    auto const d0 = feng::diag( empty, 0 );
    REQUIRE( d0.row() == 0 );
    REQUIRE( d0.col() == 0 );
    auto const d1 = feng::diag( empty, 1 );
    REQUIRE( d1.row() == 1 );
    REQUIRE( d1.col() == 1 );
    REQUIRE( all_zero( d1 ) );
    auto const dm2 = feng::diag( empty, -2 );
    REQUIRE( dm2.row() == 2 );
    REQUIRE( dm2.col() == 2 );
    REQUIRE( all_zero( dm2 ) );
    auto const v3 = feng::diag( std::vector<double>{}, 3 );
    REQUIRE( v3.row() == 3 );
    REQUIRE( v3.col() == 3 );
    REQUIRE( all_zero( v3 ) );

    auto const placed = feng::diag( s4_r1::numbered( 2, 2 ), 1 );
    REQUIRE( placed.row() == 3 );
    REQUIRE( placed.col() == 3 );
    for ( std::size_t i = 0; i != 3; ++i )
        for ( std::size_t j = 0; j != 3; ++j )
        {
            INFO( "(" << i << ", " << j << ")" );
            double const expected = ( j == i + 1 ) ? static_cast<double>( i * 11 ) : 0.0;
            REQUIRE( placed( i, j ) == expected );
        }
}

#ifdef FENG_MATRIX_CHECKED_ITERATORS
TEST_CASE( "S4 checked iterators abort on a past-the-end dereference", "[S4][S4-R1]" )
{
    S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); double volatile x = *m.col_end( 1 ); (void)x; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); double volatile x = m.col_begin( 0 )[3]; (void)x; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_begin( 0 ) - 1; (void)it; }, "matrix iterator" );
}
#endif

namespace s4_r1
{
    template < typename It >
    std::vector<double> walk_back( It first, It last )
    {
        std::vector<double> ans;
        while ( last != first ) ans.push_back( *--last );
        return ans;
    }

    // the oracle, independent of the library's range code: elements (i, j) of an r x c matrix with
    // pred(i, j), in increasing row order (each diagonal and anti-diagonal holds at most one element per row)
    template < typename Pred >
    std::vector<double> oracle( feng::matrix<double> const& m, Pred pred )
    {
        std::vector<double> ans;
        for ( std::ptrdiff_t i = 0; i != static_cast<std::ptrdiff_t>( m.row() ); ++i )
            for ( std::ptrdiff_t j = 0; j != static_cast<std::ptrdiff_t>( m.col() ); ++j )
                if ( pred( i, j ) ) ans.push_back( m( static_cast<std::size_t>( i ), static_cast<std::size_t>( j ) ) );
        return ans;
    }
}

// every variant of range family `fam` at argument `arg` of the matrix `m` (and its const alias `cm`) visits
// `expected` forward and its reverse backward, and every distance equals the oracle length
#define S4_R1_REQUIRE_RANGE( fam, arg, expected )                                                                   \
    do {                                                                                                          \
        std::vector<double> const ex_ = ( expected );                                                             \
        std::vector<double> const rev_( ex_.rbegin(), ex_.rend() );                                               \
        std::ptrdiff_t const n_ = static_cast<std::ptrdiff_t>( ex_.size() );                                      \
        INFO( #fam " " << ( arg ) );                                                                              \
        REQUIRE( s4_r1::walk( m.fam##_begin( arg ), m.fam##_end( arg ) ) == ex_ );                                \
        REQUIRE( s4_r1::walk_back( m.fam##_begin( arg ), m.fam##_end( arg ) ) == rev_ );                          \
        REQUIRE( std::distance( m.fam##_begin( arg ), m.fam##_end( arg ) ) == n_ );                               \
        REQUIRE( s4_r1::walk( cm.fam##_begin( arg ), cm.fam##_end( arg ) ) == ex_ );                              \
        REQUIRE( std::distance( cm.fam##_begin( arg ), cm.fam##_end( arg ) ) == n_ );                             \
        REQUIRE( s4_r1::walk( cm.fam##_cbegin( arg ), cm.fam##_cend( arg ) ) == ex_ );                            \
        REQUIRE( s4_r1::walk_back( cm.fam##_cbegin( arg ), cm.fam##_cend( arg ) ) == rev_ );                      \
        REQUIRE( std::distance( cm.fam##_cbegin( arg ), cm.fam##_cend( arg ) ) == n_ );                           \
        REQUIRE( s4_r1::walk( m.fam##_rbegin( arg ), m.fam##_rend( arg ) ) == rev_ );                             \
        REQUIRE( s4_r1::walk_back( m.fam##_rbegin( arg ), m.fam##_rend( arg ) ) == ex_ );                         \
        REQUIRE( std::distance( m.fam##_rbegin( arg ), m.fam##_rend( arg ) ) == n_ );                             \
        REQUIRE( s4_r1::walk( cm.fam##_rbegin( arg ), cm.fam##_rend( arg ) ) == rev_ );                           \
        REQUIRE( std::distance( cm.fam##_rbegin( arg ), cm.fam##_rend( arg ) ) == n_ );                           \
        REQUIRE( s4_r1::walk( cm.fam##_crbegin( arg ), cm.fam##_crend( arg ) ) == rev_ );                         \
        REQUIRE( std::distance( cm.fam##_crbegin( arg ), cm.fam##_crend( arg ) ) == n_ );                         \
    } while ( false )

TEST_CASE( "S4 every diagonal of every shape up to 4x5 traverses exactly", "[S4][S4-R1]" )
{
    for ( std::size_t r = 1; r <= 4; ++r )
        for ( std::size_t c = 1; c <= 5; ++c )
        {
            INFO( "shape " << r << "x" << c );
            auto m = s4_r1::numbered( r, c );
            auto const& cm = m;
            std::ptrdiff_t const R = static_cast<std::ptrdiff_t>( r ), C = static_cast<std::ptrdiff_t>( c );
            for ( std::size_t j = 0; j != c; ++j )
            {
                std::ptrdiff_t const jj = static_cast<std::ptrdiff_t>( j );
                S4_R1_REQUIRE_RANGE( col, j, s4_r1::oracle( m, [=]( std::ptrdiff_t, std::ptrdiff_t y ) { return y == jj; } ) );
            }
            for ( std::ptrdiff_t k = -( R - 1 ); k < C; ++k )
            {
                auto const diag = s4_r1::oracle( m, [=]( std::ptrdiff_t x, std::ptrdiff_t y ) { return y - x == k; } );
                auto const anti = s4_r1::oracle( m, [=]( std::ptrdiff_t x, std::ptrdiff_t y ) { return x + y == C - 1 - k; } );
                REQUIRE_FALSE( diag.empty() );
                REQUIRE_FALSE( anti.empty() );
                S4_R1_REQUIRE_RANGE( diag, k, diag );
                S4_R1_REQUIRE_RANGE( anti_diag, k, anti );
                std::size_t const u = static_cast<std::size_t>( k < 0 ? -k : k );
                if ( k >= 0 )
                {
                    S4_R1_REQUIRE_RANGE( upper_diag, u, diag );
                    S4_R1_REQUIRE_RANGE( upper_anti_diag, u, anti );
                }
                if ( k <= 0 )
                {
                    S4_R1_REQUIRE_RANGE( lower_diag, u, diag );
                    S4_R1_REQUIRE_RANGE( lower_anti_diag, u, anti );
                }
            }
            if ( c == 1 )
                for ( std::ptrdiff_t k = -( R - 1 ); k <= 0; ++k )
                    REQUIRE( std::distance( m.anti_diag_begin( k ), m.anti_diag_end( k ) ) == 1 );
        }
}

TEST_CASE( "S4 every range family rejects a column or diagonal index outside the matrix", "[S4][S4-R1]" )
{
    auto const run = []( auto f ) { S2_REQUIRE_DEATH( [f]{ auto m = s4_r1::numbered( 3, 4 ); f( m ); }, "matrix iterator" ); };
    SECTION( "column index col()" )
    {
        run( []( feng::matrix<double>& m ) { auto it = m.col_begin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = m.col_end( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).col_cbegin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = m.col_rbegin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).col_crend( 4 ); (void)it; } );
    }
    SECTION( "diagonal k = col() and k = -row()" )
    {
        run( []( feng::matrix<double>& m ) { auto it = m.diag_end( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).diag_cbegin( -3 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = m.diag_rbegin( -3 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).diag_crend( 4 ); (void)it; } );
    }
    SECTION( "upper and lower diagonal indices col() and row()" )
    {
        run( []( feng::matrix<double>& m ) { auto it = m.upper_diag_begin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).upper_diag_crbegin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = m.lower_diag_end( 3 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).lower_diag_cbegin( 3 ); (void)it; } );
    }
    SECTION( "anti-diagonal k = col() and k = -row()" )
    {
        run( []( feng::matrix<double>& m ) { auto it = m.anti_diag_begin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).anti_diag_cend( -3 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = m.anti_diag_rend( -3 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).anti_diag_crbegin( 4 ); (void)it; } );
    }
    SECTION( "upper and lower anti-diagonal indices col() and row()" )
    {
        run( []( feng::matrix<double>& m ) { auto it = m.upper_anti_diag_begin( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).upper_anti_diag_cend( 4 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = m.lower_anti_diag_rbegin( 3 ); (void)it; } );
        run( []( feng::matrix<double>& m ) { auto it = std::as_const( m ).lower_anti_diag_crend( 3 ); (void)it; } );
    }
}

#ifdef FENG_MATRIX_CHECKED_ITERATORS
TEST_CASE( "S4 checked iterators abort on every position or address outside the owner", "[S4][S4-R1]" )
{
    using it_t = feng::stride_iterator<double*>;
    SECTION( "dereference and subscript outside [0, count)" )
    {
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); double volatile x = *m.col_end( 0 ); (void)x; }, "matrix iterator: dereference" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); double volatile x = m.col_begin( 0 )[3]; (void)x; }, "matrix iterator: dereference" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); double volatile x = m.col_end( 0 )[-4]; (void)x; }, "matrix iterator: dereference" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); double* volatile p = m.diag_end().operator->(); (void)p; }, "matrix iterator: dereference" );
    }
    SECTION( "index moved outside [0, count]" )
    {
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_end( 1 ); ++it; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_end( 1 ); it++; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_begin( 1 ); --it; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_begin( 1 ); it--; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_begin( 1 ); it += 4; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.col_end( 1 ); it -= 4; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.anti_diag_begin() + 4; (void)it; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = 4 + m.anti_diag_begin(); (void)it; }, "matrix iterator: position" );
        S2_REQUIRE_DEATH( []{ auto m = s4_r1::numbered( 3, 4 ); auto it = m.diag_begin() - 1; (void)it; }, "matrix iterator: position" );
    }
    SECTION( "first or last address outside the owner, through the public constructor" )
    {
        // the owner is the first four elements of an eight-element buffer, so every address below is valid to form
        S2_REQUIRE_DEATH( []{ double buf[8] = {}; it_t it( buf + 5, 1, 1, 0, buf, 4 ); (void)it; }, "matrix iterator: first address outside" );
        S2_REQUIRE_DEATH( []{ double buf[8] = {}; it_t it( buf, 1, 1, 0, buf + 1, 3 ); (void)it; }, "matrix iterator: first address outside" );
        S2_REQUIRE_DEATH( []{ double buf[8] = {}; it_t it( buf + 4, 1, 1, 0, buf, 4 ); (void)it; }, "matrix iterator: first element" );
        S2_REQUIRE_DEATH( []{ double buf[8] = {}; it_t it( buf, 2, 3, 0, buf, 4 ); (void)it; }, "matrix iterator: last element" );
        S2_REQUIRE_DEATH( []{ double buf[8] = {}; it_t it( buf + 1, -1, 3, 0, buf, 4 ); (void)it; }, "matrix iterator: last element" );
        S2_REQUIRE_DEATH( []{ double buf[8] = {}; it_t it( buf, 1, 2, 3, buf, 4 ); (void)it; }, "matrix iterator: position" );
        // the legal limits: an empty range at data()+size() and a full range with stride 0
        double buf[8] = {};
        it_t const e( buf + 4, 1, 0, 0, buf, 4 );
        it_t const z( buf + 3, 0, 5, 0, buf, 4 );
        REQUIRE( e - e == 0 );
        REQUIRE( ( z + 5 ) - z == 5 );
    }
}
#endif
