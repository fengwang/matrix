// S6-R4 (PR-9): extrema start from element 0; mean, variance and standard_deviation result types, the shared
// ddof parameter (D-025) and empty-input aborts (D-011) (F15).
#include <cmath>
#include <complex>
#include <type_traits>
#include <utility>

#include "s2_death.hpp"

namespace s6_r4
{
    template< typename T >
    void check_negative_extrema()
    {
        feng::matrix<T> const m{ 1, 2, { T( -3 ), T( -1 ) } };
        REQUIRE( m.max() == T( -1 ) );
        REQUIRE( m.min() == T( -3 ) );
        auto const less = []( T const& x, T const& y ) { return x < y; };
        REQUIRE( m.max( less ) == T( -1 ) );
        REQUIRE( m.min( less ) == T( -3 ) );
        auto const [lo, hi] = m.minmax();
        REQUIRE( lo == T( -3 ) );
        REQUIRE( hi == T( -1 ) );
        auto const [lo2, hi2] = m.minmax( less );
        REQUIRE( lo2 == T( -3 ) );
        REQUIRE( hi2 == T( -1 ) );
        REQUIRE( feng::max( m ) == T( -1 ) );
        REQUIRE( feng::min( m ) == T( -3 ) );
        feng::matrix<T> const p{ 1, 2, { T( 1 ), T( 3 ) } };
        REQUIRE( p.min() == T( 1 ) );
        REQUIRE( p.max() == T( 3 ) );
        auto const [plo, phi] = p.minmax();
        REQUIRE( plo == T( 1 ) );
        REQUIRE( phi == T( 3 ) );
        REQUIRE( feng::min( p ) == T( 1 ) );
    }

    template< typename M >
    using mean_t = decltype( feng::mean( std::declval< M const& >() ) );
    template< typename M >
    using var_t = decltype( feng::variance( std::declval< M const& >() ) );
    template< typename M >
    using std_t = decltype( feng::standard_deviation( std::declval< M const& >() ) );

    using mi = feng::matrix<int>;
    using mu = feng::matrix<unsigned>;
    using mf = feng::matrix<float>;
    using md = feng::matrix<double>;
    using mc = feng::matrix<std::complex<double>>;
    static_assert( std::is_same_v< std::remove_cv_t< mean_t<mi> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< mean_t<mu> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< mean_t<mf> >, float > );
    static_assert( std::is_same_v< std::remove_cv_t< mean_t<md> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< mean_t<mc> >, std::complex<double> > );
    static_assert( std::is_same_v< std::remove_cv_t< var_t<mi> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< var_t<mu> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< var_t<mf> >, float > );
    static_assert( std::is_same_v< std::remove_cv_t< var_t<md> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< var_t<mc> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< std_t<mi> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< std_t<mu> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< std_t<mf> >, float > );
    static_assert( std::is_same_v< std::remove_cv_t< std_t<md> >, double > );
    static_assert( std::is_same_v< std::remove_cv_t< std_t<mc> >, double > );
    using mcf = feng::matrix<std::complex<float>>;
    static_assert( std::is_same_v< std::remove_cv_t< var_t<mcf> >, float > );
    static_assert( noexcept( feng::variance( std::declval< md const& >(), 1 ) ) );
    static_assert( noexcept( feng::standard_deviation( std::declval< md const& >(), 1 ) ) );
}

TEST_CASE( "S6-R4 max and min of all-negative matrices", "[S6][S6-R4]" )
{
    s6_r4::check_negative_extrema<float>();
    s6_r4::check_negative_extrema<double>();
    s6_r4::check_negative_extrema<int>();
}

TEST_CASE( "S6-R4 mean of integer matrices is a floating value", "[S6][S6-R4]" )
{
    REQUIRE( feng::mean( feng::matrix<int>{ 1, 2, { 1, 2 } } ) == 1.5 );
    REQUIRE( feng::mean( feng::matrix<int>{ 1, 2, { -1, -2 } } ) == -1.5 );
    REQUIRE( feng::mean( feng::matrix<unsigned>{ 1, 2, { 1U, 2U } } ) == 1.5 );
    REQUIRE( feng::mean( feng::matrix<float>{ 1, 2, { 1.0f, 2.0f } } ) == 1.5f );
    auto const z = feng::mean( feng::matrix<std::complex<double>>{ 1, 2, { { 1.0, 2.0 }, { 2.0, -4.0 } } } );
    REQUIRE( z == std::complex<double>{ 1.5, -1.0 } );
}

