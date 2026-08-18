#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

// IEEE finiteness check immune to the suite's -Ofast fast-math folding.
// Under fast-math the compiler assumes NaN/inf unreachable: std::isfinite(nan) folds to
// true, std::isnan(nan) to false, and bare comparisons against a NaN value can flip
// (observed: std::abs(nan - c) < e and nan < c compiled to true in the suite build).
// The bit pattern is read through std::memcpy (opaque to the optimizer), so the check
// cannot be folded. Exponent all-ones (0x7FF) iff ±inf or NaN.
static bool lu_is_finite( double v )
{
    std::uint64_t bits{ 0 };
    std::memcpy( &bits, &v, sizeof( bits ) );
    return ( ( bits >> 52 ) & 0x7FFull ) != 0x7FFull; // 11-bit exponent field; all-ones iff ±inf/NaN
}

// Legacy (PRE-pivoting, baseline 5c8fad9) LU solve — E09 invariance oracle.
// Verbatim copy of the pre-fix lu_decomposition + forward/backward substitution;
// it must NOT be updated when the library gains pivoting.
static int legacy_lu_decomposition( feng::matrix<double> const& A, feng::matrix<double>& L, feng::matrix<double>& U )
{
    const std::uint_least64_t n = A.row();
    L.resize( n, n );
    std::fill( L.begin(), L.end(), double{ 0 } );
    std::fill( L.diag_begin(), L.diag_end(), double( 1 ) );

    U.resize( n, n );
    std::fill( U.begin(), U.end(), double{ 0 } );

    for ( std::uint_least64_t j = 0; j < n; ++j )
    {
        for ( std::uint_least64_t i = 0; i < j + 1; ++i )
        {
            U[i][j] = A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + i, U.col_begin( j ), double() );
        }

        for ( std::uint_least64_t i = j + 1; i < n; ++i )
        {
            L[i][j] = ( A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + j, U.col_begin( j ), double() ) ) / U[j][j];

            if ( std::isinf( L[i][j] ) || std::isnan( L[i][j] ) )
                return 1;
        }
    }

    return 0;
}

static int legacy_forward( feng::matrix<double> const& A, feng::matrix<double>& x, feng::matrix<double> const& b )
{
    const std::uint_least64_t n = A.row();
    x.resize( n, 1 );
    std::fill( x.begin(), x.end(), double( 0 ) );

    for ( std::uint_least64_t i = 0; i != n; ++i )
    {
        double sum = std::inner_product( x.begin(), x.begin() + i, A.row_begin( i ), double( 0 ) );
        x[i][0]    = ( b[i][0] - sum ) / A[i][i];

        if ( std::isinf( x[i][0] ) || std::isnan( x[i][0] ) )
            return 1;
    }

    return 0;
}

static int legacy_backward( feng::matrix<double> const& A, feng::matrix<double>& x, feng::matrix<double> const& b )
{
    const std::uint_least64_t n = A.row();
    x.resize( n, 1 );
    std::fill( x.begin(), x.end(), double( 0 ) );

    for ( std::uint_least64_t i = 0; i != n; ++i )
    {
        std::uint_least64_t const r = n - 1 - i;
        double sum                 = std::inner_product( x.rbegin(), x.rbegin() + i, A.row_rbegin( r ), double( 0 ) );
        x[r][0]                    = ( b[r][0] - sum ) / A[r][r];

        if ( std::isinf( x[r][0] ) || std::isnan( x[r][0] ) )
            return 1;
    }

    return 0;
}

static feng::matrix<double> legacy_lu_solver( feng::matrix<double> const& A, feng::matrix<double> const& b )
{
    feng::matrix<double> L, U;
    int rc{ 0 };
    if ( legacy_lu_decomposition( A, L, U ) )
        rc = 1;

    feng::matrix<double> Y;
    if ( legacy_forward( L, Y, b ) )
        rc = 1;

    feng::matrix<double> x;
    if ( legacy_backward( U, x, Y ) )
        rc = 1;

    if ( rc )
        return feng::matrix<double>{ A.row(), 1, { -1.0e30, -1.0e30, -1.0e30, -1.0e30, -1.0e30, -1.0e30 } };
    return x;
}

