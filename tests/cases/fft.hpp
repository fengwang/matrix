#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>

// Session 6 (P1/C13): fft/ifft/fftshift/ifftshift.
//
// R-19 suite policy: this build uses -Ofast (fast-math), so every assertion
// here uses tolerances on finite values only — no NaN-dependent checks, no
// bit-equality pins. Exact identity pins live in the -O1 probe
// (.work/probes/E16_E17.cc).
//
// Oracle provenance (FROZEN after S6, R-18): fft_ref below is a
// self-contained copy of the CORRECTED naive 2-D DFT. The pre-fix library
// loops contained a data-index bug (they read x[r][c], the OUTPUT indices,
// instead of x[r_][c_], the input indices, inside the inner sum), so they
// computed R*C*x[0][0] at corner (0,0) and zero everywhere else — not a DFT
// at all (docs/session_6/failure_arbiter.md F1; pre-fix baseline in
// .work/evidence/s6_prefix_probe.log). The one-token fix (x[r_][c_]) defines
// the frozen reference. Do not "improve" this copy in a later session
// without a sanctioned decision.
//
// Tolerances: double 1e-9 (fast-path differential, round-trip, shift pins);
// float 1e-3 for the 8x8 differential and 1e-2 for the 126x128 differential
// (float accumulation in the library vs the double-math oracle; 126x128
// magnitudes ~2.4e4 make the float ULP dominate).

// Self-contained corrected naive 2-D DFT (double math), NumPy fft2
// convention: forward unnormalized; `inverse` selects the conjugate kernel
// with NO scaling (the library's ifft applies the single 1/(R*C)
// normalization itself; ref_inverse returns the unscaled inverse kernel).
template < typename T >
feng::matrix< std::complex< double > > fft_ref( feng::matrix< T > const& x, bool inverse )
{
    using cd = std::complex< double >;
    std::uint_least64_t const R = x.row();
    std::uint_least64_t const C = x.col();

    feng::matrix< cd > X( R, C );

    for ( std::uint_least64_t r = 0; r != R; ++r )
        for ( std::uint_least64_t c = 0; c != C; ++c )
        {
            cd X_rc{ 0.0, 0.0 };
            for ( std::uint_least64_t r_ = 0; r_ != R; ++r_ )
            {
                cd tmp{ 0.0, 0.0 };
                double const theta_r = ( inverse ? 1.0 : -1.0 ) * 2.0 * M_PI * double( r ) * double( r_ ) / double( R );
                for ( std::uint_least64_t c_ = 0; c_ != C; ++c_ )
                {
                    double const theta_c = ( inverse ? 1.0 : -1.0 ) * 2.0 * M_PI * double( c ) * double( c_ ) / double( C );
                    tmp += cd( static_cast< double >( x[r_][c_] ), 0.0 ) * cd( std::cos( theta_c ), std::sin( theta_c ) );
                }
                X_rc += tmp * cd( std::cos( theta_r ), std::sin( theta_r ) );
            }
            X[r][c] = X_rc;
        }

    return X;
}

// Max-norm distance between a library (complex) matrix and the double
// reference.
template < typename T >
double fft_max_diff( feng::matrix< std::complex< T > > const& a, feng::matrix< std::complex< double > > const& b )
{
    double r{ 0.0 };
    for ( std::uint_least64_t i = 0; i != a.row(); ++i )
        for ( std::uint_least64_t j = 0; j != a.col(); ++j )
            r = std::max( r, std::abs( std::complex< double >( a[i][j] ) - b[i][j] ) );
    return r;
}

// Max-norm distance between two real matrices.
template < typename T, typename U >
double fft_real_max_diff( feng::matrix< T > const& a, feng::matrix< U > const& b )
{
    double r{ 0.0 };
    for ( std::uint_least64_t i = 0; i != a.row(); ++i )
        for ( std::uint_least64_t j = 0; j != a.col(); ++j )
            r = std::max( r, std::abs( double( a[i][j] ) - double( b[i][j] ) ) );
    return r;
}

