// S6-R3 (PR-8): caller-owned random engines (F17, D-024). Elements come from the engine alone, in row-major
// order; floating parts lie in (0, 1); integral elements follow std::uniform_int_distribution over the full range.
#include <complex>
#include <cstdint>
#include <limits>
#include <random>
#include <thread>

namespace s6_r3
{
    template< typename T >
    bool in_open_unit( T x ) { return T{0} < x && x < T{1}; }

    template< typename T >
    void check_open_interval()
    {
        std::mt19937 g{ 2026 };
        auto const m = feng::random<T>( 64, 64, g );
        REQUIRE( m.row() == 64 );
        REQUIRE( m.col() == 64 );
        bool all_inside = true;
        for ( auto const x : m ) all_inside = all_inside && in_open_unit( x );
        REQUIRE( all_inside );
    }

    template< typename T >
    bool element_in_open_unit( T const& x )
    {
        if constexpr ( std::is_floating_point_v<T> ) return in_open_unit( x );
        else return in_open_unit( x.real() ) && in_open_unit( x.imag() );
    }

    template< typename M >
    bool all_in_open_unit( M const& m )
    {
        bool all_inside = true;
        for ( auto const& x : m ) all_inside = all_inside && element_in_open_unit( x );
        return all_inside;
    }

    // every engine overload: random( r, c, g ), random( n, g ), rand( r, c, g ), rand( n, g ), rand_like, random_like
    template< typename T >
    void check_open_interval_all_overloads()
    {
        std::mt19937_64 g{ 2027 };
        auto const a = feng::random<T>( 16, 24, g );
        auto const b = feng::random<T>( 20, g );
        auto const c = feng::rand<T>( 24, 16, g );
        auto const d = feng::rand<T>( 18, g );
        feng::matrix<T> const shape{ 12, 10 };
        auto const e = feng::rand_like( shape, g );
        auto const f = feng::random_like( shape, g );
        REQUIRE( a.row() == 16 );
        REQUIRE( a.col() == 24 );
        REQUIRE( b.row() == 20 );
        REQUIRE( b.col() == 20 );
        REQUIRE( c.row() == 24 );
        REQUIRE( c.col() == 16 );
        REQUIRE( d.row() == 18 );
        REQUIRE( d.col() == 18 );
        REQUIRE( e.row() == 12 );
        REQUIRE( e.col() == 10 );
        REQUIRE( f.row() == 12 );
        REQUIRE( f.col() == 10 );
        REQUIRE( all_in_open_unit( a ) );
        REQUIRE( all_in_open_unit( b ) );
        REQUIRE( all_in_open_unit( c ) );
        REQUIRE( all_in_open_unit( d ) );
        REQUIRE( all_in_open_unit( e ) );
        REQUIRE( all_in_open_unit( f ) );
    }

    template< typename T >
    void check_int_oracle()
    {
        std::mt19937_64 g{ 11 };
        std::mt19937_64 h{ 11 };
        auto const m = feng::random<T>( 9, 7, g );
        using D = std::conditional_t< ( sizeof( T ) < sizeof( int ) ), std::conditional_t< std::is_signed_v<T>, int, unsigned >, T >;
        std::uniform_int_distribution<D> dist{ static_cast<D>( std::numeric_limits<T>::min() ), static_cast<D>( std::numeric_limits<T>::max() ) };
        bool same = true;
        for ( auto const x : m ) same = same && ( x == static_cast<T>( dist( h ) ) );
        REQUIRE( same );
    }
}

TEST_CASE( "S6-R3 equally seeded engines give identical matrices", "[S6][S6-R3]" )
{
    {
        std::mt19937 g{ 5 }, h{ 5 };
        REQUIRE( feng::random<double>( 13, 17, g ) == feng::random<double>( 13, 17, h ) );
        REQUIRE( feng::rand<float>( 13, 17, g ) == feng::rand<float>( 13, 17, h ) );
        REQUIRE( feng::rand<double>( 6, g ) == feng::random<double>( 6, h ) );
        feng::matrix<double> const shape{ 4, 9 };
        auto const a = feng::rand_like( shape, g );
        auto const b = feng::random_like( shape, h );
        REQUIRE( a.row() == 4 );
        REQUIRE( a.col() == 9 );
        REQUIRE( a == b );
    }
    {
        // the legacy seed overload forwards to the same implementation through a local std::mt19937_64
        std::mt19937_64 g{ 7 };
        REQUIRE( feng::rand<double>( 5, 8, 7 ) == feng::random<double>( 5, 8, g ) );
        REQUIRE( feng::rand( 3, 4 ).row() == 3 ); // still the seed overload
    }
    {
        // row-major order: element k is the k-th draw
        std::mt19937 g{ 3 }, h{ 3 };
        auto const m = feng::random<double>( 3, 5, g );
        auto const v = feng::random<double>( 1, 15, h );
        bool same = true;
        for ( std::size_t r = 0; r != 3; ++r )
            for ( std::size_t c = 0; c != 5; ++c )
                same = same && ( m[r][c] == v[0][r * 5 + c] );
        REQUIRE( same );
    }
}

