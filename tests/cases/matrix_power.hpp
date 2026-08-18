#include <cmath>

// Naive power by repeated squaring-free multiplication — independent oracle
// (uses operator* only, never operator^).
static feng::matrix<double> power_loop( feng::matrix<double> const& a, unsigned long n )
{
    feng::matrix<double> x = feng::eye< double >( a.row(), a.col() );
    for ( unsigned long k = 0; k != n; ++k )
        x = x * a;
    return x;
}

TEST_CASE( "Matrix operator^ (integer powers)", "[matrix_power]" )
{
    using feng::matrix;

    // Scenario: closed form for the 2x2 Jordan block A = [1 1; 0 1]: A^k = [1 k; 0 1]
    // for every k. Covers n = 0 (identity), 1 (identity map), and odd n = 3, 5 — the
    // pre-fix odd branch (lhs ^ (n-1) * lhs) was ill-formed for any odd n >= 3, making
    // the entire operator^ a compile error at every call site.
    {
        matrix<double> const A{ 2, 2, { 1.0, 1.0, 0.0, 1.0 } };
        for ( unsigned long n = 0; n <= 5; ++n )
        {
            matrix<double> const P = A ^ n;
            REQUIRE( P.row() == 2 );
            REQUIRE( P.col() == 2 );
            double const off = ( double ) n;
            bool const diag_ok = ( P[0][0] == 1.0 ) && ( P[1][1] == 1.0 );
            REQUIRE( diag_ok );
            bool const zero_ok = ( P[1][0] == 0.0 ) && ( P[0][1] == off );
            REQUIRE( zero_ok );
        }
    }

    // Scenario: 3x3 powers n = 0..5 vs the naive multiplication loop (oracle).
    {
        matrix<double> const A{ 3, 3,
                                { 1.0, 2.0, 0.0,
                                  0.0, 1.0, 3.0,
                                  1.0, 0.0, 1.0 } };
        for ( unsigned long n = 0; n <= 5; ++n )
        {
            matrix<double> const P = A ^ n;
            matrix<double> const R = power_loop( A, n );
            for ( unsigned long i = 0; i != 3; ++i )
                for ( unsigned long j = 0; j != 3; ++j )
                {
                    bool const close = ( std::abs( P[i][j] - R[i][j] ) < 1.0e-9 );
                    REQUIRE( close );
                }
        }
    }

    // Scenario: 1x1 scalar powers — 2^k is exact in double for these k.
    {
        matrix<double> const A{ 1, 1, { 2.0 } };
        // Note: parenthesized — C++ "^" (xor) binds tighter than "==".
        REQUIRE( ( A ^ 0 ) == matrix<double>{ 1, 1, { 1.0 } } );
        REQUIRE( ( A ^ 1 ) == A );
        REQUIRE( ( A ^ 2 ) == matrix<double>{ 1, 1, { 4.0 } } );
        REQUIRE( ( A ^ 3 ) == matrix<double>{ 1, 1, { 8.0 } } );
        REQUIRE( ( A ^ 4 ) == matrix<double>{ 1, 1, { 16.0 } } );
        REQUIRE( ( A ^ 5 ) == matrix<double>{ 1, 1, { 32.0 } } );
    }
}
