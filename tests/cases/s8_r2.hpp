// S8-R2 (PR-11, F14): fft and ifft follow numpy.fft.fft2 and ifft2: X[k][l] = sum x[m][n] exp(-2 pi i (km/R + ln/C)),
// ifft uses exp(+...) and divides by R*C; result types per D-031. The oracle is a tests-only O(N^2) direct DFT
// accumulating in long double (D-007). Tolerances: eps of the result's real type, N = R*C, L = ceil(log2(2N)).
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>
#include <utility>
#include <vector>

namespace s8r2
{
    template< typename T > struct real_of { using type = T; };
    template< typename T > struct real_of< std::complex< T > > { using type = T; };
    template< typename T > using real_of_t = typename real_of< T >::type;

    // L = ceil(log2(2N))
    inline long double big_l( std::size_t n )
    {
        long double l = 0;
        for ( std::size_t p = 1; p < 2 * n; p *= 2 ) l += 1;
        return l;
    }

    // the oracle: direct 2-D DFT with sign -1 (fft) or +1 (ifft, without the scale), in long double
    template< typename T >
    feng::matrix< std::complex< long double > > direct_dft( feng::matrix< T > const& x, int sign )
    {
        using cl = std::complex< long double >;
        std::size_t const R = x.row();
        std::size_t const C = x.col();
        feng::matrix< cl > out{ R, C };
        long double const two_pi = 2.0L * std::numbers::pi_v< long double >;
        // exp(sign 2 pi i j / R) and exp(sign 2 pi i j / C) for every residue j; the phase index is reduced exactly
        std::vector< cl > wr( R ), wc( C );
        for ( std::size_t j = 0; j != R; ++j )
            wr[j] = std::polar( 1.0L, static_cast< long double >( sign ) * two_pi * static_cast< long double >( j ) / static_cast< long double >( R ) );
        for ( std::size_t j = 0; j != C; ++j )
            wc[j] = std::polar( 1.0L, static_cast< long double >( sign ) * two_pi * static_cast< long double >( j ) / static_cast< long double >( C ) );
        for ( std::size_t k = 0; k != R; ++k )
            for ( std::size_t l = 0; l != C; ++l )
            {
                cl acc{ 0.0L, 0.0L };
                for ( std::size_t m = 0; m != R; ++m )
                {
                    cl const w_r = wr[( k * m ) % R];
                    for ( std::size_t n = 0; n != C; ++n )
                    {
                        cl const v{ static_cast< long double >( std::real( x[m][n] ) ),
                                    static_cast< long double >( std::imag( x[m][n] ) ) };
                        acc += v * ( w_r * wc[( l * n ) % C] );
                    }
                }
                out[k][l] = acc;
            }
        return out;
    }

    // deterministic values in [-1, 1]
    template< typename T >
    feng::matrix< T > sample( std::size_t r, std::size_t c, std::uint64_t seed )
    {
        feng::matrix< T > m{ r, c };
        std::uint64_t s = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        auto next = [&s]()
        {
            s = s * 6364136223846793005ULL + 1442695040888963407ULL;
            return static_cast< double >( s >> 11 ) / 9007199254740992.0 * 2.0 - 1.0;
        };
        using R = real_of_t< T >;
        for ( std::size_t i = 0; i != r * c; ++i )
        {
            if constexpr ( std::is_integral_v< T > )
                m.data()[i] = static_cast< T >( std::lround( next() * 9.0 ) ); // -9 .. 9
            else if constexpr ( std::is_same_v< T, R > )
                m.data()[i] = static_cast< R >( next() );
            else
            {
                R const re = static_cast< R >( next() );
                R const im = static_cast< R >( next() );
                m.data()[i] = T{ re, im };
            }
        }
        return m;
    }

    template< typename T >
    long double sum_abs( feng::matrix< T > const& x )
    {
        long double s = 0;
        for ( std::size_t i = 0; i != x.size(); ++i ) s += static_cast< long double >( std::abs( x.data()[i] ) );
        return s;
    }

    template< typename T >
    long double max_abs( feng::matrix< T > const& x )
    {
        long double s = 0;
        for ( std::size_t i = 0; i != x.size(); ++i )
            s = std::max( s, static_cast< long double >( std::abs( x.data()[i] ) ) );
        return s;
    }