TEST_CASE( "Matrix lu_decomposition partial pivoting + lu_solver", "[lu_pivoting]" )
{
    // P2 regression (spec: docs/session_3/specs/lu_pivoting.md).
    // lu_decomposition performs partial pivoting (P*A = L*U); lu_solver applies P to b,
    // so solutions are invariant vs the pre-pivoting algorithm for nonsingular systems
    // (legacy oracle above).

    // Scenario: solution invariance vs legacy oracle (E09 acceptance).
    // Fixed well-conditioned 6x6 SPD system; exact solution (1,2,3,4,5,6).
    {
        feng::matrix<double> const A{ 6, 6,
                                      { 2.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                                        1.0, 3.0, 1.0, 0.0, 0.0, 0.0,
                                        0.0, 1.0, 2.0, 1.0, 0.0, 0.0,
                                        0.0, 0.0, 1.0, 3.0, 1.0, 0.0,
                                        0.0, 0.0, 0.0, 1.0, 2.0, 1.0,
                                        0.0, 0.0, 0.0, 0.0, 1.0, 3.0 } };
        feng::matrix<double> const b{ 6, 1, { 4.0, 10.0, 12.0, 20.0, 20.0, 23.0 } };
        feng::matrix<double> const x_exact{ 6, 1, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };

        feng::matrix<double> const x_legacy = legacy_lu_solver( A, b );
        auto const sol = feng::lu_solver( A, b );
        REQUIRE( sol.has_value() );
        feng::matrix<double> const x_lib = sol.value();

        double d_oracle{ 0.0 }, d_exact{ 0.0 };
        for ( unsigned long i = 0; i != 6; ++i )
        {
            d_oracle = std::max( d_oracle, std::abs( x_lib[i][0] - x_legacy[i][0] ) );
            d_exact  = std::max( d_exact, std::abs( x_lib[i][0] - x_exact[i][0] ) );
        }
        REQUIRE( d_oracle < 1.0e-9 );
        REQUIRE( d_exact < 1.0e-9 );
    }

    // Scenario: zero-first-pivot rescue — pre-pivoting code fails (L[i][j] = x/0 -> inf/nan).
    // NOTE (suite-flags finding): the suite builds with -Ofast (fast-math), which folds the
    // library's isinf/isnan guards (the pre-fix solver returns has_value with x = NaN instead
    // of nullopt) and makes NaN comparisons unreliable — no comparison form proved
    // trustworthy in-context (std::abs(nan - c) < e and even two-sided ranges compiled to
    // true for NaN in the suite build, while evaluating false in a standalone TU with the
    // same flags). The x-content check is therefore a bit-level finiteness check
    // (lu_is_finite, opaque to the optimizer) plus a two-sided range as defense in depth.
    // Pre-fix the finiteness check is the genuine red; the singular-system nullopt pin
    // (guard behavior) lives in the -O1 probe, where IEEE guards are honored.
    {
        feng::matrix<double> const A{ 3, 3, { 0.0, 1.0, 2.0, 1.0, 0.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const b{ 3, 1, { 3.0, 4.0, 15.0 } };
        auto const sol = feng::lu_solver( A, b );
        REQUIRE( sol.has_value() );
        feng::matrix<double> const x = sol.value();
        // Finiteness first (fast-math-immune, see lu_is_finite), then the two-sided range.
        for ( unsigned long i = 0; i != 3; ++i )
            REQUIRE( lu_is_finite( x[i][0] ) );
        for ( unsigned long i = 0; i != 3; ++i )
        {
            bool const in_range = ( x[i][0] > 1.0 - 1.0e-9 ) && ( x[i][0] < 1.0 + 1.0e-9 );
            REQUIRE( in_range );
        }
    }

    // ---- 5-arg partial-pivoting API (stage B; green on first run — no pre-fix red state
    //      exists for a new API) ----

    // Scenario: hand-verified 3x3 with two row swaps.
    // A = [0 1 2; 1 0 3; 4 5 6]: col-0 pivot swaps rows 0<->2 (p=2), col-1 pivot swaps
    // rows 1<->2 (p=2). Expected: perm = [2,0,1] (PA = LU), sign = +1 (two swaps),
    // L = [1 0 0; 0 1 0; 1/4 -5/4 1], U = [4 5 6; 0 1 2; 0 0 4], det = sign*prod(U_ii) = 16.
    {
        feng::matrix<double> const A{ 3, 3, { 0.0, 1.0, 2.0, 1.0, 0.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> L, U;
        int sign{ 0 };
        std::vector< std::uint_least64_t > perm;
        int const rc = feng::lu_decomposition( A, L, U, sign, perm );
        REQUIRE( rc == 0 );
        REQUIRE( sign == 1 );
        REQUIRE( perm.size() == 3 );
        REQUIRE( perm[0] == 2 );
        REQUIRE( perm[1] == 0 );
        REQUIRE( perm[2] == 1 );

        // L: unit lower triangular with the hand-derived multipliers.
        REQUIRE( L[0][0] == 1.0 );
        REQUIRE( L[1][1] == 1.0 );
        REQUIRE( L[2][2] == 1.0 );
        bool const l01 = L[0][1] == 0.0 && L[0][2] == 0.0;
        REQUIRE( l01 );
        REQUIRE( L[1][0] == 0.0 );
        bool const l20 = ( L[2][0] > 0.25 - 1.0e-12 && L[2][0] < 0.25 + 1.0e-12 );
        REQUIRE( l20 );
        bool const l21 = ( L[2][1] > -1.25 - 1.0e-12 && L[2][1] < -1.25 + 1.0e-12 );
        REQUIRE( l21 );

        // U: upper triangular with the hand-derived entries.
        REQUIRE( U[0][0] == 4.0 );
        REQUIRE( U[1][1] == 1.0 );
        REQUIRE( U[2][2] == 4.0 );
        REQUIRE( U[1][0] == 0.0 );
        REQUIRE( U[2][0] == 0.0 );
        REQUIRE( U[2][1] == 0.0 );
        bool const u01 = ( U[0][1] > 5.0 - 1.0e-12 && U[0][1] < 5.0 + 1.0e-12 );
        REQUIRE( u01 );
        bool const u02 = ( U[0][2] > 6.0 - 1.0e-12 && U[0][2] < 6.0 + 1.0e-12 );
        REQUIRE( u02 );
        bool const u12 = ( U[1][2] > 2.0 - 1.0e-12 && U[1][2] < 2.0 + 1.0e-12 );
        REQUIRE( u12 );

        // PA == LU: row i of L*U equals original row perm[i] of A.
        feng::matrix<double> const PA = L * U;
        for ( unsigned long i = 0; i != 3; ++i )
            for ( unsigned long j = 0; j != 3; ++j )
            {
                double const d = PA[i][j] - A[ perm[i] ][ j ];
                bool const close = ( d > -1.0e-12 && d < 1.0e-12 );
                REQUIRE( close );
            }

        // det consistency: sign * prod(U_ii) = 16 (hand-derived).
        double const det = ( double ) sign * ( double ) U[0][0] * ( double ) U[1][1] * ( double ) U[2][2];
        bool const det_ok = ( det > 16.0 - 1.0e-12 && det < 16.0 + 1.0e-12 );
        REQUIRE( det_ok );
    }

    // Scenario: 2x2 single swap — pivot col 0 swaps rows (|3| > |2|), sign = -1.
    {
        feng::matrix<double> const A{ 2, 2, { 2.0, 1.0, 3.0, 2.0 } };
        feng::matrix<double> L, U;
        int sign{ 0 };
        std::vector< std::uint_least64_t > perm;
        int const rc = feng::lu_decomposition( A, L, U, sign, perm );
        REQUIRE( rc == 0 );
        REQUIRE( sign == -1 );
        REQUIRE( perm.size() == 2 );
        REQUIRE( perm[0] == 1 );
        REQUIRE( perm[1] == 0 );
        REQUIRE( U[0][0] == 3.0 );
        bool const l10 = ( L[1][0] > 2.0 / 3.0 - 1.0e-12 && L[1][0] < 2.0 / 3.0 + 1.0e-12 );
        REQUIRE( l10 );
        bool const u11 = ( U[1][1] > -1.0 / 3.0 - 1.0e-12 && U[1][1] < -1.0 / 3.0 + 1.0e-12 );
        REQUIRE( u11 );
    }

    // Scenario: no-pivot matrix keeps identity permutation and sign = +1 (3x3 diagonal).
    {
        feng::matrix<double> const A{ 3, 3, { 9.0, 0.0, 0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 7.0 } };
        feng::matrix<double> L, U;
        int sign{ 0 };
        std::vector< std::uint_least64_t > perm;
        int const rc = feng::lu_decomposition( A, L, U, sign, perm );
        REQUIRE( rc == 0 );
        REQUIRE( sign == 1 );
        REQUIRE( perm.size() == 3 );
        REQUIRE( perm[0] == 0 );
        REQUIRE( perm[1] == 1 );
        REQUIRE( perm[2] == 2 );
        REQUIRE( U[0][0] == 9.0 );
        REQUIRE( U[1][1] == 8.0 );
        REQUIRE( U[2][2] == 7.0 );
    }

    // Scenario: 3-arg overload still works (source-compatible delegation) and agrees
    // with the 5-arg decomposition on the hand-verified 3x3.
    {
        feng::matrix<double> const A{ 3, 3, { 0.0, 1.0, 2.0, 1.0, 0.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> L3, U3, L5, U5;
        int sign5{ 0 };
        std::vector< std::uint_least64_t > perm5;
        int const rc3 = feng::lu_decomposition( A, L3, U3 );
        int const rc5 = feng::lu_decomposition( A, L5, U5, sign5, perm5 );
        REQUIRE( rc3 == 0 );
        REQUIRE( rc3 == rc5 );
        for ( unsigned long i = 0; i != 3; ++i )
            for ( unsigned long j = 0; j != 3; ++j )
            {
                bool const l_eq = L3[i][j] == L5[i][j];
                REQUIRE( l_eq );
                bool const u_eq = U3[i][j] == U5[i][j];
                REQUIRE( u_eq );
            }
    }
}