TEST_CASE( "S6-R4 variance and standard_deviation share ddof", "[S6][S6-R4]" )
{
    feng::matrix<int> const m{ 1, 4, { 1, 2, 3, 4 } };
    // mean 2.5; squared deviations 2.25 + 0.25 + 0.25 + 2.25 = 5
    REQUIRE( feng::variance( m ) == 1.25 );
    REQUIRE( feng::variance( m, 0 ) == 1.25 );
    REQUIRE( std::abs( feng::variance( m, 1 ) - 5.0 / 3.0 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::standard_deviation( m ) - std::sqrt( 1.25 ) ) < 1.0e-12 );
    REQUIRE( std::abs( feng::standard_deviation( m, 1 ) - std::sqrt( 5.0 / 3.0 ) ) < 1.0e-12 );
    feng::matrix<double> const d{ 2, 2, { 1.0, 2.0, 3.0, 4.0 } };
    REQUIRE( feng::variance( d ) == 1.25 );
    REQUIRE( feng::standard_deviation( d, 1 ) * feng::standard_deviation( d, 1 ) == Approx( feng::variance( d, 1 ) ) );
    feng::matrix<unsigned> const u{ 1, 2, { 1U, 3U } };
    REQUIRE( feng::variance( u ) == 1.0 ); // no unsigned wrap of x - mean
    // complex: |z - mean|^2 with mean (0, 0): |1+i|^2 = |-1-i|^2 = 2
    feng::matrix<std::complex<double>> const c{ 1, 2, { { 1.0, 1.0 }, { -1.0, -1.0 } } };
    REQUIRE( feng::variance( c ) == 2.0 );
    REQUIRE( feng::standard_deviation( c ) == std::sqrt( 2.0 ) );
    feng::matrix<std::complex<float>> const cf{ 1, 2, { { 1.0f, 1.0f }, { -1.0f, -1.0f } } };
    auto const vf = feng::variance( cf );
    static_assert( std::is_same_v< std::remove_cv_t< decltype( vf ) >, float > );
    REQUIRE( vf == 2.0f );
}

TEST_CASE( "S6-R4 empty inputs abort with a message", "[S6][S6-R4]" )
{
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)e.max(); }, "matrix max: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)e.min(); }, "matrix min: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)e.minmax(); }, "matrix minmax: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)feng::max( e ); }, "feng::max: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)feng::min( e ); }, "feng::min: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<int> const e; (void)feng::mean( e ); }, "feng::mean: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)feng::variance( e ); }, "feng::variance: " );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)feng::standard_deviation( e ); }, "feng::standard_deviation: " );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const one{ 1, 1, { 2.0 } }; (void)feng::variance( one, 1 ); }, "feng::variance: " );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const one{ 1, 1, { 2.0 } }; (void)feng::standard_deviation( one, 1 ); }, "feng::standard_deviation: " );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)e.max( []( double x, double y ) { return x < y; } ); }, "matrix max: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)e.min( []( double x, double y ) { return x < y; } ); }, "matrix min: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<double> const e; (void)e.minmax( []( double x, double y ) { return x < y; } ); }, "matrix minmax: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<std::complex<double>> const e; (void)feng::mean( e ); }, "feng::mean: empty matrix" );
    S2_REQUIRE_DEATH( []{ feng::matrix<std::complex<double>> const e; (void)feng::variance( e ); }, "feng::variance: needs more elements than ddof" );
    S2_REQUIRE_DEATH( []{ feng::matrix<std::complex<double>> const e; (void)feng::standard_deviation( e ); }, "feng::standard_deviation: needs more elements than ddof" );
    S2_REQUIRE_DEATH( []{ feng::matrix<int> const e; (void)feng::variance( e ); }, "feng::variance: needs more elements than ddof" );
    S2_REQUIRE_DEATH( []{ feng::matrix<int> const e; (void)feng::standard_deviation( e ); }, "feng::standard_deviation: needs more elements than ddof" );
}
