// S2-R4 (PR-3): operand shapes are checked before traversal; pooling modes; self-assignment and overlap policy.
#include "./s2_death.hpp"

#include <valarray>
#include <vector>

TEST_CASE( "S2 valarray length 3 times 2x3 aborts", "[S2][S2-R4]" )
{
    SECTION( "length 2 gives the explicit sums" )
    {
        feng::matrix<double> m{ 2, 3 };
        for ( std::size_t r = 0; r != 2; ++r )
            for ( std::size_t c = 0; c != 3; ++c )
                m[r][c] = static_cast<double>( 10 * r + c ) + 0.5;
        std::valarray<double> const v{ 2.0, -3.0 };
        auto const p = v * m;
        REQUIRE( p.row() == 1 );
        REQUIRE( p.col() == 3 );
        for ( std::size_t c = 0; c != 3; ++c )
            REQUIRE( p[0][c] == 2.0 * m[0][c] - 3.0 * m[1][c] );
    }
    SECTION( "length 3 aborts with a shape message" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 2, 3 };
            std::valarray<double> const v{ 1.0, 2.0, 3.0 };
            auto const p = v * m;
            (void)p;
        }, "shape" );
    }
}

TEST_CASE( "S2 left vector product checks rows", "[S2][S2-R4]" )
{
    SECTION( "length 2 times 2x3 gives the explicit sums" )
    {
        feng::matrix<double> m{ 2, 3 };
        for ( std::size_t r = 0; r != 2; ++r )
            for ( std::size_t c = 0; c != 3; ++c )
                m[r][c] = static_cast<double>( 10 * r + c ) + 0.5;
        std::vector<double> const v{ 2.0, -3.0 };
        auto const p = v * m;
        REQUIRE( p.row() == 1 );
        REQUIRE( p.col() == 3 );
        for ( std::size_t c = 0; c != 3; ++c )
            REQUIRE( p[0][c] == 2.0 * m[0][c] - 3.0 * m[1][c] );
    }
    SECTION( "length 3 times 2x3 aborts with a shape message" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 2, 3 };
            std::vector<double> const v{ 1.0, 2.0, 3.0 };
            auto const p = v * m;
            (void)p;
        }, "shape" );
    }
    SECTION( "2x3 times a vector of length 2 aborts with a shape message" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 2, 3 };
            std::vector<double> const v{ 1.0, 2.0 };
            auto const p = m * v;
            (void)p;
        }, "shape" );
    }
    SECTION( "2x3 times a valarray of length 2 aborts with a shape message" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 2, 3 };
            std::valarray<double> const v{ 1.0, 2.0 };
            auto const p = m * v;
            (void)p;
        }, "shape" );
    }
}

TEST_CASE( "S2 binary and ternary element-wise maps check shapes", "[S2][S2-R4]" )
{
    SECTION( "binary map_impl" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> a{ 2, 3 };
            feng::matrix<double> b{ 3, 2 };
            auto const p = feng::matrix_details::map( []( double x, double y ){ return x + y; } )( a, b );
            (void)p;
        }, "shape" );
    }
    SECTION( "ternary map_impl" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> a{ 2, 3 };
            feng::matrix<double> b{ 2, 3 };
            feng::matrix<double> c{ 2, 2 };
            auto const p = feng::matrix_details::map( []( double x, double y, double z ){ return x + y + z; } )( a, b, c );
            (void)p;
        }, "shape" );
    }
    SECTION( "hypot" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> a{ 2, 3 };
            feng::matrix<double> b{ 3, 2 };
            auto const p = feng::hypot( a, b );
            (void)p;
        }, "shape" );
    }
    SECTION( "fmin" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> a{ 2, 3 };
            feng::matrix<double> b{ 2, 4 };
            auto const p = feng::fmin( a, b );
            (void)p;
        }, "shape" );
    }
    SECTION( "fma" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> a{ 2, 3 };
            feng::matrix<double> b{ 2, 3 };
            feng::matrix<double> c{ 3, 3 };
            auto const p = feng::fma( a, b, c );
            (void)p;
        }, "shape" );
    }
    SECTION( "matching shapes still work" )
    {
        feng::matrix<double> a{ 2, 3 };
        feng::matrix<double> b{ 2, 3 };
        for ( std::size_t i = 0; i != 6; ++i ) { a.data()[i] = 3.0; b.data()[i] = 4.0; }
        auto const h = feng::hypot( a, b );
        auto const s = feng::matrix_details::map( []( double x, double y, double z ){ return x + y + z; } )( a, b, a );
        for ( std::size_t i = 0; i != 6; ++i )
        {
            REQUIRE( h.data()[i] == 5.0 );
            REQUIRE( s.data()[i] == 10.0 );
        }
    }
}

