// S8-R1 (PR-11, F15): conv and conv2 follow scipy.signal.convolve2d: kernel reversed, full (ra+rb-1)x(ca+cb-1),
// same cropped from full at ((rb-1)/2, (cb-1)/2), valid (ra-rb+1)x(ca-cb+1) or from the swapped operands when B
// contains A, otherwise an abort; an unknown mode aborts (D-011); an operand with a zero dimension gives 0x0 for
// full and valid and zeros of A's shape for same (D-030). Literal values below come from scipy 1.18.1.
#include <complex>
#include <csignal>
#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "./s2_death.hpp"

namespace s8r1
{
    template< typename T >
    feng::matrix< T > make( std::size_t r, std::size_t c, std::vector< T > const& v )
    {
        feng::matrix< T > m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i ) m.data()[i] = v[i];
        return m;
    }

    template< typename T >
    bool equal( feng::matrix< T > const& a, feng::matrix< T > const& b )
    {
        if ( a.row() != b.row() || a.col() != b.col() ) return false;
        for ( std::size_t i = 0; i != a.size(); ++i )
            if ( !( a.data()[i] == b.data()[i] ) ) return false;
        return true;
    }

    template< typename T >
    bool zero_by_zero( feng::matrix< T > const& m ) { return m.row() == 0 && m.col() == 0; }

    // A (3x4) and B (2x3) of the asymmetric case
    template< typename T >
    feng::matrix< T > A() { return make< T >( 3, 4, { 1, -2, 3, 0, 4, 5, -1, 2, 0, 3, 2, -4 } ); }
    template< typename T >
    feng::matrix< T > B() { return make< T >( 2, 3, { 1, 0, -1, 2, 1, 3 } ); }

    template< typename Callable >
    void require_one_message_death( Callable&& fn, std::string const& expected )
    {
        s2_death::outcome const out = s2_death::run( std::forward< Callable >( fn ) );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::line_count( out.err ) == 1 );
        REQUIRE( s2_death::contains( out.err, "contract violation" ) );
        REQUIRE( s2_death::contains( out.err, expected ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "AddressSanitizer" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "runtime error:" ) );
    }
}

TEST_CASE( "S8-R1 conv of [1,2] with [3,4] is [3,10,8]", "[S8][S8-R1]" )
{
    auto const a = s8r1::make< double >( 1, 2, { 1, 2 } );
    auto const b = s8r1::make< double >( 1, 2, { 3, 4 } );
    auto const want = s8r1::make< double >( 1, 3, { 3, 10, 8 } );
    CHECK( s8r1::equal( feng::conv( a, b ), want ) );
    CHECK( s8r1::equal( feng::conv( a, b, "full" ), want ) );
    CHECK( s8r1::equal( feng::conv2( a, b ), want ) );
    CHECK( s8r1::equal( feng::conv2( a, b, std::string{ "full" } ), want ) );
}

TEST_CASE( "S8-R1 a 1x1 kernel k gives k*A in every mode", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< double >();
    auto const k = s8r1::make< double >( 1, 1, { -3 } );
    feng::matrix< double > want = a;
    for ( auto& x : want ) x *= -3;
    CHECK( s8r1::equal( feng::conv( a, k ), want ) );
    CHECK( s8r1::equal( feng::conv( a, k, "full" ), want ) );
    CHECK( s8r1::equal( feng::conv( a, k, "same" ), want ) );
    CHECK( s8r1::equal( feng::conv( a, k, "valid" ), want ) );
    // the 1x1 operand first: full is commutative, same keeps the 1x1 shape, valid swaps
    CHECK( s8r1::equal( feng::conv( k, a ), want ) );
    CHECK( s8r1::equal( feng::conv( k, a, "same" ), s8r1::make< double >( 1, 1, { -15 } ) ) ); // full[1][1] = -3 * A[1][1]
    CHECK( s8r1::equal( feng::conv( k, a, "valid" ), want ) );
}

TEST_CASE( "S8-R1 asymmetric 3x4 with a 2x3 kernel matches scipy in every mode", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< double >();
    auto const b = s8r1::B< double >();
    CHECK( s8r1::equal( feng::conv( a, b ), s8r1::make< double >( 4, 6, { 1, -2, 2, 2, -3, 0, 6, 2, 2, -6, 10, -2, 8, 17, 17, 11, -3, 10, 0, 6, 7, 3, 2, -12 } ) ) );
    CHECK( s8r1::equal( feng::conv( a, b, "same" ), s8r1::make< double >( 3, 4, { -2, 2, 2, -3, 2, 2, -6, 10, 17, 17, 11, -3 } ) ) );
    CHECK( s8r1::equal( feng::conv( a, b, "valid" ), s8r1::make< double >( 2, 2, { 2, -6, 17, 11 } ) ) );
    CHECK( s8r1::equal( feng::conv2( a, b, "valid" ), s8r1::make< double >( 2, 2, { 2, -6, 17, 11 } ) ) );
}

