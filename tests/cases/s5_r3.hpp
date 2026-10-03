// S5-R3 (PR-7, F08): the BMP loader validates the headers with checked arithmetic; S5-R4 (PR-2, D-011): it fails
// with an empty optional and one stderr line instead of aborting. BMP bytes are built here; temporary files go under
// std::filesystem::temp_directory_path().
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <unistd.h> // getpid: per-process temp names

namespace s5_r3
{
    // The process id keeps two suite binaries run by hand at once from sharing a temp path.
    inline std::string temp_path( std::string const& name )
    {
        return ( std::filesystem::temp_directory_path() / ( "feng_s5_r3_" + std::to_string( ::getpid() ) + "_" + name ) ).string();
    }

    inline std::string temp_file( std::string const& name, std::vector<std::uint8_t> const& bytes )
    {
        auto const path = temp_path( name );
        std::ofstream ofs( path, std::ios::binary | std::ios::trunc );
        ofs.write( reinterpret_cast<char const*>( bytes.data() ), static_cast<std::streamsize>( bytes.size() ) );
        ofs.close();
        return path;
    }

    inline void put16( std::vector<std::uint8_t>& b, std::size_t at, std::uint32_t v )
    {
        b[at] = static_cast<std::uint8_t>( v & 0xff );
        b[at + 1] = static_cast<std::uint8_t>( ( v >> 8 ) & 0xff );
    }

    inline void put32( std::vector<std::uint8_t>& b, std::size_t at, std::uint32_t v )
    {
        put16( b, at, v & 0xffff );
        put16( b, at + 2, v >> 16 );
    }

    struct header
    {
        std::int32_t width = 3;
        std::int32_t height = 2;
        std::uint32_t info_size = 40;
        std::uint32_t bpp = 24;
        std::uint32_t compression = 0;
        std::uint32_t planes = 1;
        std::uint32_t extra_offset = 0; // gap between the info header and the pixels
        std::size_t trailing = 0;
    };

    // The colour of logical (top-down) pixel (r, c): {red, green, blue}.
    inline std::array<std::uint8_t, 3> colour( std::size_t r, std::size_t c )
    {
        return { static_cast<std::uint8_t>( 10 * r + c ), static_cast<std::uint8_t>( 100 + 10 * r + c ), static_cast<std::uint8_t>( 200 + 10 * r + c ) };
    }

    // A BMP whose logical image is colour(r, c); a positive height stores rows bottom-up, a negative one top-down.
    inline std::vector<std::uint8_t> bmp( header const& h )
    {
        std::uint32_t const offset = 14 + h.info_size + h.extra_offset;
        std::size_t const rows = static_cast<std::size_t>( h.height < 0 ? -static_cast<std::int64_t>( h.height ) : h.height );
        std::size_t const cols = static_cast<std::size_t>( h.width );
        std::size_t const bytes_per_pixel = h.bpp / 8;
        std::size_t const stride = ( cols * h.bpp + 31 ) / 32 * 4;
        std::vector<std::uint8_t> b( offset + stride * rows + h.trailing, 0xEE );
        std::fill( b.begin(), b.begin() + offset, std::uint8_t{ 0 } );
        b[0] = 'B'; b[1] = 'M';
        put32( b, 2, static_cast<std::uint32_t>( b.size() ) );
        put32( b, 10, offset );
        put32( b, 14, h.info_size );
        put32( b, 18, static_cast<std::uint32_t>( h.width ) );
        put32( b, 22, static_cast<std::uint32_t>( h.height ) );
        put16( b, 26, h.planes );
        put16( b, 28, h.bpp );
        put32( b, 30, h.compression );
        for ( std::size_t k = 0; bytes_per_pixel >= 3 && k != rows; ++k )
        {
            std::size_t const r = h.height > 0 ? rows - 1 - k : k; // the logical row stored k-th
            for ( std::size_t c = 0; c != cols; ++c )
            {
                auto const [red, green, blue] = colour( r, c );
                std::size_t const at = offset + k * stride + c * bytes_per_pixel;
                b[at] = blue; b[at + 1] = green; b[at + 2] = red;
                if ( bytes_per_pixel == 4 ) b[at + 3] = 0x7F;
            }
        }
        return b;
    }

