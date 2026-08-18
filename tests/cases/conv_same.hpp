#include <cmath>
// S4 C9: conv "same" mode must accept kernels with row >= 1 AND col >= 1
// (independently). Pre-fix the second assert was a copy-paste of the first
// (`rb > 1` tested twice), so a 1x1 kernel aborted in debug (assert-live)
// builds — SIGABRT at matrix.hpp:6755 (probe p1).

TEST_CASE( "Matrix conv same-mode kernel preconditions (C9)", "[conv]" )
{
    // Scenario (a) — E11: 1x1 kernel is scaling (the well-defined case the
    // pre-fix copy-pasted assert rejected).
    {
        feng::matrix<double> const A{ 2, 2, { 1.0, 2.0, 3.0, 4.0 } };
        feng::matrix<double> const K{ 1, 1, { 0.5 } };
        feng::matrix<double> const C = feng::conv( A, K, std::string{ "same" } );
        double const want[2][2] = { { 0.5, 1.0 }, { 1.5, 2.0 } };
        REQUIRE( C.row() == 2 );
        REQUIRE( C.col() == 2 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                REQUIRE( std::abs( C[r][c] - want[r][c] ) < 1.0e-12 );
    }

    // Scenario (b) — rb==1, cb==2 (the direction the copy-paste masked).
    // Sum filter over a 2x3: same-mode slice of the full conv, hand-derived
    // (full = {{1,3,5,3},{4,9,11,6}}; slice rows {0,2}, cols {0,3}).
    {
        feng::matrix<double> const A{ 2, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const K{ 1, 2, { 1.0, 1.0 } };
        feng::matrix<double> const C = feng::conv( A, K, std::string{ "same" } );
        double const want[2][3] = { { 1.0, 3.0, 5.0 }, { 4.0, 9.0, 11.0 } };
        REQUIRE( C.row() == 2 );
        REQUIRE( C.col() == 3 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
                REQUIRE( std::abs( C[r][c] - want[r][c] ) < 1.0e-12 );
    }

    // Scenario (c) — rb==2, cb==1 (the mirror direction).
    // Hand-derived (full = {{1,2},{4,6},{8,10},{5,6}; slice rows {0,3}, cols {0,2}).
    {
        feng::matrix<double> const A{ 3, 2, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const K{ 2, 1, { 1.0, 1.0 } };
        feng::matrix<double> const C = feng::conv( A, K, std::string{ "same" } );
        double const want[3][2] = { { 1.0, 2.0 }, { 4.0, 6.0 }, { 8.0, 10.0 } };
        REQUIRE( C.row() == 3 );
        REQUIRE( C.col() == 2 );
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 2; ++c )
                REQUIRE( std::abs( C[r][c] - want[r][c] ) < 1.0e-12 );
    }

    // Scenario (d) — "valid" mode regression (the valid branch is untouched by
    // this fix; a 2x3 kernel with a single 0.5 at [0][0] makes the valid slice
    // exactly 0.5 * A[0:3, 0:3], pinning the slice arithmetic).
    {
        feng::matrix<double> const A{ 4, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0 } };
        feng::matrix<double> const K{ 2, 3, { 0.5, 0.0, 0.0, 0.0, 0.0, 0.0 } };
        feng::matrix<double> const C = feng::conv( A, K, std::string{ "valid" } );
        double const want[3][3] = { { 0.5, 1.0, 1.5 }, { 3.0, 3.5, 4.0 }, { 5.5, 6.0, 6.5 } };
        REQUIRE( C.row() == 3 );
        REQUIRE( C.col() == 3 );
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
                REQUIRE( std::abs( C[r][c] - want[r][c] ) < 1.0e-12 );
    }

    // Scenario (e) — L2 (S4 sharded review): same-mode with a kernel >= 2x2, the
    // mainstream pre-fix usage — content must be unchanged by the assert relaxation.
    // Full 4x4 conv (measured; the library correlates with the kernel's
    // bottom-right element anchored — for 1D kernels this coincides with the
    // centered convention pinned in (b)/(c)):
    //   {{1, 3, 5, 3}, {5, 12, 16, 9}, {11, 24, 28, 15}, {7, 15, 17, 9}};
    // same-mode slice at offset (2-1)>>1 = 0, size 3 -> the top-left 3x3.
    {
        feng::matrix<double> const A{ 3, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 } };
        feng::matrix<double> const K{ 2, 2, { 1.0, 1.0, 1.0, 1.0 } };
        feng::matrix<double> const C = feng::conv( A, K, std::string{ "same" } );
        double const want[3][3] = { { 1.0, 3.0, 5.0 }, { 5.0, 12.0, 16.0 }, { 11.0, 24.0, 28.0 } };
        REQUIRE( C.row() == 3 );
        REQUIRE( C.col() == 3 );
        for ( unsigned long r = 0; r != 3; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
                REQUIRE( std::abs( C[r][c] - want[r][c] ) < 1.0e-12 );
    }
}