TEST_CASE( "S8-R1 same mode with even and oversized kernels follows scipy's alignment", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< double >();
    auto const even = s8r1::make< double >( 2, 2, { 1, 2, 3, 4 } );
    CHECK( s8r1::equal( feng::conv( a, even, "same" ), s8r1::make< double >( 3, 4, { 1, 0, -1, 6, 7, 11, 10, 12, 12, 34, 25, 2 } ) ) );
    feng::matrix< double > over{ 4, 5 };
    for ( std::size_t i = 0; i != over.size(); ++i ) over.data()[i] = double( i + 1 );
    CHECK( s8r1::equal( feng::conv( a, over, "same" ), s8r1::make< double >( 3, 4, { 33, 45, 57, 34, 91, 114, 127, 80, 166, 179, 192, 120 } ) ) );
}

TEST_CASE( "S8-R1 valid mode swaps the operands when B contains A", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< double >();
    feng::matrix< double > over{ 4, 5 };
    for ( std::size_t i = 0; i != over.size(); ++i ) over.data()[i] = double( i + 1 );
    auto const want = s8r1::make< double >( 2, 2, { 114, 127, 179, 192 } );
    CHECK( s8r1::equal( feng::conv( a, over, "valid" ), want ) );
    CHECK( s8r1::equal( feng::conv( over, a, "valid" ), want ) );
}

TEST_CASE( "S8-R1 empty operands follow D-030", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< double >();
    feng::matrix< double > const e00{ 0, 0 };
    feng::matrix< double > const e03{ 0, 3 };
    feng::matrix< double > const e20{ 2, 0 };
    for ( auto const* e : { &e00, &e03, &e20 } )
    {
        CHECK( s8r1::zero_by_zero( feng::conv( a, *e ) ) );
        CHECK( s8r1::zero_by_zero( feng::conv( a, *e, "full" ) ) );
        CHECK( s8r1::zero_by_zero( feng::conv( a, *e, "valid" ) ) );
        CHECK( s8r1::equal( feng::conv( a, *e, "same" ), feng::matrix< double >{ 3, 4, 0.0 } ) );
        CHECK( s8r1::zero_by_zero( feng::conv( *e, a ) ) );
        CHECK( s8r1::zero_by_zero( feng::conv( *e, a, "valid" ) ) );
        CHECK( feng::conv( *e, a, "same" ).row() == e->row() );
        CHECK( feng::conv( *e, a, "same" ).col() == e->col() );
    }
}

TEST_CASE( "S8-R1 int elements are exact", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< int >();
    auto const b = s8r1::B< int >();
    CHECK( s8r1::equal( feng::conv( a, b ), s8r1::make< int >( 4, 6, { 1, -2, 2, 2, -3, 0, 6, 2, 2, -6, 10, -2, 8, 17, 17, 11, -3, 10, 0, 6, 7, 3, 2, -12 } ) ) );
    CHECK( s8r1::equal( feng::conv( a, b, "same" ), s8r1::make< int >( 3, 4, { -2, 2, 2, -3, 2, 2, -6, 10, 17, 17, 11, -3 } ) ) );
    CHECK( s8r1::equal( feng::conv( a, b, "valid" ), s8r1::make< int >( 2, 2, { 2, -6, 17, 11 } ) ) );
    // complex elements take the same path
    using cd = std::complex< double >;
    auto const ca = s8r1::make< cd >( 1, 2, { cd{ 1, 1 }, cd{ 2, 0 } } );
    auto const cb = s8r1::make< cd >( 1, 2, { cd{ 3, 0 }, cd{ 0, 4 } } );
    CHECK( s8r1::equal( feng::conv( ca, cb ), s8r1::make< cd >( 1, 3, { cd{ 3, 3 }, cd{ 2, 4 }, cd{ 0, 8 } } ) ) );
}

TEST_CASE( "S8-R1 impossible valid shapes and unknown modes abort", "[S8][S8-R1]" )
{
    auto const a = s8r1::A< double >();
    feng::matrix< double > const wide{ 2, 5, 1.0 };
    feng::matrix< double > const tall{ 4, 3, 1.0 };
    s8r1::require_one_message_death( [&] { auto r = feng::conv( a, wide, "valid" ); (void)r; }, "conv: 'valid' mode" );
    s8r1::require_one_message_death( [&] { auto r = feng::conv( a, tall, "valid" ); (void)r; }, "conv: 'valid' mode" );
    s8r1::require_one_message_death( [&] { auto r = feng::conv( a, a, "Full" ); (void)r; }, "conv: unknown mode 'Full'" );
    s8r1::require_one_message_death( [&] { auto r = feng::conv2( a, a, std::string{ "" } ); (void)r; }, "conv: unknown mode ''" );
}

