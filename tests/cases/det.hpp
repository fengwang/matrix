#include <cmath>
#include <cstdint>
#include <cstring>

// Exact-zero check immune to the suite's -Ofast fast-math folding.
// Under fast-math, ds == 0.0 with ds = NaN evaluated to TRUE in the suite build
// (observed: -nan == 0.0 -> 1, bits fff8000000000000). A bit-pattern equality check
// read through std::memcpy cannot be folded: only +0.0 (bits == 0) qualifies.
static bool det_is_zero( double v )
{
    std::uint64_t bits{ 0 };
    std::memcpy( &bits, &v, sizeof( bits ) );
    return bits == 0;
}

// Independent reference determinant (Bareiss fraction-free elimination), 4x4.
// Used only to cross-check the library's LU-product determinant; it shares no code
// with feng::matrix (no LU, no recursion, no inverse).
static double bareiss_det( double const m[4][4] )
{
    double b[4][4];
    for ( int i = 0; i < 4; ++i )
        for ( int j = 0; j < 4; ++j )
            b[i][j] = m[i][j];
    for ( int k = 0; k < 3; ++k )
        for ( int i = k + 1; i < 4; ++i )
            for ( int j = k + 1; j < 4; ++j )
                if ( k == 0 )
                    b[i][j] = b[i][j] * b[k][k] - b[i][k] * b[k][j];
                else
                    b[i][j] = ( b[i][j] * b[k][k] - b[i][k] * b[k][j] ) / b[k - 1][k - 1];
    return b[3][3];
}

TEST_CASE( "Matrix determinant", "[det]" )
{
    using feng::matrix;

    // Scenario: singular block matrix — det is EXACTLY 0 (pre-fix Schur recursion
    // produced -nan via the unguarded P.inverse()). P7 policy: exact zero pivot -> 0,
    // no epsilon threshold.
    {
        matrix<double> const s{ 4, 4,
                                { 1.0, 2.0, 0.0, 0.0,
                                  2.0, 4.0, 0.0, 0.0,
                                  0.0, 0.0, 1.0, 1.0,
                                  0.0, 0.0, 1.0, 2.0 } };
        double const ds = s.det();
        // Bit-level: pre-fix returns -nan (bits 0xfff8...), which also passes a plain
        // "ds == 0.0" under the suite's fast-math flags — the bit check is the genuine red.
        REQUIRE( det_is_zero( ds ) );
    }

    // Scenario: 1x1 — the entry itself (0 -> 0, 5 -> 5).
    {
        REQUIRE( matrix<double>{ 1, 1, { 0.0 } }.det() == 0.0 );
        REQUIRE( matrix<double>{ 1, 1, { 5.0 } }.det() == 5.0 );
    }

    // Scenario: 2x2 anti-identity — det = -1 (sign path through the pivot swap).
    {
        matrix<double> const a{ 2, 2, { 0.0, 1.0, 1.0, 0.0 } };
        REQUIRE( a.det() == -1.0 );
    }

    // Scenario: 2x2 with a required row swap — det = 2*2 - 1*3 = 1.
    // (Tolerance-based: the LU multiplier 2/3 is not exactly representable.)
    {
        matrix<double> const a{ 2, 2, { 2.0, 1.0, 3.0, 2.0 } };
        bool const close = ( std::abs( a.det() - 1.0 ) < 1.0e-12 );
        REQUIRE( close );
    }

    // Scenario: 3x3 requiring two row swaps — hand-derived det = 16
    // (A = [0 1 2; 1 0 3; 4 5 6], LU from the lu_pivoting cases: sign=+1, U_ii = 4,1,4).
    {
        matrix<double> const a{ 3, 3, { 0.0, 1.0, 2.0, 1.0, 0.0, 3.0, 4.0, 5.0, 6.0 } };
        double const d = a.det();
        bool const close = ( std::abs( d - 16.0 ) < 1.0e-12 );
        REQUIRE( close );
    }

    // Scenario: known-nonsingular 4x4 (det = 51, hand-verified) vs the independent
    // Bareiss reference — guards against sign/permutation errors in the LU product.
    {
        matrix<double> const a{ 4, 4,
                                { 1.0, 2.0, 3.0, 4.0,
                                  2.0, 1.0, 3.0, 2.0,
                                  3.0, 3.0, 1.0, 0.0,
                                  4.0, 0.0, 2.0, 1.0 } };
        double const m4[4][4] = { { 1, 2, 3, 4 }, { 2, 1, 3, 2 }, { 3, 3, 1, 0 }, { 4, 0, 2, 1 } };
        double const ref = bareiss_det( m4 );
        REQUIRE( ref == 51.0 );
        double const d = a.det();
        bool const close = ( std::abs( d - ref ) < 1.0e-9 * std::abs( ref ) );
        REQUIRE( close );
    }

    // Scenario: near-singular (not singular) — det is nonzero and close to the
    // product of the diagonal; no epsilon threshold may zero it out.
    {
        matrix<double> const a{ 2, 2, { 1.0, 0.0, 0.0, 1.0e-14 } };
        double const d = a.det();
        REQUIRE( d != 0.0 );
        bool const close = ( std::abs( d - 1.0e-14 ) < 1.0e-30 );
        REQUIRE( close );
    }
}