    inline bool matches( std::optional<std::array<feng::matrix<std::uint8_t>, 3>> const& img, std::size_t rows, std::size_t cols )
    {
        if ( !img ) return false;
        for ( auto const& m : *img )
            if ( m.row() != rows || m.col() != cols ) return false;
        for ( std::size_t r = 0; r != rows; ++r )
            for ( std::size_t c = 0; c != cols; ++c )
            {
                auto const [red, green, blue] = colour( r, c );
                if ( ( *img )[0][r][c] != red || ( *img )[1][r][c] != green || ( *img )[2][r][c] != blue ) return false;
            }
        return true;
    }

    inline bool rejected( std::string const& name, std::vector<std::uint8_t> const& bytes )
    {
        std::ostringstream captured;
        auto* const old = std::cerr.rdbuf( captured.rdbuf() );
        auto const img = feng::load_bmp( temp_file( name, bytes ) );
        std::cerr.rdbuf( old );
        std::string const text = captured.str();
        return !img && text.find( "load_bmp" ) != std::string::npos && std::count( text.begin(), text.end(), '\n' ) == 1;
    }
}

TEST_CASE( "S5 load_bmp rejects width 2147483647 and height -2147483648", "[S5][S5-R3][S5-R4]" )
{
    auto b = s5_r3::bmp( {} );
    s5_r3::put32( b, 18, 2147483647u );
    s5_r3::put32( b, 22, 0x80000000u );
    REQUIRE( s5_r3::rejected( "huge.bmp", b ) );
    s5_r3::put32( b, 22, 0x80000001u ); // -2147483647: the pixel array would not fit in the file
    REQUIRE( s5_r3::rejected( "huge2.bmp", b ) );
    s5_r3::put32( b, 22, 2147483647u );
    REQUIRE( s5_r3::rejected( "huge3.bmp", b ) );
}

TEST_CASE( "S5 bottom-up and top-down files load to the same channels", "[S5][S5-R3]" )
{
    // 24- and 32-bit (alpha ignored), bottom-up and top-down, widths whose rows need 0 to 3 padding bytes
    for ( std::uint32_t bpp : { 24u, 32u } )
        for ( std::int32_t width : { 1, 2, 3, 4, 5 } )
            for ( std::int32_t height : { 3, -3, 1, -1 } )
            {
                INFO( "bpp " << bpp << ", width " << width << ", height " << height );
                s5_r3::header h;
                h.bpp = bpp; h.width = width; h.height = height;
                auto const rows = static_cast<std::size_t>( height < 0 ? -height : height );
                REQUIRE( s5_r3::matches( feng::load_bmp( s5_r3::temp_file( "orient.bmp", s5_r3::bmp( h ) ) ), rows, static_cast<std::size_t>( width ) ) );
            }

    // every supported info header size, alone, with a gap before the pixels and with trailing bytes
    for ( std::uint32_t info : { 40u, 52u, 56u, 108u, 124u } )
        for ( std::uint32_t bpp : { 24u, 32u } )
            for ( std::uint32_t gap : { 0u, 6u } )
                for ( std::size_t trailing : { std::size_t{ 0 }, std::size_t{ 3 } } )
                {
                    INFO( "info " << info << ", bpp " << bpp << ", gap " << gap << ", trailing " << trailing );
                    s5_r3::header g;
                    g.width = 5; g.height = info % 2 ? 3 : -3; g.info_size = info; g.bpp = bpp;
                    g.extra_offset = gap; g.trailing = trailing;
                    REQUIRE( s5_r3::matches( feng::load_bmp( s5_r3::temp_file( "var.bmp", s5_r3::bmp( g ) ) ), 3, 5 ) );
                }
}