namespace s8r1
{
    // the oracle: full[i][j] = sum A[p][q] * B[i-p][j-q], accumulated in long double (complex for complex T)
    template< typename T >
    std::vector< std::complex< long double > > direct_full( feng::matrix< T > const& a, feng::matrix< T > const& b )
    {
        std::size_t const R = a.row() + b.row() - 1, C = a.col() + b.col() - 1;
        std::vector< std::complex< long double > > out( R * C );
        for ( std::size_t p = 0; p != a.row(); ++p )
            for ( std::size_t q = 0; q != a.col(); ++q )
                for ( std::size_t u = 0; u != b.row(); ++u )
                    for ( std::size_t v = 0; v != b.col(); ++v )
                    {
                        std::complex< long double > const x{ static_cast< long double >( std::real( a[p][q] ) ), static_cast< long double >( std::imag( a[p][q] ) ) };
                        std::complex< long double > const y{ static_cast< long double >( std::real( b[u][v] ) ), static_cast< long double >( std::imag( b[u][v] ) ) };
                        out[( p + u ) * C + q + v] += x * y;
                    }
        return out;
    }

    // max |got - full[r0 + i][c0 + j]| over got's elements
    template< typename T >
    long double crop_err( feng::matrix< T > const& got, std::vector< std::complex< long double > > const& full, std::size_t C,
                          std::size_t r0, std::size_t c0 )
    {
        long double e = 0;
        for ( std::size_t i = 0; i != got.row(); ++i )
            for ( std::size_t j = 0; j != got.col(); ++j )
            {
                std::complex< long double > const g{ static_cast< long double >( std::real( got[i][j] ) ), static_cast< long double >( std::imag( got[i][j] ) ) };
                long double const d = std::abs( g - full[( r0 + i ) * C + c0 + j] );
                if ( !( d <= e ) ) e = d;
            }
        return e;
    }

    template< typename T >
    feng::matrix< T > sample( std::size_t r, std::size_t c, int seed )
    {
        feng::matrix< T > m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i )
        {
            int const u = static_cast< int >( ( i * 37 + static_cast< std::size_t >( seed ) * 11 ) % 19 ) - 9;
            int const w = static_cast< int >( ( i * 23 + static_cast< std::size_t >( seed ) * 7 ) % 17 ) - 8;
            if constexpr ( std::is_integral_v< T > ) m.data()[i] = static_cast< T >( u );
            else if constexpr ( std::is_floating_point_v< T > ) m.data()[i] = static_cast< T >( u ) / T( 7 );
            else m.data()[i] = T{ static_cast< double >( u ) / 7.0, static_cast< double >( w ) / 5.0 };
        }
        return m;
    }

    template< typename T >
    void check_direct( std::size_t ra, std::size_t ca, std::size_t rb, std::size_t cb )
    {
        INFO( "A " << ra << "x" << ca << ", B " << rb << "x" << cb );
        auto const a = sample< T >( ra, ca, 1 );
        auto const b = sample< T >( rb, cb, 2 );
        static_assert( std::is_same_v< decltype( feng::conv( a, b ) ), feng::matrix< T > > ); // D-030
        auto const full = direct_full( a, b );
        std::size_t const C = ca + cb - 1;
        long double sa = 0, sb = 0;
        for ( auto const& x : a ) sa += static_cast< long double >( std::abs( x ) );
        for ( auto const& x : b ) sb += static_cast< long double >( std::abs( x ) );
        long double tol = 0; // integers are exact
        if constexpr ( !std::is_integral_v< T > )
            tol = 4.0L * static_cast< long double >( std::numeric_limits< decltype( std::abs( T{} ) ) >::epsilon() ) * sa * sb;
        auto const f = feng::conv( a, b );
        REQUIRE( f.row() == ra + rb - 1 );
        REQUIRE( f.col() == C );
        CHECK( crop_err( f, full, C, 0, 0 ) <= tol );
        auto const s = feng::conv( a, b, "same" );
        REQUIRE( s.row() == ra );
        REQUIRE( s.col() == ca );
        CHECK( crop_err( s, full, C, ( rb - 1 ) / 2, ( cb - 1 ) / 2 ) <= tol );
        bool const a_holds = ra >= rb && ca >= cb, b_holds = rb >= ra && cb >= ca;
        if ( a_holds || b_holds )
        {
            auto const v = feng::conv( a, b, "valid" );
            std::size_t const vr = a_holds ? ra - rb + 1 : rb - ra + 1, vc = a_holds ? ca - cb + 1 : cb - ca + 1;
            REQUIRE( v.row() == vr );
            REQUIRE( v.col() == vc );
            CHECK( crop_err( v, full, C, a_holds ? rb - 1 : ra - 1, a_holds ? cb - 1 : ca - 1 ) <= tol );
        }
    }

    template< typename T >
    void check_direct_all()
    {
        for ( auto [rb, cb] : { std::pair{ 1, 1 }, std::pair{ 3, 4 }, std::pair{ 2, 2 }, std::pair{ 3, 9 }, std::pair{ 6, 8 } } )
            check_direct< T >( 5, 7, static_cast< std::size_t >( rb ), static_cast< std::size_t >( cb ) );
    }
}

TEST_CASE( "S8-R1 conv on float, int and complex<double> matches a direct sum", "[S8][S8-R1]" )
{
    SECTION( "float" ) { s8r1::check_direct_all< float >(); }
    SECTION( "int" ) { s8r1::check_direct_all< int >(); }
    SECTION( "complex<double>" ) { s8r1::check_direct_all< std::complex< double > >(); }
    SECTION( "double" ) { s8r1::check_direct_all< double >(); }
}
