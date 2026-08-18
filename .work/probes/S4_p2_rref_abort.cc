// S4 pre-flight probe p2 (C10, pre-fix evidence): rref on a SQUARE system must currently
// ABORT in a debug (assert-live) build — the precondition is `row < col`.
// Build (debug, asserts live): g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4_p2 .work/probes/S4_p2_rref_abort.cc
// Pre-fix expectation: assertion failure + abort (exit 134). Post-fix: prints the RREF.

#include "../../matrix.hpp"

#include <cstdio>

using feng::matrix;

int main()
{
    matrix<double> const m{ 2, 2, { 2.0, 0.0, 0.0, 3.0 } };
    auto const r = feng::rref( m );
    if ( !r.has_value() )
    {
        std::printf( "rref square: nullopt\n" );
        std::printf( "FAIL S4_p2: expected a value\n" );
        return 1;
    }
    std::printf( "rref square: [%g %g; %g %g]\n", ( *r )[ 0 ][ 0 ], ( *r )[ 0 ][ 1 ], ( *r )[ 1 ][ 0 ], ( *r )[ 1 ][ 1 ] );
    std::printf( "PASS S4_p2\n" );
    return 0;
}