TEST_CASE( "S2 matrix operators check shapes", "[S2][S2-R4]" )
{
    SECTION( "+=" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> a{ 2, 3 }; feng::matrix<double> const b{ 3, 2 }; a += b; }, "shape" );
    }
    SECTION( "-=" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> a{ 2, 3 }; feng::matrix<double> const b{ 2, 2 }; a -= b; }, "shape" );
    }
    SECTION( "/=" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> a{ 2, 2 }; feng::matrix<double> const b{ 3, 3 }; a /= b; }, "shape" );
    }
    SECTION( "+" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> const a{ 2, 3 }; feng::matrix<double> const b{ 3, 2 }; auto const p = a + b; (void)p; }, "shape" );
    }
    SECTION( "-" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> const a{ 2, 3 }; feng::matrix<double> const b{ 2, 4 }; auto const p = a - b; (void)p; }, "shape" );
    }
    SECTION( "/" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> const a{ 2, 2 }; feng::matrix<double> const b{ 3, 3 }; auto const p = a / b; (void)p; }, "shape" );
    }
    SECTION( "*= keeps its dims check with a shape message" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> a{ 2, 3 }; feng::matrix<double> const b{ 2, 3 }; a *= b; }, "shape" );
    }
}

TEST_CASE( "S2 pooling rejects an unknown action", "[S2][S2-R4]" )
{
    SECTION( "dim 2" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 4, 4 };
            auto const p = feng::pooling( m, 2, "median" );
            (void)p;
        }, "pooling" );
    }
    SECTION( "dim 1, before the identity early return" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 4, 4 };
            auto const p = feng::pooling( m, 1, "median" );
            (void)p;
        }, "pooling" );
    }
    SECTION( "dim 0, before the empty early return" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 4, 4 };
            auto const p = feng::pooling( m, 0, "median" );
            (void)p;
        }, "pooling" );
    }
    SECTION( "dims (1, 1), two-dim form" )
    {
        S2_REQUIRE_DEATH( []{
            feng::matrix<double> m{ 4, 4 };
            auto const p = feng::pooling( m, 1, 1, "median" );
            (void)p;
        }, "pooling" );
    }
}

namespace s2_r4_private
{
    inline feng::matrix<double> filled( std::size_t r, std::size_t c )
    {
        feng::matrix<double> m{ r, c };
        for ( std::size_t i = 0; i != r; ++i )
            for ( std::size_t j = 0; j != c; ++j )
                m[i][j] = static_cast<double>( ( 7 * i + 3 * j ) % 11 ) - 5.0;
        return m;
    }

    // A plain copy of the block [r0, r1) x [c0, c1) of m, made element by element.
    inline feng::matrix<double> block( feng::matrix<double> const& m, std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1 )
    {
        feng::matrix<double> b{ r1 - r0, c1 - c0 };
        for ( std::size_t i = r0; i != r1; ++i )
            for ( std::size_t j = c0; j != c1; ++j )
                b[i - r0][j - c0] = m[i][j];
        return b;
    }