TEST_CASE( "S5 load_bmp rejects truncations and unsupported headers", "[S5][S5-R3][S5-R4]" )
{
    auto const good = s5_r3::bmp( {} );
    s5_r3::header h32; h32.bpp = 32; h32.info_size = 124;
    for ( auto const& valid : { good, s5_r3::bmp( h32 ) } )
        for ( std::size_t n = 0; n != valid.size(); ++n )
        {
            INFO( "truncated to " << n << " of " << valid.size() << " bytes" );
            REQUIRE( s5_r3::rejected( "trunc.bmp", std::vector<std::uint8_t>( valid.begin(), valid.begin() + static_cast<std::ptrdiff_t>( n ) ) ) );
        }

    auto sig = good; sig[1] = 'A';
    REQUIRE( s5_r3::rejected( "sig.bmp", sig ) );
    for ( std::uint32_t info : { 0u, 12u, 39u, 41u, 64u, 125u, 100000u } )
    {
        auto b = good; s5_r3::put32( b, 14, info );
        REQUIRE( s5_r3::rejected( "info.bmp", b ) );
    }
    for ( std::uint32_t comp : { 1u, 2u, 3u, 6u } )
    {
        s5_r3::header h; h.compression = comp;
        REQUIRE( s5_r3::rejected( "comp.bmp", s5_r3::bmp( h ) ) );
    }
    for ( std::uint32_t bpp : { 1u, 4u, 8u, 16u } )
    {
        s5_r3::header h; h.bpp = bpp;
        auto b = s5_r3::bmp( h );
        b.resize( b.size() + 64, 0 );
        REQUIRE( s5_r3::rejected( "bpp.bmp", b ) );
    }
    { s5_r3::header h; h.planes = 2; REQUIRE( s5_r3::rejected( "planes.bmp", s5_r3::bmp( h ) ) ); }
    { auto b = good; s5_r3::put32( b, 18, 0 ); REQUIRE( s5_r3::rejected( "w0.bmp", b ) ); }
    { auto b = good; s5_r3::put32( b, 18, 0xFFFFFFFFu ); REQUIRE( s5_r3::rejected( "wneg.bmp", b ) ); }
    { auto b = good; s5_r3::put32( b, 18, 0x80000000u ); REQUIRE( s5_r3::rejected( "wmin.bmp", b ) ); }
    { auto b = good; s5_r3::put32( b, 22, 0 ); REQUIRE( s5_r3::rejected( "h0.bmp", b ) ); }
    { auto b = good; s5_r3::put32( b, 22, 0x80000000u ); REQUIRE( s5_r3::rejected( "hmin.bmp", b ) ); }
    // the pixel-data offset one byte before the end of each info header
    for ( std::uint32_t info : { 40u, 124u } )
    {
        s5_r3::header h; h.info_size = info;
        auto b = s5_r3::bmp( h );
        s5_r3::put32( b, 10, 14 + info - 1 );
        REQUIRE( s5_r3::rejected( "offset_in_info.bmp", b ) );
    }
    { auto b = good; s5_r3::put32( b, 10, 50 ); REQUIRE( s5_r3::rejected( "offset_low.bmp", b ) ); }
    { auto b = good; s5_r3::put32( b, 10, 0xFFFFFFF0u ); REQUIRE( s5_r3::rejected( "offset_high.bmp", b ) ); }

    // width x height whose stride or pixel-array arithmetic overflows, or merely exceeds a 54-byte image: parse_bmp
    // must reject them from the header alone, before allocating any channel
    for ( auto const [w, hgt] : std::vector<std::pair<std::uint32_t, std::uint32_t>>{
              { 0x7FFFFFFFu, 0x80000001u }, { 0x7FFFFFFFu, 0x7FFFFFFFu }, { 65536u, 65536u }, { 0x7FFFFFFFu, 1u }, { 1u, 0x7FFFFFFFu } } )
        for ( std::uint32_t bpp : { 24u, 32u } )
        {
            INFO( "width " << w << ", height " << hgt << ", bpp " << bpp );
            auto b = good;
            b.resize( 54 );
            s5_r3::put32( b, 18, w ); s5_r3::put32( b, 22, hgt ); s5_r3::put16( b, 28, bpp );
            REQUIRE( !feng::matrix_details::parse_bmp( b.data(), b.size() ) );
            REQUIRE( s5_r3::rejected( "overflow.bmp", b ) );
        }

    auto const missing = s5_r3::temp_path( "does_not_exist.bmp" );
    std::filesystem::remove( missing );
    REQUIRE( !feng::load_bmp( missing ) );
    REQUIRE( !feng::load_bmp( std::filesystem::temp_directory_path().string() ) );
}

