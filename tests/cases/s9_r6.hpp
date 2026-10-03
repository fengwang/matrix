// S9-R6 (PR-12): the one elementwise transform helper. matrix_details::transform( m, f ) builds a matrix of m's
// shape whose element type is f's result (or the R of transform< R >) and whose allocator is m's rebound to it;
// the unary family goes through it and keeps its pre-S9 result types and values.
#include "./s3_alloc.hpp"

#include <cmath>
#include <complex>
#include <cstddef>
#include <type_traits>

namespace s9_r6
{
    using alloc = s3_alloc::tracking_allocator< double, true, true, true >;
    using amat = feng::matrix< double, alloc >;
    template < typename T >
    using rebound = feng::matrix< T, s3_alloc::tracking_allocator< T, true, true, true > >;

    inline amat sample( int id, std::size_t r, std::size_t c )
    {
        amat m{ alloc{ id }, r, c };
        for ( std::size_t i = 0; i != m.size(); ++i )
            m.data()[i] = 0.25 + 0.5 * static_cast< double >( i % 97 ) - 3.0 * static_cast< double >( i % 5 );
        return m;
    }
}

TEST_CASE( "S9-R6 transform keeps shape, allocator and result type", "[S9][S9-R6]" )
{
    using s9_r6::amat;
    using s9_r6::rebound;
    s3_alloc::reset();
    {
        auto const m = s9_r6::sample( 7, 3, 5 );

        SECTION( "the helper: result type from the callable or given" )
        {
            auto const t = feng::matrix_details::transform( m, []( double x ) { return static_cast< int >( x * 2.0 ); } );
            static_assert( std::is_same_v< std::remove_const_t< decltype( t ) >, rebound< int > > );
            REQUIRE( t.row() == 3 );
            REQUIRE( t.col() == 5 );
            REQUIRE( t.get_allocator().id == 7 );
            for ( std::size_t i = 0; i != m.size(); ++i )
                REQUIRE( t.data()[i] == static_cast< int >( m.data()[i] * 2.0 ) );
            auto const f = feng::matrix_details::transform< float >( m, []( double x ) { return x + 1.0; } );
            static_assert( std::is_same_v< std::remove_const_t< decltype( f ) >, rebound< float > > );
            REQUIRE( f.get_allocator().id == 7 );
            REQUIRE( f.data()[4] == static_cast< float >( m.data()[4] + 1.0 ) );
            feng::matrix< double > const empty;
            REQUIRE( feng::matrix_details::transform( empty, []( double x ) { return x; } ).size() == 0 );
        }

        SECTION( "the unary family keeps its result types and the allocator" )
        {
            static_assert( std::is_same_v< decltype( feng::abs( m ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::sqrt( m ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::sqrt( feng::matrix< int >{} ) ), feng::matrix< int > > );
            static_assert( std::is_same_v< decltype( feng::ilogb( m ) ), rebound< int > > );
            static_assert( std::is_same_v< decltype( feng::lrint( m ) ), rebound< long > > );
            static_assert( std::is_same_v< decltype( feng::llrint( m ) ), rebound< long long > > );
            static_assert( std::is_same_v< decltype( feng::lround( m ) ), rebound< long > > );
            static_assert( std::is_same_v< decltype( feng::llround( m ) ), rebound< long long > > );
            auto const a = feng::abs( m );
            auto const l = feng::lround( m );
            auto const g = feng::ilogb( m );
            REQUIRE( a.row() == 3 );
            REQUIRE( a.col() == 5 );
            REQUIRE( a.get_allocator().id == 7 );
            REQUIRE( l.get_allocator().id == 7 );
            REQUIRE( g.get_allocator().id == 7 );
            for ( std::size_t i = 0; i != m.size(); ++i )
            {
                REQUIRE( a.data()[i] == std::abs( m.data()[i] ) );
                REQUIRE( l.data()[i] == std::lround( m.data()[i] ) );
                REQUIRE( g.data()[i] == std::ilogb( m.data()[i] ) );
            }
            feng::matrix< int > const im{ 1, 3, { 2, 9, 10 } };
            auto const is = feng::sqrt( im );
            REQUIRE( is[0][0] == 1 );
            REQUIRE( is[0][1] == 3 );
            REQUIRE( is[0][2] == 3 );
        }

        SECTION( "a parallel-size input" )
        {
            auto const big = s9_r6::sample( 9, 64, 80 ); // 5120 elements, above the parallel threshold
            auto const e = feng::exp( big );
            auto const r = feng::llrint( big );
            REQUIRE( e.row() == 64 );
            REQUIRE( e.col() == 80 );
            REQUIRE( e.get_allocator().id == 9 );
            REQUIRE( r.get_allocator().id == 9 );
            for ( std::size_t i = 0; i != big.size(); ++i )
            {
                REQUIRE( e.data()[i] == std::exp( big.data()[i] ) );
                REQUIRE( r.data()[i] == std::llrint( big.data()[i] ) );
            }
        }
    }
    s3_alloc::require_balanced();
}

// S9-T11: the binary family (matrix-matrix, matrix-scalar, scalar-matrix) and the complex family go through the same
// helper; results keep m's type (binary, conj, proj) or the complex value type (real, imag, abs, arg, norm), and the
// allocator of the matrix operand.
TEST_CASE( "S9-R6 binary and complex families keep their values and types", "[S9][S9-R6]" )
{
    using s9_r6::amat;
    using s9_r6::rebound;
    using C = std::complex< double >;
    using cmat = rebound< C >;
    s3_alloc::reset();
    {
        auto const m = s9_r6::sample( 7, 3, 5 );
        auto n = s9_r6::sample( 8, 3, 5 );
        for ( std::size_t i = 0; i != n.size(); ++i )
            n.data()[i] = 1.5 - n.data()[i];

        SECTION( "binary: matrix-matrix, matrix-scalar and scalar-matrix" )
        {
            static_assert( std::is_same_v< decltype( feng::hypot( m, n ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::hypot( m, 2.0 ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::hypot( 2.0, m ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::pow( m, 2 ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::fma( m, n, m ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::ldexp( feng::matrix< int >{}, 2.0 ) ), feng::matrix< int > > );
            auto const mm = feng::atan2( m, n );
            auto const ms = feng::atan2( m, 0.75 );
            auto const sm = feng::atan2( 0.75, m );
            auto const pi = feng::pow( m, 3 );
            auto const f3 = feng::fma( m, n, m );
            REQUIRE( mm.row() == 3 );
            REQUIRE( mm.col() == 5 );
            REQUIRE( mm.get_allocator().id == 7 );
            REQUIRE( ms.get_allocator().id == 7 );
            REQUIRE( sm.get_allocator().id == 7 );
            REQUIRE( f3.get_allocator().id == 7 );
            for ( std::size_t i = 0; i != m.size(); ++i )
            {
                REQUIRE( mm.data()[i] == std::atan2( m.data()[i], n.data()[i] ) );
                REQUIRE( ms.data()[i] == std::atan2( m.data()[i], 0.75 ) );
                REQUIRE( sm.data()[i] == std::atan2( 0.75, m.data()[i] ) );
                REQUIRE( pi.data()[i] == std::pow( m.data()[i], 3 ) );
                REQUIRE( f3.data()[i] == std::fma( m.data()[i], n.data()[i], m.data()[i] ) );
            }
        }

        SECTION( "complex: real, imag, abs, arg, norm, conj, proj" )
        {
            cmat c{ s3_alloc::tracking_allocator< C, true, true, true >{ 5 }, 3, 5 };
            for ( std::size_t i = 0; i != c.size(); ++i )
                c.data()[i] = C{ m.data()[i], n.data()[i] };
            static_assert( std::is_same_v< decltype( feng::real( c ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::imag( c ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::abs( c ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::arg( c ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::norm( c ) ), amat > );
            static_assert( std::is_same_v< decltype( feng::conj( c ) ), cmat > );
            static_assert( std::is_same_v< decltype( feng::proj( c ) ), cmat > );
            static_assert( std::is_same_v< decltype( feng::real( feng::matrix< std::complex< float > >{} ) ), feng::matrix< float > > );
            auto const re = feng::real( c );
            auto const im = feng::imag( c );
            auto const ar = feng::arg( c );
            auto const no = feng::norm( c );
            auto const cj = feng::conj( c );
            REQUIRE( re.row() == 3 );
            REQUIRE( re.col() == 5 );
            REQUIRE( re.get_allocator().id == 5 );
            REQUIRE( no.get_allocator().id == 5 );
            REQUIRE( cj.get_allocator().id == 5 );
            for ( std::size_t i = 0; i != c.size(); ++i )
            {
                REQUIRE( re.data()[i] == std::real( c.data()[i] ) );
                REQUIRE( im.data()[i] == std::imag( c.data()[i] ) );
                REQUIRE( ar.data()[i] == std::arg( c.data()[i] ) );
                REQUIRE( no.data()[i] == std::norm( c.data()[i] ) );
                REQUIRE( cj.data()[i] == std::conj( c.data()[i] ) );
            }
        }
    }
    s3_alloc::require_balanced();
}