    // max |a - b| elementwise, b in long double
    template< typename C >
    long double max_diff( feng::matrix< C > const& a, feng::matrix< std::complex< long double > > const& b )
    {
        long double d = 0;
        for ( std::size_t i = 0; i != a.size(); ++i )
        {
            std::complex< long double > const v{ static_cast< long double >( a.data()[i].real() ),
                                                 static_cast< long double >( a.data()[i].imag() ) };
            d = std::max( d, std::abs( v - b.data()[i] ) );
        }
        return d;
    }

    template< typename T >
    feng::matrix< std::complex< long double > > widen( feng::matrix< T > const& x )
    {
        feng::matrix< std::complex< long double > > out{ x.row(), x.col() };
        for ( std::size_t i = 0; i != x.size(); ++i )
            out.data()[i] = { static_cast< long double >( std::real( x.data()[i] ) ),
                              static_cast< long double >( std::imag( x.data()[i] ) ) };
        return out;
    }

    template< typename T >
    void check_against_direct( std::size_t r, std::size_t c )
    {
        using R = typename decltype( feng::fft( std::declval< feng::matrix< T > const& >() ) )::value_type::value_type; // D-031
        INFO( r << "x" << c << " " << ( std::is_same_v< T, real_of_t< T > > ? "real" : "complex" ) << " eps "
                << std::numeric_limits< R >::epsilon() );
        auto const x = sample< T >( r, c, 17 * r + c );
        long double const n = static_cast< long double >( r * c );
        long double const eps = static_cast< long double >( std::numeric_limits< R >::epsilon() );
        long double const l = big_l( r * c );

        auto const X = feng::fft( x );
        REQUIRE( X.row() == r );
        REQUIRE( X.col() == c );
        long double const tol_f = 16.0L * l * eps * sum_abs( x );
        long double const df = max_diff( X, direct_dft( x, -1 ) );
        INFO( "fft error " << static_cast< double >( df ) << " tol " << static_cast< double >( tol_f ) );
        CHECK( df <= tol_f );

        // ifft against the direct inverse DFT divided by R*C
        auto const xi = feng::ifft( x );
        auto inv = direct_dft( x, +1 );
        for ( std::size_t i = 0; i != inv.size(); ++i ) inv.data()[i] /= n;
        long double const di = max_diff( xi, inv );
        INFO( "ifft error " << static_cast< double >( di ) << " tol " << static_cast< double >( tol_f / n ) );
        CHECK( di <= tol_f / n );

        // ifft(fft(x)) within 16 L eps max|x| of x
        auto const back = feng::ifft( X );
        long double const dr = max_diff( back, widen( x ) );
        long double const tol_r = 16.0L * l * eps * max_abs( x );
        INFO( "roundtrip error " << static_cast< double >( dr ) << " tol " << static_cast< double >( tol_r ) );
        CHECK( dr <= tol_r );
    }

    inline std::pair< std::size_t, std::size_t > const sizes[] = {
        { 1, 1 }, { 1, 2 }, { 1, 7 }, { 1, 16 }, { 3, 5 }, { 4, 6 }, { 7, 11 }, { 13, 1 },
        { 31, 17 }, { 8, 8 }, { 16, 32 }, { 64, 64 } };

    template< typename T >
    void check_all_sizes()
    {
        for ( auto [r, c] : sizes ) check_against_direct< T >( r, c );
    }
}

TEST_CASE( "S8-R2 fft matches the direct DFT on every listed size", "[S8][S8-R2]" )
{
    SECTION( "double" ) { s8r2::check_all_sizes< double >(); }
    SECTION( "complex<double>" ) { s8r2::check_all_sizes< std::complex< double > >(); }
    SECTION( "float" ) { s8r2::check_all_sizes< float >(); }
    SECTION( "complex<float>" ) { s8r2::check_all_sizes< std::complex< float > >(); }
    SECTION( "long double" ) { s8r2::check_all_sizes< long double >(); }
}