TEST_CASE( "S5 a save_as_bmp file loads back to its channels", "[S5][S5-R3]" )
{
    auto const& map = feng::matrix_details::bmp_details::color_maps.at( "parula" );
    for ( unsigned cols : { 1u, 2u, 3u, 5u } )
    {
        INFO( "member save_as_bmp, 3 x " << cols );
        feng::matrix<double> m{ 3, cols };
        for ( std::size_t k = 0; k != m.size(); ++k ) m.data()[k] = static_cast<double>( ( k * 7 ) % 11 );
        auto const path = s5_r3::temp_path( "saved.bmp" );
        REQUIRE( m.save_as_bmp( path, "parula" ) );
        auto const img = feng::load_bmp( path );
        REQUIRE( img );
        auto const [mn, mx] = m.minmax();
        for ( auto const& channel : *img ) { REQUIRE( channel.row() == 3 ); REQUIRE( channel.col() == cols ); }
        for ( std::size_t r = 0; r != 3; ++r )
            for ( std::size_t c = 0; c != cols; ++c )
            {
                auto const [red, green, blue] = map( ( m[r][c] - mn ) / ( mx - mn + 1.0e-10 ) );
                REQUIRE( ( *img )[0][r][c] == red );
                REQUIRE( ( *img )[1][r][c] == green );
                REQUIRE( ( *img )[2][r][c] == blue );
            }
        std::filesystem::remove( path );
    }

    // channels with minimum 0 and maximum 255 are written unscaled, so load_bmp returns them exactly
    for ( unsigned cols : { 1u, 2u, 3u, 5u } )
    {
        INFO( "free save_as_bmp( name, r, g, b ), 3 x " << cols );
        feng::matrix<double> r{ 3, cols }, g{ 3, cols }, b{ 3, cols };
        for ( std::size_t k = 0; k != r.size(); ++k )
        {
            r.data()[k] = static_cast<double>( ( k * 37 ) % 256 );
            g.data()[k] = static_cast<double>( ( k * 91 + 5 ) % 256 );
            b.data()[k] = static_cast<double>( ( k * 53 + 11 ) % 256 );
        }
        for ( auto* m : { &r, &g, &b } )
        {
            ( *m )[0][0] = 0.0;
            ( *m )[2][cols - 1] = 255.0;
        }
        auto const path = s5_r3::temp_path( "saved_rgb.bmp" );
        REQUIRE( feng::save_as_bmp( path, r, g, b ) );
        auto const img = feng::load_bmp( path );
        REQUIRE( img );
        for ( auto const& channel : *img ) { REQUIRE( channel.row() == 3 ); REQUIRE( channel.col() == cols ); }
        for ( std::size_t y = 0; y != 3; ++y )
            for ( std::size_t x = 0; x != cols; ++x )
            {
                INFO( "pixel " << y << ", " << x );
                REQUIRE( ( *img )[0][y][x] == static_cast<std::uint8_t>( r[y][x] ) );
                REQUIRE( ( *img )[1][y][x] == static_cast<std::uint8_t>( g[y][x] ) );
                REQUIRE( ( *img )[2][y][x] == static_cast<std::uint8_t>( b[y][x] ) );
            }
        std::filesystem::remove( path );
    }
}
