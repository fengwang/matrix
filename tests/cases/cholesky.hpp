#include <cmath>
// S4 P2 (cholesky): cholesky_decomposition returns bool (was void) and
// returns false when a diagonal step is not positive-definite (residual
// <= 0), before the sqrt — so `a` is left in a defined state and no NaN
// is written. Pre-fix: non-PD input silently produced a[1][1] = -nan with
// no failure channel (probe p0).

TEST_CASE( "Matrix cholesky_decomposition positive-definite guard (P2)", "[cholesky]" )
{
    // Scenario (a) — E13 non-PD: eigenvalues -1, 3. Second diagonal step is
    // 1 - 2^2 = -3 <= 0 -> false, before the sqrt. `a` keeps the defined
    // state: a[0][0]=1 (sqrt(1)), a[1][0]=2, a[1][1] still the input value 1
    // (the bad sqrt is never assigned).
    {
        feng::matrix<double> const m{ 2, 2, { 1.0, 2.0, 2.0, 1.0 } };
        feng::matrix<double> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( !ok );
        REQUIRE( a.row() == 2 );
        REQUIRE( a.col() == 2 );
        REQUIRE( std::abs( a[0][0] - 1.0 ) < 1.0e-12 );
        REQUIRE( std::abs( a[1][0] - 2.0 ) < 1.0e-12 );
        REQUIRE( std::abs( a[1][1] - 1.0 ) < 1.0e-12 ); // input value preserved; no NaN written
    }

    // Scenario (b) — PD: true, factor content, and a * a^T ~ m.
    {
        feng::matrix<double> const m{ 2, 2, { 4.0, 2.0, 2.0, 3.0 } };
        feng::matrix<double> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( ok );
        REQUIRE( std::abs( a[0][0] - 2.0 ) < 1.0e-12 );
        REQUIRE( std::abs( a[0][1] - 0.0 ) < 1.0e-12 ); // upper triangle zero-filled
        REQUIRE( std::abs( a[1][0] - 1.0 ) < 1.0e-12 );
        REQUIRE( std::abs( a[1][1] - std::sqrt( 2.0 ) ) < 1.0e-12 );
        // a * a^T: (i,j) = sum_k a[i][k] * a[j][k] = [[2^2+0^2, 2*1+0*sqrt(2)], [1*2+sqrt(2)*0, 1^2+2]] = m
        double const p00 = a[0][0] * a[0][0] + a[0][1] * a[0][1];
        double const p01 = a[0][0] * a[1][0] + a[0][1] * a[1][1];
        double const p11 = a[1][0] * a[1][0] + a[1][1] * a[1][1];
        REQUIRE( std::abs( p00 - 4.0 ) < 1.0e-10 );
        REQUIRE( std::abs( p01 - 2.0 ) < 1.0e-10 );
        REQUIRE( std::abs( p11 - 3.0 ) < 1.0e-10 );
    }

    // Scenario (c) — PSD-singular boundary (eigenvalues 0, 2): second
    // diagonal step is exactly 0 -> false (strict positivity, C-11).
    {
        feng::matrix<double> const m{ 2, 2, { 1.0, 1.0, 1.0, 1.0 } };
        feng::matrix<double> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( !ok );
    }

    // Scenario (d) — 1x1 zero: diagonal step 0 -> false.
    {
        feng::matrix<double> const m{ 1, 1, { 0.0 } };
        feng::matrix<double> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( !ok );
    }

    // Scenario (e) — 1x1 positive: true, factor is the exact sqrt.
    {
        feng::matrix<double> const m{ 1, 1, { 4.0 } };
        feng::matrix<double> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( ok );
        REQUIRE( std::abs( a[0][0] - 2.0 ) < 1.0e-12 );
    }

    // Scenario (f) — L3 (S4 sharded review): 3x3 SPD pinning the multi-step
    // inner_product accumulation. m = L * L^T with the exact factor
    // L = [[2,0,0],[1,2,0],[0.5,0.5,1]] (all values binary-exact).
    {
        feng::matrix<double> const m{ 3, 3, { 4.0, 2.0, 1.0, 2.0, 5.0, 1.5, 1.0, 1.5, 1.5 } };
        feng::matrix<double> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( ok );
        double const want[3][3] = { { 2.0, 0.0, 0.0 }, { 1.0, 2.0, 0.0 }, { 0.5, 0.5, 1.0 } };
        for ( unsigned long i = 0; i != 3; ++i )
            for ( unsigned long j = 0; j != 3; ++j )
                REQUIRE( std::abs( a[i][j] - want[i][j] ) < 1.0e-12 );
    }

    // Scenario (g) — L3 (S4 sharded review): float value_type (the guard's
    // value_type(0) under the non-double type).
    {
        feng::matrix<float> const m{ 1, 1, { 4.0f } };
        feng::matrix<float> a;
        bool const ok = feng::cholesky_decomposition( m, a );
        REQUIRE( ok );
        REQUIRE( std::abs( a[0][0] - 2.0f ) < 1.0e-6f );
    }
}