TEST_CASE( "S8-R2 fft of an off-origin impulse is the closed-form phase ramp", "[S8][S8-R2]" )
{
    for ( auto [R, C, r0, c0] : { std::array< std::size_t, 4 >{ 8, 8, 3, 5 }, std::array< std::size_t, 4 >{ 7, 11, 2, 9 },
                                  std::array< std::size_t, 4 >{ 4, 6, 0, 1 }, std::array< std::size_t, 4 >{ 1, 7, 0, 4 } } )
    {
        INFO( R << "x" << C << " impulse at " << r0 << "," << c0 );
        feng::matrix< double > x{ R, C };
        std::fill( x.begin(), x.end(), 0.0 );
        x[r0][c0] = 1.0;
        auto const X = feng::fft( x );
        long double const tol = 16.0L * s8r2::big_l( R * C ) * std::numeric_limits< double >::epsilon();
        long double const two_pi = 2.0L * std::numbers::pi_v< long double >;
        for ( std::size_t k = 0; k != R; ++k )
            for ( std::size_t l = 0; l != C; ++l )
            {
                long double const ph = static_cast< long double >( ( k * r0 ) % R ) / static_cast< long double >( R )
                                     + static_cast< long double >( ( l * c0 ) % C ) / static_cast< long double >( C );
                std::complex< long double > const want = std::polar( 1.0L, -two_pi * ph );
                std::complex< long double > const got{ X[k][l].real(), X[k][l].imag() };
                CHECK( std::abs( got - want ) <= tol );
            }
    }
}

TEST_CASE( "S8-R2 fft of a sinusoid peaks at (a, b) and (-a, -b)", "[S8][S8-R2]" )
{
    for ( auto [R, C, a, b] : { std::array< std::size_t, 4 >{ 8, 12, 1, 3 }, std::array< std::size_t, 4 >{ 7, 10, 2, 3 },
                                std::array< std::size_t, 4 >{ 16, 16, 5, 2 } } )
    {
        INFO( R << "x" << C << " frequency " << a << "," << b );
        feng::matrix< double > x{ R, C };
        long double const two_pi = 2.0L * std::numbers::pi_v< long double >;
        for ( std::size_t m = 0; m != R; ++m )
            for ( std::size_t n = 0; n != C; ++n )
            {
                long double const ph = static_cast< long double >( ( a * m ) % R ) / static_cast< long double >( R )
                                     + static_cast< long double >( ( b * n ) % C ) / static_cast< long double >( C );
                x[m][n] = static_cast< double >( std::cos( two_pi * ph ) );
            }
        auto const X = feng::fft( x );
        long double const tol = 16.0L * s8r2::big_l( R * C ) * std::numeric_limits< double >::epsilon() * s8r2::sum_abs( x );
        std::size_t const ka = ( R - a ) % R;
        std::size_t const kb = ( C - b ) % C;
        for ( std::size_t k = 0; k != R; ++k )
            for ( std::size_t l = 0; l != C; ++l )
            {
                bool const peak = ( k == a && l == b ) || ( k == ka && l == kb );
                long double const want = peak ? static_cast< long double >( R * C ) / 2.0L : 0.0L;
                std::complex< long double > const got{ X[k][l].real(), X[k][l].imag() };
                CHECK( std::abs( got - std::complex< long double >{ want, 0.0L } ) <= tol );
            }
    }
}

TEST_CASE( "S8-R2 ifft divides by R*C", "[S8][S8-R2]" )
{
    for ( auto [R, C] : { std::pair< std::size_t, std::size_t >{ 4, 6 }, std::pair< std::size_t, std::size_t >{ 7, 11 },
                          std::pair< std::size_t, std::size_t >{ 8, 8 } } )
    {
        INFO( R << "x" << C );
        long double const tol = 16.0L * s8r2::big_l( R * C ) * std::numeric_limits< double >::epsilon() * 3.0L;
        // ifft of the constant 3 is 3 at the origin and 0 elsewhere
        feng::matrix< double > k{ R, C };
        std::fill( k.begin(), k.end(), 3.0 );
        auto const xi = feng::ifft( k );
        for ( std::size_t i = 0; i != R; ++i )
            for ( std::size_t j = 0; j != C; ++j )
                CHECK( std::abs( xi[i][j] - std::complex< double >{ ( i == 0 && j == 0 ) ? 3.0 : 0.0, 0.0 } ) <= tol );
        // ifft of an origin impulse of height 3*R*C is the constant 3
        feng::matrix< double > d{ R, C };
        std::fill( d.begin(), d.end(), 0.0 );
        d[0][0] = 3.0 * static_cast< double >( R * C );
        auto const xd = feng::ifft( d );
        for ( std::size_t i = 0; i != xd.size(); ++i )
            CHECK( std::abs( xd.data()[i] - std::complex< double >{ 3.0, 0.0 } ) <= tol );
    }
}

