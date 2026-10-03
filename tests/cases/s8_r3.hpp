// S8-R3 (PR-11, F14): fftshift and ifftshift are numpy's rolls by R/2, C/2 and -(R/2), -(C/2) on a matrix of x's
// own type and allocator; no transform is computed. Literal values below come from numpy 2.5.3 on arange(R*C).
#include <algorithm>
#include <complex>
#include <cstddef>
#include <type_traits>
#include <vector>

namespace s8r3
{
    template< typename T >
    feng::matrix< T > iota( std::size_t r, std::size_t c )
    {
        feng::matrix< T > m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i ) m.data()[i] = T( static_cast< int >( i ) );
        return m;
    }

    template< typename T >
    std::vector< T > values( feng::matrix< T > const& m )
    {
        return std::vector< T >( m.data(), m.data() + m.size() );
    }

    template< typename T >
    bool equal( feng::matrix< T > const& a, feng::matrix< T > const& b )
    {
        return a.row() == b.row() && a.col() == b.col() && values( a ) == values( b );
    }

    // the formula: out[(i + dr) mod R][(j + dc) mod C] = x[i][j]
    template< typename T >
    feng::matrix< T > rolled( feng::matrix< T > const& x, std::ptrdiff_t dr, std::ptrdiff_t dc )
    {
        std::ptrdiff_t const R = static_cast< std::ptrdiff_t >( x.row() );
        std::ptrdiff_t const C = static_cast< std::ptrdiff_t >( x.col() );
        feng::matrix< T > out{ x.row(), x.col() };
        for ( std::ptrdiff_t i = 0; i != R; ++i )
            for ( std::ptrdiff_t j = 0; j != C; ++j )
                out[( ( i + dr ) % R + R ) % R][( ( j + dc ) % C + C ) % C] = x[i][j];
        return out;
    }

    template< typename T >
    void check_size( std::size_t r, std::size_t c )
    {
        INFO( r << "x" << c );
        auto const x = iota< T >( r, c );
        auto const s = feng::fftshift( x );
        auto const is = feng::ifftshift( x );
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( s ) >, feng::matrix< T > > );
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( is ) >, feng::matrix< T > > );
        std::ptrdiff_t const hr = static_cast< std::ptrdiff_t >( r / 2 );
        std::ptrdiff_t const hc = static_cast< std::ptrdiff_t >( c / 2 );
        CHECK( equal( s, rolled( x, hr, hc ) ) );
        CHECK( equal( is, rolled( x, -hr, -hc ) ) );
        CHECK( equal( feng::ifftshift( s ), x ) );
        CHECK( equal( feng::fftshift( is ), x ) );
        auto sorted = []( feng::matrix< T > const& m )
        {
            auto v = values( m );
            std::sort( v.begin(), v.end(), []( T const& a, T const& b ) { return std::real( a ) < std::real( b ); } );
            return v;
        };
        CHECK( sorted( s ) == sorted( x ) );
        CHECK( sorted( is ) == sorted( x ) );
    }

    template< typename T >
    feng::matrix< T > make( std::size_t r, std::size_t c, std::vector< int > const& v )
    {
        feng::matrix< T > m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i ) m.data()[i] = T( v[i] );
        return m;
    }

    template< typename T >
    void check_literals()
    {
        // numpy.fft.fftshift / ifftshift of numpy.arange(R*C).reshape(R, C)
        CHECK( equal( feng::fftshift( iota< T >( 3, 5 ) ),
                      make< T >( 3, 5, { 13, 14, 10, 11, 12, 3, 4, 0, 1, 2, 8, 9, 5, 6, 7 } ) ) );
        CHECK( equal( feng::ifftshift( iota< T >( 3, 5 ) ),
                      make< T >( 3, 5, { 7, 8, 9, 5, 6, 12, 13, 14, 10, 11, 2, 3, 4, 0, 1 } ) ) );
        std::vector< int > const s46{ 15, 16, 17, 12, 13, 14, 21, 22, 23, 18, 19, 20,
                                      3,  4,  5,  0,  1,  2,  9,  10, 11, 6,  7,  8 };
        CHECK( equal( feng::fftshift( iota< T >( 4, 6 ) ), make< T >( 4, 6, s46 ) ) );
        CHECK( equal( feng::ifftshift( iota< T >( 4, 6 ) ), make< T >( 4, 6, s46 ) ) );
        CHECK( equal( feng::fftshift( iota< T >( 5, 5 ) ),
                      make< T >( 5, 5, { 18, 19, 15, 16, 17, 23, 24, 20, 21, 22, 3, 4, 0, 1, 2,
                                         8,  9,  5,  6,  7,  13, 14, 10, 11, 12 } ) ) );
        CHECK( equal( feng::ifftshift( iota< T >( 5, 5 ) ),
                      make< T >( 5, 5, { 12, 13, 14, 10, 11, 17, 18, 19, 15, 16, 22, 23, 24, 20, 21,
                                         2,  3,  4,  0,  1,  7,  8,  9,  5,  6 } ) ) );
        CHECK( equal( feng::fftshift( iota< T >( 1, 4 ) ), make< T >( 1, 4, { 2, 3, 0, 1 } ) ) );
        CHECK( equal( feng::ifftshift( iota< T >( 1, 4 ) ), make< T >( 1, 4, { 2, 3, 0, 1 } ) ) );
        CHECK( equal( feng::fftshift( iota< T >( 6, 1 ) ), make< T >( 6, 1, { 3, 4, 5, 0, 1, 2 } ) ) );
        CHECK( equal( feng::ifftshift( iota< T >( 6, 1 ) ), make< T >( 6, 1, { 3, 4, 5, 0, 1, 2 } ) ) );
        CHECK( equal( feng::fftshift( iota< T >( 1, 1 ) ), iota< T >( 1, 1 ) ) );
        CHECK( equal( feng::ifftshift( iota< T >( 1, 1 ) ), iota< T >( 1, 1 ) ) );
    }

    template< typename T >
    void check_all()
    {
        check_literals< T >();
        for ( auto [r, c] : { std::pair{ 1, 1 }, std::pair{ 1, 4 }, std::pair{ 3, 5 }, std::pair{ 4, 6 },
                              std::pair{ 5, 5 }, std::pair{ 6, 1 } } )
            check_size< T >( static_cast< std::size_t >( r ), static_cast< std::size_t >( c ) );
        for ( auto [r, c] : { std::pair{ 0, 0 }, std::pair{ 0, 3 }, std::pair{ 4, 0 } } )
        {
            feng::matrix< T > const e{ static_cast< std::size_t >( r ), static_cast< std::size_t >( c ) };
            auto const s = feng::fftshift( e );
            auto const is = feng::ifftshift( e );
            CHECK( s.row() == e.row() );
            CHECK( s.col() == e.col() );
            CHECK( is.row() == e.row() );
            CHECK( is.col() == e.col() );
            CHECK( s.size() == 0 );
            CHECK( is.size() == 0 );
        }
    }
}

TEST_CASE( "S8-R3 shifts are numpy rolls and invert each other exactly", "[S8][S8-R3]" )
{
    SECTION( "double" ) { s8r3::check_all< double >(); }
    SECTION( "int" ) { s8r3::check_all< int >(); }
    SECTION( "complex<double>" ) { s8r3::check_all< std::complex< double > >(); }
}
