// S5 post-fix combined probe: E14 (rand engine invariants) + E15 (save_png boundary).
// Build (verbatim, contract form):
//   g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5 .work/probes/E14_E15.cc
// Prints PASS on success.
#include "../../matrix.hpp"

#include <cstdio>
#include <filesystem>

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
static bool in_open_unit_interval( feng::matrix<T> const& a )
{
    for ( unsigned long r = 0; r < a.row(); ++r )
        for ( unsigned long c = 0; c < a.col(); ++c )
            if ( a[r][c] < T( 0 ) || a[r][c] >= T( 1 ) )
                return false;
    return true;
}

int main()
{
    // ---- E14: explicit-seed determinism + [0,1) range (invariant pin) ----
    auto const a = feng::rand< double >( 64, 64, 7 );
    auto const b = feng::rand< double >( 64, 64, 7 );
    auto const c = feng::rand< double >( 64, 64, 8 );
    check( a.row() == 64 && a.col() == 64, "E14 shape" );
    check( equal( a, b ), "E14 same seed (7) -> identical" );
    check( !equal( a, c ), "E14 different seeds (7 vs 8) -> different" );
    check( in_open_unit_interval( a ), "E14 double range [0,1)" );

    auto const fa = feng::rand< float >( 32, 32, 7 );
    auto const fb = feng::rand< float >( 32, 32, 7 );
    check( equal( fa, fb ), "E14 float same seed -> identical" );
    check( in_open_unit_interval( fa ), "E14 float range [0,1)" );

    // ---- E15: save_png boundary (silent no-op on unwritable path; happy path intact) ----
    feng::matrix< double > const m{ 4, 4, 1.0 };
    bool const ok = m.save_as_png( "/nonexistent_dir_s5/x.png" ); // must not crash
    check( ok, "E15 unwritable path: no crash, returns as before" );

    bool const ok2 = m.save_as_png( ".work/evidence/s5_E15_positive_control.png" );
    bool const exists = std::filesystem::exists( ".work/evidence/s5_E15_positive_control.png" );
    check( ok2 && exists, "E15 positive control: writable path produces PNG" );

    if ( failures != 0 )
    {
        std::printf( "FAILURES: %d\n", failures );
        return 1;
    }
    std::printf( "PASS\n" );
    return 0;
}