    inline bool same( feng::matrix<double> const& a, feng::matrix<double> const& b )
    {
        if ( a.row() != b.row() || a.col() != b.col() ) return false;
        for ( std::size_t i = 0; i != a.row(); ++i )
            for ( std::size_t j = 0; j != a.col(); ++j )
                if ( a[i][j] != b[i][j] ) return false;
        return true;
    }
}

TEST_CASE( "S2 self-assignment keeps the contents", "[S2][S2-R4]" )
{
    auto a = s2_r4_private::filled( 5, 7 );
    auto const before = s2_r4_private::filled( 5, 7 );
    double const* const storage = a.data();

    auto& self = a;
    a = self;
    REQUIRE( s2_r4_private::same( a, before ) );
    REQUIRE( a.data() == storage );

    a.operator=< double, std::allocator<double> >( self );
    REQUIRE( s2_r4_private::same( a, before ) );
    REQUIRE( a.data() == storage );

    a.copy( self );
    REQUIRE( s2_r4_private::same( a, before ) );
    REQUIRE( a.data() == storage );
}

TEST_CASE( "S2 self multiply uses the old matrix", "[S2][S2-R4]" )
{
    for ( std::size_t n : { std::size_t{ 3 }, std::size_t{ 18 }, std::size_t{ 32 } } )
    {
        INFO( "n = " << n );
        auto a = s2_r4_private::filled( n, n );
        auto const old = s2_r4_private::filled( n, n );
        feng::matrix<double> expected{ n, n };
        for ( std::size_t i = 0; i != n; ++i )
            for ( std::size_t j = 0; j != n; ++j )
            {
                double s = 0.0;
                for ( std::size_t k = 0; k != n; ++k ) s += old[i][k] * old[k][j];
                expected[i][j] = s;
            }
        a *= a;
        REQUIRE( s2_r4_private::same( a, expected ) );
    }
}

TEST_CASE( "S2 overlapping slice copy uses a snapshot", "[S2][S2-R4]" )
{
    SECTION( "a view of the destination copied into an overlapping block" )
    {
        auto m = s2_r4_private::filled( 5, 6 );
        auto const snapshot = s2_r4_private::block( m, 0, 3, 0, 4 );
        auto expected = s2_r4_private::filled( 5, 6 );
        expected.copy( snapshot, { 1, 4 }, { 1, 5 } );

        m.copy( feng::make_view( m, { 0, 3 }, { 0, 4 } ), { 1, 4 }, { 1, 5 } );
        REQUIRE( s2_r4_private::same( m, expected ) );
    }
    SECTION( "a view of the destination copied backwards into an overlapping block" )
    {
        auto m = s2_r4_private::filled( 5, 6 );
        auto const snapshot = s2_r4_private::block( m, 1, 4, 2, 6 );
        auto expected = s2_r4_private::filled( 5, 6 );
        expected.copy( snapshot, { 0, 3 }, { 0, 4 } );

        m.copy( feng::make_view( m, { 1, 4 }, { 2, 6 } ), { 0, 3 }, { 0, 4 } );
        REQUIRE( s2_r4_private::same( m, expected ) );
    }
    SECTION( "the whole matrix copied onto itself" )
    {
        auto m = s2_r4_private::filled( 4, 4 );
        auto const before = s2_r4_private::filled( 4, 4 );
        m.copy( m, { 0, 4 }, { 0, 4 } );
        REQUIRE( s2_r4_private::same( m, before ) );
    }
    SECTION( "a disjoint matrix copies as before" )
    {
        auto m = s2_r4_private::filled( 5, 6 );
        auto const src = s2_r4_private::filled( 2, 2 );
        m.copy( src, { 3, 5 }, { 4, 6 } );
        for ( std::size_t i = 0; i != 2; ++i )
            for ( std::size_t j = 0; j != 2; ++j )
                REQUIRE( m[3 + i][4 + j] == src[i][j] );
    }
}