TEST_CASE( "S8-R2 fft and ifft of an empty matrix keep its shape", "[S8][S8-R2]" )
{
    for ( auto [R, C] : { std::pair< std::size_t, std::size_t >{ 0, 0 }, std::pair< std::size_t, std::size_t >{ 0, 3 },
                          std::pair< std::size_t, std::size_t >{ 5, 0 } } )
    {
        feng::matrix< double > const e{ R, C };
        auto const X = feng::fft( e );
        auto const x = feng::ifft( e );
        CHECK( X.row() == R );
        CHECK( X.col() == C );
        CHECK( x.row() == R );
        CHECK( x.col() == C );
        feng::matrix< std::complex< float > > const ec{ R, C };
        CHECK( feng::fft( ec ).size() == 0 );
        CHECK( feng::ifft( ec ).size() == 0 );
    }
}

TEST_CASE( "S8-R2 fft result types follow D-031", "[S8][S8-R2]" )
{
    using std::declval;
    static_assert( std::is_same_v< decltype( feng::fft( declval< feng::matrix< float > const& >() ) ),
                                   feng::matrix< std::complex< float > > > );
    static_assert( std::is_same_v< decltype( feng::fft( declval< feng::matrix< double > const& >() ) ),
                                   feng::matrix< std::complex< double > > > );
    static_assert( std::is_same_v< decltype( feng::fft( declval< feng::matrix< long double > const& >() ) ),
                                   feng::matrix< std::complex< long double > > > );
    static_assert( std::is_same_v< decltype( feng::fft( declval< feng::matrix< int > const& >() ) ),
                                   feng::matrix< std::complex< double > > > );
    static_assert( std::is_same_v< decltype( feng::fft( declval< feng::matrix< std::complex< float > > const& >() ) ),
                                   feng::matrix< std::complex< float > > > );
    static_assert( std::is_same_v< decltype( feng::fft( declval< feng::matrix< std::complex< double > > const& >() ) ),
                                   feng::matrix< std::complex< double > > > );
    static_assert( std::is_same_v< decltype( feng::ifft( declval< feng::matrix< float > const& >() ) ),
                                   feng::matrix< std::complex< float > > > );
    static_assert( std::is_same_v< decltype( feng::ifft( declval< feng::matrix< int > const& >() ) ),
                                   feng::matrix< std::complex< double > > > );
    static_assert( std::is_same_v< decltype( feng::ifft( declval< feng::matrix< long double > const& >() ) ),
                                   feng::matrix< std::complex< long double > > > );
    static_assert( std::is_same_v< decltype( feng::ifft( declval< feng::matrix< std::complex< double > > const& >() ) ),
                                   feng::matrix< std::complex< double > > > );
    static_assert( std::is_same_v< decltype( feng::ifft( declval< feng::matrix< std::complex< float > > const& >() ) ),
                                   feng::matrix< std::complex< float > > > );
    static_assert( std::is_same_v< decltype( feng::ifft( declval< feng::matrix< double > const& >() ) ),
                                   feng::matrix< std::complex< double > > > );
    // values of every D-031 element type against the direct DFT, at the tolerance of the result's real type
    for ( auto [r, c] : { std::pair< std::size_t, std::size_t >{ 3, 5 }, std::pair< std::size_t, std::size_t >{ 8, 8 },
                          std::pair< std::size_t, std::size_t >{ 7, 11 } } )
    {
        s8r2::check_against_direct< float >( r, c );
        s8r2::check_against_direct< long double >( r, c );
        s8r2::check_against_direct< int >( r, c );
        s8r2::check_against_direct< std::complex< float > >( r, c );
        s8r2::check_against_direct< std::complex< double > >( r, c );
    }
    // an int input transforms like its double copy
    feng::matrix< int > xi{ 3, 5 };
    feng::matrix< double > xd{ 3, 5 };
    for ( std::size_t i = 0; i != xi.size(); ++i )
    {
        xi.data()[i] = static_cast< int >( i * 7 % 11 ) - 5;
        xd.data()[i] = static_cast< double >( xi.data()[i] );
    }
    auto const Xi = feng::fft( xi );
    auto const Xd = feng::fft( xd );
    for ( std::size_t i = 0; i != Xi.size(); ++i ) CHECK( Xi.data()[i] == Xd.data()[i] );
    auto const Ii = feng::ifft( xi );
    auto const Id = feng::ifft( xd );
    for ( std::size_t i = 0; i != Ii.size(); ++i ) CHECK( Ii.data()[i] == Id.data()[i] );
}