TEST_CASE( "Matrix fft", "[fft]" )
{
    using feng::fft;
    using feng::fftshift;
    using feng::ifft;
    using feng::ifftshift;
    using feng::matrix;
    using cd = std::complex< double >;

    // Scenario: differential vs the frozen oracle, fast path (8x8, both dims power-of-two).
    {
        matrix< double > x( 8, 8 );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                x[r][c] = std::sin( double( r * c ) ) + 0.5 * std::cos( 0.3 * double( r ) - 0.7 * double( c ) );

        REQUIRE( fft_max_diff( fft( x ), fft_ref( x, false ) ) < 1.0e-9 );
        auto const ifft_x = ifft( fft( x ) );
        auto const ref_inv = fft_ref( x, true );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                REQUIRE( std::abs( ifft_x[r][c] * 64.0 - ref_inv[r][c] ) < 1.0e-9 );
    }

    // Scenario: differential vs the frozen oracle, fast path, float (tolerance 1e-3, R-19).
    {
        matrix< float > x( 8, 8 );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                x[r][c] = static_cast< float >( std::sin( double( r * c ) ) + 0.5 * std::cos( 0.3 * double( r ) - 0.7 * double( c ) ) );

        REQUIRE( fft_max_diff( fft( x ), fft_ref( x, false ) ) < 1.0e-3 );
    }

    // Scenario: differential vs the frozen oracle, fallback path (6x8, row not power-of-two).
    {
        matrix< double > x( 6, 8 );
        for ( std::uint_least64_t r = 0; r != 6; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                x[r][c] = 0.25 * std::cos( 0.5 * double( r ) + 0.3 * double( c ) ) + double( r - c ) / 8.0;

        REQUIRE( fft_max_diff( fft( x ), fft_ref( x, false ) ) < 1.0e-9 );
        auto const ifft_x = ifft( fft( x ) );
        auto const ref_inv = fft_ref( x, true );
        for ( std::uint_least64_t r = 0; r != 6; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                REQUIRE( std::abs( ifft_x[r][c] * 48.0 - ref_inv[r][c] ) < 1.0e-9 );
    }

    // Scenario: differential vs the frozen oracle, fallback path at the contract's
    // adversarial size (126x128 float; 126 is not a power of two).
    {
        matrix< float > x( 126, 128 );
        for ( std::uint_least64_t r = 0; r != 126; ++r )
            for ( std::uint_least64_t c = 0; c != 128; ++c )
                x[r][c] = static_cast< float >( std::sin( double( r ) * 0.01 ) * std::cos( double( c ) * 0.01 ) );

        REQUIRE( fft_max_diff( fft( x ), fft_ref( x, false ) ) < 1.0e-2 );
    }

    // Scenario: E16 (in-suite mirror of the contract probe) — fft of the 8x8
    // delta at (0,0) is all ones (NumPy fft2 convention, unnormalized).
    {
        matrix< double > d( 8, 8 );
        std::fill( d.begin(), d.end(), 0.0 );
        d[0][0] = 1.0;

        auto const X = fft( d );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                REQUIRE( std::abs( X[r][c] - cd( 1.0, 0.0 ) ) < 1.0e-9 );
    }

    // Scenario: E16 round-trip — ifft(fft(x)) == x (the 1/(R*C) normalization
    // makes the round-trip an identity; imaginary parts of a real transform
    // round to ~0 and are checked by the same complex modulus).
    {
        matrix< double > x( 8, 8 );
        std::fill( x.begin(), x.end(), 3.0 );
        x[0][0] += 1.0;

        auto const y = ifft( fft( x ) );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                REQUIRE( std::abs( y[r][c] - cd( x[r][c], 0.0 ) ) < 1.0e-9 );
    }

    // Scenario: normalization applied exactly once, on ifft only —
    // ifft(ifft(x)) carries 1/(R*C)^2 (catches a factor on fft, or a
    // doubled/missing factor on ifft).
    {
        matrix< double > x( 8, 8 );
        std::fill( x.begin(), x.end(), 3.0 );
        x[0][0] += 1.0;

        auto const y = ifft( ifft( x ) );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                REQUIRE( std::abs( y[r][c] - cd( x[r][c] / 4096.0, 0.0 ) ) < 1.0e-9 ); // (R*C)^2 = 64^2
    }

    // Scenario: complex round-trip (fast path, 8x8).
    {
        matrix< cd > z( 8, 8 );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                z[r][c] = cd( std::cos( double( r + c ) ), std::sin( double( r - c ) ) );

        auto const w = ifft( fft( z ) );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                REQUIRE( std::abs( w[r][c] - z[r][c] ) < 1.0e-9 );
    }

    // Scenario: E17 odd-3 pins — both fftshift and ifftshift roll by (3+1)/2 = 2,
    // i.e. old positions (1,2,0). The 1x3/3x1 shapes hit the fallback transform
    // (3 is not a power of two). fft([1,2,3]) = [6, -1.5+0.866025i, -1.5-0.866025i].
    {
        cd const F[3] = { cd( 6.0, 0.0 ), cd( -1.5, 0.8660254037844386 ), cd( -1.5, -0.8660254037844386 ) };
        cd const expect[3] = { F[1], F[2], F[0] };

        matrix< double > row( 1, 3 );
        row[0][0] = 1.0; row[0][1] = 2.0; row[0][2] = 3.0;
        auto const fr = fftshift( row );
        auto const ir = ifftshift( row );
        for ( int k = 0; k < 3; ++k )
        {
            REQUIRE( std::abs( fr[0][k] - expect[k] ) < 1.0e-9 );
            REQUIRE( std::abs( ir[0][k] - expect[k] ) < 1.0e-9 );
        }

        matrix< double > col( 3, 1 );
        col[0][0] = 1.0; col[1][0] = 2.0; col[2][0] = 3.0;
        auto const fc = fftshift( col );
        auto const ic = ifftshift( col );
        for ( int k = 0; k < 3; ++k )
        {
            REQUIRE( std::abs( fc[k][0] - expect[k] ) < 1.0e-9 );
            REQUIRE( std::abs( ic[k][0] - expect[k] ) < 1.0e-9 );
        }
    }

    // Scenario: E17 even-4 pins — both functions keep the historical swap-of-halves
    // order (2,3,0,1) (fast path; 4 is a power of two).
    // fft([1,2,3,4]) = [10, -2+2i, -2, -2-2i]; after the roll: [F2, F3, F0, F1].
    {
        cd const F[4] = { cd( 10.0, 0.0 ), cd( -2.0, 2.0 ), cd( -2.0, 0.0 ), cd( -2.0, -2.0 ) };
        cd const expect[4] = { F[2], F[3], F[0], F[1] };

        matrix< double > row( 1, 4 );
        row[0][0] = 1.0; row[0][1] = 2.0; row[0][2] = 3.0; row[0][3] = 4.0;
        auto const fr = fftshift( row );
        auto const ir = ifftshift( row );
        for ( int k = 0; k < 4; ++k )
        {
            REQUIRE( std::abs( fr[0][k] - expect[k] ) < 1.0e-9 );
            REQUIRE( std::abs( ir[0][k] - expect[k] ) < 1.0e-9 );
        }
    }

    // Scenario: C13 odd-5 pin — roll (2,3,4,0,1) (pre-fix remap was (3,4,2,0,1)).
    // Values are oracle-derived (self-contained) rather than hand-computed.
    {
        matrix< double > x( 5, 1 );
        for ( std::uint_least64_t r = 0; r != 5; ++r )
            x[r][0] = double( r + 1 );

        auto const F = fft_ref( x, false );
        int const perm[5] = { 2, 3, 4, 0, 1 };

        auto const fs = fftshift( x );
        auto const is_ = ifftshift( x );
        for ( int k = 0; k < 5; ++k )
        {
            REQUIRE( std::abs( fs[k][0] - F[perm[k]][0] ) < 1.0e-9 );
            REQUIRE( std::abs( is_[k][0] - F[perm[k]][0] ) < 1.0e-9 );
        }
        // and the transform itself is pinned: F[0] = sum = 15.
        REQUIRE( std::abs( F[0][0] - cd( 15.0, 0.0 ) ) < 1.0e-9 );
    }

    // Scenario: even-dimension regression pin — for even dims the roll must
    // reproduce the historical swap-of-halves remap (pre-fix bit-identity).
    // Hand-rolled copy of the pre-fix remap (rows first, then columns).
    {
        matrix< double > x( 4, 8 );
        for ( std::uint_least64_t r = 0; r != 4; ++r )
            for ( std::uint_least64_t c = 0; c != 8; ++c )
                x[r][c] = std::sin( 0.3 * double( r ) ) * std::cos( 0.5 * double( c ) ) + 0.25 * double( r - c );

        matrix< cd > const X = fft( x );
        matrix< cd > swapped = X;
        std::uint_least64_t const R = 4, C = 8;
        std::uint_least64_t const row_starter = ( R >> 1 ) + ( R & 1 );
        for ( std::uint_least64_t i = 0; row_starter + i < R; ++i )
            for ( std::uint_least64_t c = 0; c != C; ++c )
                std::swap( swapped[i][c], swapped[row_starter + i][c] );
        std::uint_least64_t const col_starter = ( C >> 1 ) + ( C & 1 );
        for ( std::uint_least64_t i = 0; col_starter + i < C; ++i )
            for ( std::uint_least64_t r = 0; r != R; ++r )
                std::swap( swapped[r][i], swapped[r][col_starter + i] );

        auto const fs = fftshift( x );
        for ( std::uint_least64_t r = 0; r != R; ++r )
            for ( std::uint_least64_t c = 0; c != C; ++c )
                REQUIRE( std::abs( fs[r][c] - swapped[r][c] ) < 1.0e-9 );
    }

    // Scenario: strides — 1x8 and 8x1 (row-only / column-only fast path) vs oracle.
    {
        matrix< double > row( 1, 8 );
        for ( std::uint_least64_t c = 0; c != 8; ++c )
            row[0][c] = double( c + 1 );
        REQUIRE( fft_max_diff( fft( row ), fft_ref( row, false ) ) < 1.0e-9 );

        matrix< double > col( 8, 1 );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            col[r][0] = double( r + 1 );
        REQUIRE( fft_max_diff( fft( col ), fft_ref( col, false ) ) < 1.0e-9 );
        auto const ifft_col = ifft( fft( col ) );
        auto const ref_col = fft_ref( col, true );
        for ( std::uint_least64_t r = 0; r != 8; ++r )
            REQUIRE( std::abs( ifft_col[r][0] * 8.0 - ref_col[r][0] ) < 1.0e-9 );
    }

    // Scenario: 1x1 identity (fft is the identity; ifft scales by 1/(1*1) = 1).
    {
        matrix< double > x( 1, 1 );
        x[0][0] = 2.5;
        REQUIRE( std::abs( fft( x )[0][0] - cd( 2.5, 0.0 ) ) < 1.0e-12 );
        REQUIRE( std::abs( ifft( x )[0][0] - cd( 2.5, 0.0 ) ) < 1.0e-12 );
    }

    // Scenario: empty input — pre-fix guard behavior (empty out, no crash).
    {
        matrix< double > e;
        auto const X = fft( e );
        REQUIRE( X.row() == 0 );
        REQUIRE( X.col() == 0 );
    }

    // Scenario: purity — the input is not mutated by any of the four functions.
    {
        matrix< double > x( 4, 4 );
        for ( std::uint_least64_t r = 0; r != 4; ++r )
            for ( std::uint_least64_t c = 0; c != 4; ++c )
                x[r][c] = 0.5 * double( r + 1 ) * std::cos( double( c ) );
        matrix< double > const x0 = x;

        fft( x );
        ifft( fft( x ) );
        fftshift( x );
        ifftshift( x );

        REQUIRE( fft_real_max_diff( x, x0 ) == 0.0 );
    }
}
