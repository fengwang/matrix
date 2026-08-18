#include <cmath>
#include <cstdlib>
TEST_CASE( "Matrix shrink_to_size", "[shrinksizes]" )
{
    // C1 regression (spec: docs/session_1/specs/shrink_to_size.md).
    // All assertions are on CONTENT (ragged values), not shape alone — the T1 anti-pattern guard.

    // Scenario: column-only shrink on a filled matrix (E01 acceptance; review ASan repro 5x5->5x3).
    {
        feng::matrix<double> m{ 5, 5, 1.0 };
        m.shrink_to_size( 5, 3 );
        REQUIRE( m.row() == 5 );
        REQUIRE( m.col() == 3 );
        for ( unsigned long r = 0; r != 5; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
                REQUIRE( std::abs( m[r][c] - 1.0 ) < 1.0e-12 );
    }

    // Scenario: mixed shrink-and-grow with ragged values (review silent-corruption repro 3x10->5x2).
    {
        feng::matrix<double> m{ 3, 10, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                         11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
                                         21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0 } };
        m.shrink_to_size( 5, 2 );
        double const expected[5][2] = { { 1.0, 2.0 }, { 11.0, 12.0 }, { 21.0, 22.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
        REQUIRE( m.row() == 5 );
        REQUIRE( m.col() == 2 );
        for ( unsigned long r = 0; r != 5; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                REQUIRE( std::abs( m[r][c] - expected[r][c] ) < 1.0e-12 );
    }

    // Scenario: grow-only zero-pad (contract acceptance E01: {1,1,7} -> 4x4).
    {
        feng::matrix<double> m{ 1, 1, 7.0 };
        m.shrink_to_size( 4, 4 );
        REQUIRE( m.row() == 4 );
        REQUIRE( m.col() == 4 );
        REQUIRE( std::abs( m[0][0] - 7.0 ) < 1.0e-12 );
        for ( unsigned long r = 0; r != 4; ++r )
            for ( unsigned long c = 0; c != 4; ++c )
                if ( ! ( ( r == 0 ) && ( c == 0 ) ) )
                    REQUIRE( std::abs( m[r][c] ) < 1.0e-12 );
    }

    // Scenario: non-square shrink, both directions (adversarial; ragged values catch row/col offset errors).
    {
        feng::matrix<double> wide{ 10, 3 };
        for ( unsigned long r = 0; r != 10; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
                wide[r][c] = static_cast<double>( r * 3 + c + 1 );
        wide.shrink_to_size( 5, 2 );
        REQUIRE( wide.row() == 5 );
        REQUIRE( wide.col() == 2 );
        for ( unsigned long r = 0; r != 5; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                REQUIRE( std::abs( wide[r][c] - static_cast<double>( r * 3 + c + 1 ) ) < 1.0e-12 );

        feng::matrix<double> tall{ 3, 10 };
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 10; ++c )
                tall[r][c] = static_cast<double>( r * 10 + c + 1 );
        tall.shrink_to_size( 5, 2 );
        REQUIRE( tall.row() == 5 );
        REQUIRE( tall.col() == 2 );
        for ( unsigned long r = 0; r != 5; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                REQUIRE( std::abs( tall[r][c] - ( r < 3 ? static_cast<double>( r * 10 + c + 1 ) : 0.0 ) ) < 1.0e-12 );
    }

    // Scenario: shrink-only extreme (10x10 ragged -> 1x1 keeps the original (0,0) value).
    {
        feng::matrix<double> m{ 10, 10 };
        for ( unsigned long r = 0; r != 10; ++r )
            for ( unsigned long c = 0; c != 10; ++c )
                m[r][c] = static_cast<double>( r * 10 + c + 1 );
        m.shrink_to_size( 1, 1 );
        REQUIRE( m.row() == 1 );
        REQUIRE( m.col() == 1 );
        REQUIRE( std::abs( m[0][0] - 1.0 ) < 1.0e-12 );
    }

    // Scenario: same-size no-op stays a no-op (early-return path).
    {
        feng::matrix<double> m{ 2, 3, { 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 } };
        m.shrink_to_size( 2, 3 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
                REQUIRE( std::abs( m[r][c] - static_cast<double>( r * 3 + c + 4 ) ) < 1.0e-12 );
    }
}
