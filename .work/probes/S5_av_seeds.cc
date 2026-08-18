// S5 adversarial verification probe: seed classes (0/1/RAND_MAX-era/UINT_MAX),
// the contract's literal 4x4 seed-7 check, empty/unit shapes, non-degeneracy.
// Build: g++ -std=c++20 -DPARALLEL -O1 -o .work/evidence/seed_S5_av .work/probes/S5_av_seeds.cc
#include "../../matrix.hpp"

#include <cstdio>
#include <limits>

static int failures = 0;

static void check( bool ok, char const* what )
{
    if ( ! ok )
    {
        std::printf( "FAIL: %s\n", what );
        ++failures;
    }
}

template < typename T >
static bool equal( feng::matrix<T> const& a, feng::matrix<T> const& b )
{
    for ( unsigned long r = 0; r < a.row(); ++r )
        for ( unsigned long c = 0; c < a.col(); ++c )
            if ( a[r][c] != b[r][c] )
                return false;
    return true;
}

template < typename T >
static bool in_range( feng::matrix<T> const& a )
{
    for ( unsigned long r = 0; r < a.row(); ++r )
        for ( unsigned long c = 0; c < a.col(); ++c )
            if ( a[r][c] < T( 0 ) || a[r][c] >= T( 1 ) )
                return false;
    return true;
}

// non-degeneracy: not a constant matrix (guards against a broken seed init)
template < typename T >
static bool non_degenerate( feng::matrix<T> const& a )
{
    T const v0 = a[0][0];
    for ( unsigned long r = 0; r < a.row(); ++r )
        for ( unsigned long c = 0; c < a.col(); ++c )
            if ( a[r][c] != v0 )
                return true;
    return false;
}

int main()
{
    // contract literal: rand(4,4,7) twice equal; 7 vs 8 differ; [0,1)
    auto const e14a = feng::rand< double >( 4, 4, 7 );
    auto const e14b = feng::rand< double >( 4, 4, 7 );
    auto const e14c = feng::rand< double >( 4, 4, 8 );
    check( e14a == e14b, "4x4 seed 7 twice equal (contract literal)" );
    check( !( e14a == e14c ), "4x4 seed 7 vs 8 differ (contract literal)" );
    check( in_range( e14a ), "4x4 seed 7 in [0,1)" );

    // seed 0: time-based mix — deterministic given the same (time, &ans) within one process;
    // non-degenerate; in range
    auto const s0 = feng::rand< double >( 32, 32, 0 );
    check( in_range( s0 ), "seed 0 in [0,1)" );
    check( non_degenerate( s0 ), "seed 0 non-degenerate" );

    // seed 1 (the examples' seed): deterministic within process, in range
    auto const s1a = feng::rand< double >( 8, 8, 1 );
    auto const s1b = feng::rand< double >( 8, 8, 1 );
    check( s1a == s1b, "seed 1 twice equal" );
    check( in_range( s1a ), "seed 1 in [0,1)" );
    check( non_degenerate( s1a ), "seed 1 non-degenerate" );

    // RAND_MAX-era large seed (2147483647) and the full unsigned range (UINT_MAX)
    auto const lm1a = feng::rand< double >( 16, 16, 2147483647u );
    auto const lm1b = feng::rand< double >( 16, 16, 2147483647u );
    check( lm1a == lm1b, "seed 2147483647 twice equal" );
    check( in_range( lm1a ), "seed 2147483647 in [0,1)" );
    check( non_degenerate( lm1a ), "seed 2147483647 non-degenerate" );
    auto const umax_a = feng::rand< double >( 16, 16, std::numeric_limits< unsigned int >::max() );
    auto const umax_b = feng::rand< double >( 16, 16, std::numeric_limits< unsigned int >::max() );
    check( umax_a == umax_b, "seed UINT_MAX twice equal" );
    check( in_range( umax_a ), "seed UINT_MAX in [0,1)" );
    check( non_degenerate( umax_a ), "seed UINT_MAX non-degenerate (no all-constant seed path)" );

    // float bound precision: 10000 draws in [0,1)
    auto const f = feng::rand< float >( 100, 100, 42 );
    check( in_range( f ), "float 10000 draws in [0,1)" );
    check( non_degenerate( f ), "float non-degenerate" );

    // shapes: 0x0 and 1x1
    auto const z = feng::rand< double >( 0, 0, 7 );
    check( z.row() == 0 && z.col() == 0, "0x0 shape preserved" );
    auto const u = feng::rand< double >( 1, 1, 7 );
    check( u.row() == 1 && u.col() == 1 && in_range( u ), "1x1 shape + range" );

    if ( failures != 0 )
    {
        std::printf( "FAILURES: %d\n", failures );
        return 1;
    }
    std::printf( "PASS AV-SEEDS\n" );
    return 0;
}
