#include <cmath>
TEST_CASE( "Matrix fliplr/flipud", "[flip]" )
{
    // C3 regression (spec: docs/session_3/specs/flip_aliases.md).
    // MATLAB/NumPy convention: fliplr = flipdim(m, 2) (left-right), flipud = flipdim(m, 1)
    // (up-down). Pre-fix the aliases were swapped.

    // Scenario: E05 2x3 content.
    {
        feng::matrix<double> const m{ 2, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const lr = feng::fliplr( m );
        feng::matrix<double> const ud = feng::flipud( m );
        double const want_lr[2][3] = { { 3.0, 2.0, 1.0 }, { 6.0, 5.0, 4.0 } };
        double const want_ud[2][3] = { { 4.0, 5.0, 6.0 }, { 1.0, 2.0, 3.0 } };
        REQUIRE( lr.row() == 2 );
        REQUIRE( lr.col() == 3 );
        REQUIRE( ud.row() == 2 );
        REQUIRE( ud.col() == 3 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
            {
                REQUIRE( std::abs( lr[r][c] - want_lr[r][c] ) < 1.0e-12 );
                REQUIRE( std::abs( ud[r][c] - want_ud[r][c] ) < 1.0e-12 );
            }
    }

    // Scenario: 3x3 ragged content (both aliases, 9 distinct values).
    {
        feng::matrix<double> const m{ 3, 3, { 2.0, -1.0, 7.5, 0.0, 4.0, 3.25, -6.0, 1.5, 8.0 } };
        feng::matrix<double> const lr = feng::fliplr( m );
        feng::matrix<double> const ud = feng::flipud( m );
        double const want_lr[3][3] = { { 7.5, -1.0, 2.0 }, { 3.25, 4.0, 0.0 }, { 8.0, 1.5, -6.0 } };
        double const want_ud[3][3] = { { -6.0, 1.5, 8.0 }, { 0.0, 4.0, 3.25 }, { 2.0, -1.0, 7.5 } };
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
            {
                REQUIRE( std::abs( lr[r][c] - want_lr[r][c] ) < 1.0e-12 );
                REQUIRE( std::abs( ud[r][c] - want_ud[r][c] ) < 1.0e-12 );
            }
    }

    // Scenario: 1x3 fliplr == elementwise row reversal.
    {
        feng::matrix<double> const m{ 1, 3, { 1.0, 2.0, 3.0 } };
        feng::matrix<double> const lr = feng::fliplr( m );
        REQUIRE( lr.row() == 1 );
        REQUIRE( lr.col() == 3 );
        REQUIRE( std::abs( lr[0][0] - 3.0 ) < 1.0e-12 );
        REQUIRE( std::abs( lr[0][1] - 2.0 ) < 1.0e-12 );
        REQUIRE( std::abs( lr[0][2] - 1.0 ) < 1.0e-12 );
    }

    // Scenario: 3x1 flipud == row reversal.
    {
        feng::matrix<double> const m{ 3, 1, { 1.0, 2.0, 3.0 } };
        feng::matrix<double> const ud = feng::flipud( m );
        REQUIRE( ud.row() == 3 );
        REQUIRE( ud.col() == 1 );
        REQUIRE( std::abs( ud[0][0] - 3.0 ) < 1.0e-12 );
        REQUIRE( std::abs( ud[1][0] - 2.0 ) < 1.0e-12 );
        REQUIRE( std::abs( ud[2][0] - 1.0 ) < 1.0e-12 );
    }

    // Scenario: 1x1 fixed point (both aliases).
    {
        feng::matrix<double> const m{ 1, 1, { -4.0 } };
        feng::matrix<double> const lr = feng::fliplr( m );
        feng::matrix<double> const ud = feng::flipud( m );
        REQUIRE( std::abs( lr[0][0] + 4.0 ) < 1.0e-12 );
        REQUIRE( std::abs( ud[0][0] + 4.0 ) < 1.0e-12 );
    }
}
