#include <cmath>
TEST_CASE( "Matrix pinv", "[pinv]" )
{
    // C4 + R1 regression (spec: docs/session_3/specs/pseudoinverse.md).
    // pinv computes the Moore-Penrose pseudoinverse through the single
    // SVD-inversion core (matrix_details::pinv_core, moved as-is from the retired
    // svd_inverse, A2). Threshold: a singular value sigma is inverted
    // iff |sigma| > 1.0e-10 (inherited rule; strict — P7: no new epsilon).
    // Domain note (D4): the SVD core is numerically valid for tall/square matrices;
    // no MP assertions are made on m < n matrices (wide-SVD gap, S6 candidate).

    // Max-norm Moore-Penrose residuals of p against a:
    //   R1 ||a.p.a - a||, R2 ||p.a.p - p||, R3 ||(a.p)' - a.p||, R4 ||(p.a)' - p.a||.
    auto mp_residual = []( feng::matrix<double> const& a, feng::matrix<double> const& p ) -> double
    {
        feng::matrix<double> const apa = a * p * a;
        feng::matrix<double> const pap = p * a * p;
        feng::matrix<double> const ap  = a * p;
        feng::matrix<double> const pa  = p * a;
        auto max_diff = []( auto const& x, auto const& y ) -> double
        {
            double r{ 0.0 };
            for ( unsigned long i = 0; i != x.row(); ++i )
                for ( unsigned long j = 0; j != x.col(); ++j )
                    r = std::max( r, std::abs( x[i][j] - y[i][j] ) );
            return r;
        };
        double r = std::max( max_diff( apa, a ), max_diff( pap, p ) );
        for ( unsigned long i = 0; i != ap.row(); ++i )
            for ( unsigned long j = 0; j != ap.col(); ++j )
                r = std::max( r, std::abs( ap[i][j] - ap[j][i] ) );
        for ( unsigned long i = 0; i != pa.row(); ++i )
            for ( unsigned long j = 0; j != pa.col(); ++j )
                r = std::max( r, std::abs( pa[i][j] - pa[j][i] ) );
        return r;
    };

    // Scenario: diagonal 2x2 (E06 acceptance) — pre-fix returned diag(1,2) (no inversion).
    {
        feng::matrix<double> const m{ 2, 2, { 1.0, 0.0, 0.0, 2.0 } };
        feng::matrix<double> const p = feng::pinv( m );
        REQUIRE( p.row() == 2 );
        REQUIRE( p.col() == 2 );
        REQUIRE( std::abs( p[0][0] - 1.0 ) < 1.0e-8 );
        REQUIRE( std::abs( p[0][1] ) < 1.0e-8 );
        REQUIRE( std::abs( p[1][0] ) < 1.0e-8 );
        REQUIRE( std::abs( p[1][1] - 0.5 ) < 1.0e-8 );
    }

    // Scenario: pinv is deterministic (two calls, bitwise equal).
    {
        feng::matrix<double> const m{ 3, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 10.0 } };
        feng::matrix<double> const p1 = feng::pinv( m );
        feng::matrix<double> const p2 = feng::pinv( m );
        for ( unsigned long i = 0; i != 3; ++i )
            for ( unsigned long j = 0; j != 3; ++j )
                REQUIRE( p1[i][j] == p2[i][j] );
    }

    // Scenario: 4x2 full-column-rank — all four Moore-Penrose conditions.
    {
        feng::matrix<double> const a{ 4, 2, { 1.0, 0.0, 0.0, 1.0, 2.0, 0.0, 0.0, 3.0 } };
        feng::matrix<double> const p = feng::pinv( a );
        REQUIRE( p.row() == 2 );
        REQUIRE( p.col() == 4 );
        REQUIRE( mp_residual( a, p ) < 1.0e-8 );
    }

    // Scenario: 4x2 rank-1 (D4 adversarial case) — all four Moore-Penrose conditions.
    {
        feng::matrix<double> const a{ 4, 2, { 1.0, 2.0, 2.0, 4.0, 3.0, 6.0, 4.0, 8.0 } };
        feng::matrix<double> const p = feng::pinv( a );
        REQUIRE( p.row() == 2 );
        REQUIRE( p.col() == 4 );
        REQUIRE( mp_residual( a, p ) < 1.0e-8 );
    }

    // Scenario: 3x3 diag(1,1,0) — rank 2; sigma == 0 stays 0.
    {
        feng::matrix<double> const a{ 3, 3, { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 } };
        feng::matrix<double> const p = feng::pinv( a );
        REQUIRE( p.row() == 3 );
        REQUIRE( p.col() == 3 );
        for ( unsigned long i = 0; i != 3; ++i )
            for ( unsigned long j = 0; j != 3; ++j )
                REQUIRE( std::abs( p[i][j] - a[i][j] ) < 1.0e-12 );
    }

    // Scenario: all-zero 2x2 -> all-zero pseudoinverse.
    {
        feng::matrix<double> const a{ 2, 2, { 0.0, 0.0, 0.0, 0.0 } };
        feng::matrix<double> const p = feng::pinv( a );
        for ( unsigned long i = 0; i != 2; ++i )
            for ( unsigned long j = 0; j != 2; ++j )
                REQUIRE( std::abs( p[i][j] ) < 1.0e-12 );
    }

    // Scenario: threshold boundary — |sigma| > 1e-10 strict (inherited rule).
    {
        feng::matrix<double> const b1{ 1, 1, { 1.0e-10 } };
        feng::matrix<double> const p1 = feng::pinv( b1 );
        REQUIRE( std::abs( p1[0][0] - 1.0e-10 ) < 1.0e-12 ); // not inverted (not 1e10)

        feng::matrix<double> const b2{ 1, 1, { 2.0e-10 } };
        feng::matrix<double> const p2 = feng::pinv( b2 );
        REQUIRE( std::abs( p2[0][0] - 5.0e9 ) / 5.0e9 < 1.0e-3 ); // inverted to 1/(2e-10)
    }
}
