#include <cmath>
// S4 C10: rref / gauss_jordan_elimination must accept square systems (and any
// non-empty matrix). Pre-fix the precondition was `row < col`, so the most
// common case — a square system — aborted in debug (assert-live) builds:
// SIGABRT at matrix.hpp:6486 (probe p2).
//
// NOTE: the `row > col` (over-determined) case is deliberately NOT tested
// here: the algorithm reads OOB for row > col (strided col_begin(i) for
// i >= col) — pre-existing release-reachable UB (ASan probe p3), documented
// and pinned by the before/after ASan pair, out of this session's scope.

TEST_CASE( "Matrix rref square system precondition (C10)", "[rref]" )
{
    // Scenario (a) — E12: square system accepted, RREF of a diagonal matrix is the identity.
    {
        feng::matrix<double> const m{ 2, 2, { 2.0, 0.0, 0.0, 3.0 } };
        auto const r = feng::rref( m );
        REQUIRE( r.has_value() );
        double const want[2][2] = { { 1.0, 0.0 }, { 0.0, 1.0 } };
        REQUIRE( ( *r ).row() == 2 );
        REQUIRE( ( *r ).col() == 2 );
        for ( unsigned long i = 0; i != 2; ++i )
            for ( unsigned long j = 0; j != 2; ++j )
                REQUIRE( std::abs( ( *r )[ i ][ j ] - want[i][j] ) < 1.0e-10 );
    }

    // Scenario (b) — singular square system: the existing 1e-10 pivot exit
    // returns nullopt (finite comparison, fast-math safe); no hang, no abort.
    {
        feng::matrix<double> const m{ 2, 2, { 1.0, 2.0, 2.0, 4.0 } };
        auto const r = feng::rref( m );
        REQUIRE( !r.has_value() );
    }

    // Scenario (c) — wide regression (row < col, the originally-supported case):
    // an already-reduced wide matrix is its own RREF.
    {
        feng::matrix<double> const m{ 2, 3, { 1.0, 0.0, 2.0, 0.0, 1.0, 3.0 } };
        auto const r = feng::rref( m );
        REQUIRE( r.has_value() );
        REQUIRE( ( *r ).row() == 2 );
        REQUIRE( ( *r ).col() == 3 );
        for ( unsigned long i = 0; i != 2; ++i )
            for ( unsigned long j = 0; j != 3; ++j )
                REQUIRE( std::abs( ( *r )[ i ][ j ] - m[i][j] ) < 1.0e-10 );
    }
}
