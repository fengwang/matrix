// S5 pre-flight probe (pre-fix evidence, P5 probe-first):
//   (a) SAME call site, seed 0, same second: repeated calls return the SAME first element
//       (per-call-site correlation — the documented "two calls identical" claim refined by
//       ltrace evidence: seed = time + &ans; &ans is stable per call site, differs across
//       call sites by the stack distance, e.g. 32 bytes for two adjacent locals).
//   (b) explicit-seed determinism: rand(4,4,7) twice -> equal (invariant; must survive the engine swap)
//   (c) value-stream record: rand(2,5,7) and rand(1,4,1) printed (handoff before/after note)
// Build: g++ -std=c++20 -DPARALLEL -O1 -o .work/evidence/seed_S5_p0 .work/probes/S5_p0_preflight.cc
#include "../../matrix.hpp"

#include <cstdio>
#include <ctime>

namespace
{
    template < typename T >
    bool equal( feng::matrix<T> const& a, feng::matrix<T> const& b )
    {
        for ( unsigned long r = 0; r < a.row(); ++r )
            for ( unsigned long c = 0; c < a.col(); ++c )
                if ( a[r][c] != b[r][c] )
                    return false;
        return true;
    }
}

int main()
{
    std::printf( "time before seed-0 loop: %lld\n", static_cast< long long >( std::time( nullptr ) ) );
    // (a) same call site: first element over 100 repetitions of rand(4,4,0)
    int const distinct_first = [ ]()
    {
        int distinct = 1;
        double first = 0.0, prev = 0.0;
        for ( int i = 0; i != 100; ++i )
        {
            feng::matrix< double > const m = feng::rand< double >( 4, 4, 0 );
            first = ( i == 0 ) ? m[0][0] : first;
            if ( i != 0 && m[0][0] != prev )
                ++distinct;
            prev = m[0][0];
        }
        ( void ) first;
        return distinct;
    }();
    std::printf( "time after  seed-0 loop: %lld\n", static_cast< long long >( std::time( nullptr ) ) );
    std::printf( "same-site seed-0 first-element distinct values over 100 calls: %d (correlation confirmed if <= 2)\n", distinct_first );

    // (a2) cross call-site: the documented form — two locals, same second
    feng::matrix< double > const x = feng::rand< double >( 4, 4, 0 );
    feng::matrix< double > const y = feng::rand< double >( 4, 4, 0 );
    std::printf( "cross-site (different &ans salts) equal: %d\n", int( equal( x, y ) ) );

    auto const a = feng::rand< double >( 4, 4, 7 );
    auto const b = feng::rand< double >( 4, 4, 7 );
    auto const c = feng::rand< double >( 4, 4, 8 );
    std::printf( "explicit seed 7 == 7: %d\n", int( equal( a, b ) ) );
    std::printf( "seed 7 != 8: %d\n", int( !equal( a, c ) ) );

    std::printf( "rand(2,5,7):\n" );
    auto const v = feng::rand< double >( 2, 5, 7 );
    for ( unsigned long r = 0; r < 2; ++r )
        for ( unsigned long cc = 0; cc < 5; ++cc )
            std::printf( "%.17g ", v[r][cc] );
    std::printf( "\nrand(1,4,1) [examples use seed 1]:\n" );
    auto const one = feng::rand< double >( 1, 4, 1 );
    for ( unsigned long cc = 0; cc < 4; ++cc )
        std::printf( "%.17g ", one[0][cc] );
    std::printf( "\n" );
    return 0;
}