TEST_CASE( "S6-R3 floating and complex elements lie in the open interval (0, 1)", "[S6][S6-R3]" )
{
    s6_r3::check_open_interval<float>();
    s6_r3::check_open_interval<double>();
    s6_r3::check_open_interval<long double>();
    std::mt19937 g{ 99 };
    auto const m = feng::random<std::complex<double>>( 32, 32, g );
    bool all_inside = true;
    for ( auto const z : m ) all_inside = all_inside && s6_r3::in_open_unit( z.real() ) && s6_r3::in_open_unit( z.imag() );
    REQUIRE( all_inside );
    // real part is drawn before the imaginary part
    std::mt19937 h{ 99 }, k{ 99 };
    auto const z = feng::random<std::complex<float>>( 1, 1, h )[0][0];
    auto const p = feng::random<float>( 1, 2, k );
    REQUIRE( z.real() == p[0][0] );
    REQUIRE( z.imag() == p[0][1] );
    s6_r3::check_open_interval_all_overloads<float>();
    s6_r3::check_open_interval_all_overloads<double>();
    s6_r3::check_open_interval_all_overloads<long double>();
    s6_r3::check_open_interval_all_overloads<std::complex<float>>();
    s6_r3::check_open_interval_all_overloads<std::complex<double>>();
    s6_r3::check_open_interval_all_overloads<std::complex<long double>>();
}

TEST_CASE( "S6-R3 integral elements follow a uniform integer distribution", "[S6][S6-R3]" )
{
    s6_r3::check_int_oracle<int>();
    s6_r3::check_int_oracle<std::uint32_t>();
    s6_r3::check_int_oracle<std::int8_t>();
    s6_r3::check_int_oracle<std::uint8_t>();
    s6_r3::check_int_oracle<short>();
    s6_r3::check_int_oracle<unsigned short>();
    s6_r3::check_int_oracle<long long>();
    s6_r3::check_int_oracle<unsigned long long>();
    s6_r3::check_int_oracle<std::int16_t>();
    s6_r3::check_int_oracle<std::uint16_t>();
    s6_r3::check_int_oracle<std::int64_t>();
    s6_r3::check_int_oracle<std::uint64_t>();
    std::mt19937 g{ 1 };
    auto const s = feng::random<std::int8_t>( 32, 32, g );
    auto const u = feng::random<std::uint8_t>( 32, 32, g );
    bool s_neg = false, s_pos = false, u_low = false, u_high = false;
    for ( auto const x : s ) { s_neg = s_neg || x < 0; s_pos = s_pos || x > 0; }
    for ( auto const x : u ) { u_low = u_low || x < 128; u_high = u_high || x >= 128; }
    REQUIRE( s_neg );
    REQUIRE( s_pos );
    REQUIRE( u_low );
    REQUIRE( u_high );
}

TEST_CASE( "S6-R3 engines used from concurrent threads do not interfere", "[S6][S6-R3]" )
{
    feng::matrix<double> a, b;
    std::thread ta( [&a]{ std::mt19937 g{ 42 }; a = feng::random<double>( 256, 256, g ); } );
    std::thread tb( [&b]{ std::mt19937 g{ 42 }; b = feng::random<double>( 256, 256, g ); } );
    ta.join();
    tb.join();
    REQUIRE( a.row() == 256 );
    REQUIRE( a.col() == 256 );
    REQUIRE( a == b );
    std::mt19937 g{ 42 };
    REQUIRE( a == feng::random<double>( 256, 256, g ) );
    // a second pair drawing int
    feng::matrix<int> p, q;
    std::thread tp( [&p]{ std::mt19937 e{ 42 }; p = feng::random<int>( 256, 256, e ); } );
    std::thread tq( [&q]{ std::mt19937 e{ 42 }; q = feng::random<int>( 256, 256, e ); } );
    tp.join();
    tq.join();
    REQUIRE( p.row() == 256 );
    REQUIRE( p.col() == 256 );
    REQUIRE( p == q );
}

TEST_CASE( "S6-R3 legacy seed-only overloads keep their shapes and the open interval", "[S6][S6-R3]" )
{
    feng::matrix<double> const shape{ 5, 3 };
    auto const a = feng::rand<double>( 4, 6, 9 );
    auto const b = feng::rand<double>( 7 );
    auto const c = feng::random<double>( 3, 8 );
    auto const d = feng::random<double>( 6 );
    auto const e = feng::rand_like( shape );
    auto const f = feng::random_like( shape );
    auto const h = feng::randn_like( shape );
    REQUIRE( a.row() == 4 );
    REQUIRE( a.col() == 6 );
    REQUIRE( b.row() == 7 );
    REQUIRE( b.col() == 7 );
    REQUIRE( c.row() == 3 );
    REQUIRE( c.col() == 8 );
    REQUIRE( d.row() == 6 );
    REQUIRE( d.col() == 6 );
    REQUIRE( e.row() == 5 );
    REQUIRE( e.col() == 3 );
    REQUIRE( f.row() == 5 );
    REQUIRE( f.col() == 3 );
    REQUIRE( h.row() == 5 );
    REQUIRE( h.col() == 3 );
    REQUIRE( s6_r3::all_in_open_unit( a ) );
    REQUIRE( s6_r3::all_in_open_unit( b ) );
    REQUIRE( s6_r3::all_in_open_unit( c ) );
    REQUIRE( s6_r3::all_in_open_unit( d ) );
    REQUIRE( s6_r3::all_in_open_unit( e ) );
    REQUIRE( s6_r3::all_in_open_unit( f ) );
    REQUIRE( s6_r3::all_in_open_unit( h ) );
    // the only legacy overload taking a seed: a nonzero seed equals the engine overload with std::mt19937_64{ seed }
    std::mt19937_64 g{ 9 };
    REQUIRE( a == feng::random<double>( 4, 6, g ) );
}
