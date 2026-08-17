#include <cmath>
#include <cstdlib>
TEST_CASE( "Matrix flipdim", "[flip]" )
{
    // C2 regression (spec: docs/session_1/specs/flipdim.md).
    // Content assertions on ragged values; dim==1 case pins the branch this session must not touch.
    // fliplr/flipud are S3 territory (C3) and are intentionally NOT tested here.

    // Scenario: non-square 3x5 left-right flip (E02 acceptance; review ASan repro).
    {
        feng::matrix<double> m{ 3, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 2 );
        double const expected[3][5] = { { 5.0, 4.0, 3.0, 2.0, 1.0 },
                                        { 10.0, 9.0, 8.0, 7.0, 6.0 },
                                        { 15.0, 14.0, 13.0, 12.0, 11.0 } };
        REQUIRE( f.row() == 3 );
        REQUIRE( f.col() == 5 );
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 5; ++c )
                REQUIRE( std::abs( f[r][c] - expected[r][c] ) < 1.0e-12 );
    }

    // Scenario: square 4x4 left-right flip (review silent-corruption repro).
    {
        feng::matrix<double> m{ 4, 4, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 2 );
        REQUIRE( f.row() == 4 );
        REQUIRE( f.col() == 4 );
        for ( unsigned long r = 0; r != 4; ++r )
            for ( unsigned long c = 0; c != 4; ++c )
                REQUIRE( std::abs( f[r][c] - m[r][3 - c] ) < 1.0e-12 );
    }

    // Scenario: ragged non-square, both orientations (2x7 and 7x2).
    {
        feng::matrix<double> wide{ 2, 7 };
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 7; ++c )
                wide[r][c] = static_cast<double>( r * 7 + c + 1 );
        feng::matrix<double> const fw = feng::flipdim( wide, 2 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 7; ++c )
                REQUIRE( std::abs( fw[r][c] - wide[r][6 - c] ) < 1.0e-12 );

        feng::matrix<double> tall{ 7, 2 };
        for ( unsigned long r = 0; r != 7; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                tall[r][c] = static_cast<double>( r * 2 + c + 1 );
        feng::matrix<double> const ft = feng::flipdim( tall, 2 );
        for ( unsigned long r = 0; r != 7; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                REQUIRE( std::abs( ft[r][c] - tall[r][1 - c] ) < 1.0e-12 );
    }

    // Scenario: degenerate single-row / single-column shapes (1x5 and 5x1).
    {
        feng::matrix<double> row{ 1, 5, { 1.0, 2.0, 3.0, 4.0, 5.0 } };
        feng::matrix<double> const fr = feng::flipdim( row, 2 );
        for ( unsigned long c = 0; c != 5; ++c )
            REQUIRE( std::abs( fr[0][c] - static_cast<double>( 5 - c ) ) < 1.0e-12 );

        feng::matrix<double> col{ 5, 1, { 1.0, 2.0, 3.0, 4.0, 5.0 } };
        feng::matrix<double> const fc = feng::flipdim( col, 2 );
        REQUIRE( fc.row() == 5 );
        REQUIRE( fc.col() == 1 );
        for ( unsigned long r = 0; r != 5; ++r )
            REQUIRE( std::abs( fc[r][0] - col[r][0] ) < 1.0e-12 ); // a single column is unchanged
    }

    // Scenario: 3x5 up-down flip (dim==1 pin — the branch this session must not modify).
    {
        feng::matrix<double> m{ 3, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 1 );
        double const expected[3][5] = { { 11.0, 12.0, 13.0, 14.0, 15.0 },
                                        { 6.0, 7.0, 8.0, 9.0, 10.0 },
                                        { 1.0, 2.0, 3.0, 4.0, 5.0 } };
        REQUIRE( f.row() == 3 );
        REQUIRE( f.col() == 5 );
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 5; ++c )
                REQUIRE( std::abs( f[r][c] - expected[r][c] ) < 1.0e-12 );
    }

    // Scenario: input is not mutated by flipdim (purity pin).
    {
        feng::matrix<double> m{ 2, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 2 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
            {
                REQUIRE( std::abs( m[r][c] - static_cast<double>( r * 3 + c + 1 ) ) < 1.0e-12 );
                REQUIRE( std::abs( f[r][c] - m[r][2 - c] ) < 1.0e-12 );
            }
    }
}
