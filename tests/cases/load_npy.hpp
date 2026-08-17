#include <cassert>
TEST_CASE( "Loading npy files", "[load_npy]" )
{
    {
        feng::matrix<std::uint8_t> m;
        m.load_npy( "./images/u8.npy" );
        REQUIRE( m[0][0] == 4 ); REQUIRE( m[0][1] == 1 ); REQUIRE( m[0][2] == 8 );
        REQUIRE( m[1][0] == 9 ); REQUIRE( m[1][1] == 1 ); REQUIRE( m[1][2] == 5 );
    }
    {
        feng::matrix<std::int8_t> m;
        m.load_npy( "./images/8.npy" );
        REQUIRE( m[0][0] == 4 ); REQUIRE( m[0][1] == 1 ); REQUIRE( m[0][2] == 8 );
        REQUIRE( m[1][0] == 9 ); REQUIRE( m[1][1] == 1 ); REQUIRE( m[1][2] == 5 );
    }
    // [[4.815519 , 1.0601262, 8.989337 ],
    //  [9.510697 , 1.8137231, 5.7381544]]
    {
        feng::matrix<float> m;
        m.load_npy( "./images/32.npy" );
        REQUIRE( std::abs(4.815519-m[0][0]) < 1.0e-5 ); REQUIRE( std::abs(1.0601262-m[0][1]) < 1.0e-5 ); REQUIRE( std::abs(8.989337-m[0][2]) < 1.0e-5 );
        REQUIRE( std::abs(9.510697-m[1][0]) < 1.0e-5 ); REQUIRE( std::abs(1.8137231-m[1][1]) < 1.0e-5 ); REQUIRE( std::abs(5.7381544-m[1][2]) < 1.0e-5 );
    }
    {
        feng::matrix<double> m;
        m.load_npy( "./images/64.npy" );
        REQUIRE( std::abs(4.815519-m[0][0]) < 1.0e-5 ); REQUIRE( std::abs(1.0601262-m[0][1]) < 1.0e-5 ); REQUIRE( std::abs(8.989337-m[0][2]) < 1.0e-5 );
        REQUIRE( std::abs(9.510697-m[1][0]) < 1.0e-5 ); REQUIRE( std::abs(1.8137231-m[1][1]) < 1.0e-5 ); REQUIRE( std::abs(5.7381544-m[1][2]) < 1.0e-5 );
    }
}

// Session 2 negative-path cases (finding S1, docs/session_2_contract.yaml).
// Malformed / foreign-dtype / truncated npy files must be rejected with a clean `false`,
// leaving the matrix state exactly as it was. Crafted bytes are written at runtime into
// tmp/ (gitignored) and removed per case so repeated runs stay deterministic.

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    void write_bytes( char const* const path, std::vector< std::uint8_t > const& bytes )
    {
        std::ofstream out( path, std::ios::binary );
        out.write( reinterpret_cast< char const* >( bytes.data() ), static_cast< std::streamsize >( bytes.size() ) );
    }

    std::string dict_header( char const* const descr, char const* const shape )
    {
        return std::string( "{ 'descr': '" ) + descr + "', 'fortran_order': False, 'shape': " + shape + ", }";
    }

    // NPY v1: magic + version 01 00 + 2-byte LE header length + header + payload
    std::vector< std::uint8_t > make_v1( std::string const& header, std::vector< std::uint8_t > const& payload )
    {
        std::vector< std::uint8_t > f;
        f.insert( f.end(), { 0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00 } );
        std::uint16_t const len = static_cast< std::uint16_t >( header.size() );
        f.push_back( static_cast< std::uint8_t >( len & 0xFF ) );
        f.push_back( static_cast< std::uint8_t >( ( len >> 8 ) & 0xFF ) );
        f.insert( f.end(), header.begin(), header.end() );
        f.insert( f.end(), payload.begin(), payload.end() );
        return f;
    }
}

TEST_CASE( "load_npy rejects an unopenable or truncated (3-byte) file", "[load_npy]" )
{
    std::filesystem::create_directories( "tmp" );
    std::string const path_3b = "tmp/s2_neg_trunc3b.npy";
    write_bytes( path_3b.c_str(), { 0x93, 'N', 'U' } );

    feng::matrix< double > m;
    REQUIRE( m.load_npy( "./images/64.npy" ) );              // valid baseline state (2x3)
    std::size_t const r0 = m.row();
    std::size_t const c0 = m.col();
    double const v00 = m[0][0];

    REQUIRE( !m.load_npy( "tmp/s2_neg_missing.npy" ) );      // unopenable file
    REQUIRE( !m.load_npy( path_3b.c_str() ) );               // 3 bytes: below the 12-byte minimum

    REQUIRE( m.row() == r0 );                                // no partial state
    REQUIRE( m.col() == c0 );
    REQUIRE( m[0][0] == v00 );

    std::filesystem::remove( path_3b );
}

TEST_CASE( "load_npy rejects a truncated header", "[load_npy]" )
{
    std::filesystem::create_directories( "tmp" );

    // 11-byte file: valid magic + version 1, header_length = 0xFFFF, only 3 trailing bytes
    std::string const path_11b = "tmp/s2_neg_trunc11b.npy";
    write_bytes( path_11b.c_str(), { 0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00 } );

    // 21-byte file: header_length field claims 80 (0x50 LE16) but only 9 header bytes exist
    std::string const path_21b = "tmp/s2_neg_trunc21b.npy";
    std::vector< std::uint8_t > b21;
    b21.insert( b21.end(), { 0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00, 0x50, 0x00 } );
    b21.insert( b21.end(), { 0x7B, 0x20, 0x27, 'd', 'e', 's', 'c', 'r', 0x27 } );
    b21.insert( b21.end(), { 0x41, 0x42 } );
    write_bytes( path_21b.c_str(), b21 );

    feng::matrix< double > m;
    REQUIRE( m.load_npy( "./images/64.npy" ) );
    std::size_t const r0 = m.row();
    std::size_t const c0 = m.col();
    double const v00 = m[0][0];

    REQUIRE( !m.load_npy( path_11b.c_str() ) );
    REQUIRE( !m.load_npy( path_21b.c_str() ) );

    REQUIRE( m.row() == r0 );
    REQUIRE( m.col() == c0 );
    REQUIRE( m[0][0] == v00 );

    std::filesystem::remove( path_11b );
    std::filesystem::remove( path_21b );
}

TEST_CASE( "load_npy rejects a missing or malformed shape token", "[load_npy]" )
{
    std::filesystem::create_directories( "tmp" );
    std::vector< std::uint8_t > const payload( 64, 0x41 );

    std::vector< std::pair< std::string, std::string > > const variants =
    {
        { "tmp/s2_neg_noshape.npy", "{ 'descr': '<f8', 'fortran_order': False }" },
        { "tmp/s2_neg_1d.npy",      dict_header( "<f8", "(2,)" ) },
        { "tmp/s2_neg_neg.npy",     dict_header( "<f8", "(-1, 2)" ) },
        { "tmp/s2_neg_alpha.npy",   dict_header( "<f8", "(a, b)" ) },
        { "tmp/s2_neg_big.npy",     dict_header( "<f8", "(999999999999999999999999999999, 2)" ) }
    };

    feng::matrix< double > m;
    REQUIRE( m.load_npy( "./images/64.npy" ) );
    std::size_t const r0 = m.row();
    std::size_t const c0 = m.col();
    double const v00 = m[0][0];

    for ( auto const& v : variants )
    {
        write_bytes( v.first.c_str(), make_v1( v.second, payload ) );
        REQUIRE( !m.load_npy( v.first.c_str() ) );
    }

    REQUIRE( m.row() == r0 );
    REQUIRE( m.col() == c0 );
    REQUIRE( m[0][0] == v00 );

    for ( auto const& v : variants )
        std::filesystem::remove( v.first );
}

TEST_CASE( "load_npy rejects an overflowing header_length (0xFFFFFFFF)", "[load_npy]" )
{
    std::filesystem::create_directories( "tmp" );
    std::string const path = "tmp/s2_neg_hlenff.npy";

    // v2 layout: length field claims 0xFFFFFFFF; only 4 header bytes + 2 payload bytes exist
    std::vector< std::uint8_t > b;
    b.insert( b.end(), { 0x93, 'N', 'U', 'M', 'P', 'Y', 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF } );
    b.insert( b.end(), { '{', ' ', 0x27, 'a' } );
    b.insert( b.end(), { 0x41, 0x42 } );
    write_bytes( path.c_str(), b );

    feng::matrix< double > m;
    REQUIRE( m.load_npy( "./images/64.npy" ) );
    std::size_t const r0 = m.row();
    std::size_t const c0 = m.col();
    double const v00 = m[0][0];

    REQUIRE( !m.load_npy( path.c_str() ) );

    REQUIRE( m.row() == r0 );
    REQUIRE( m.col() == c0 );
    REQUIRE( m[0][0] == v00 );

    std::filesystem::remove( path );
}

TEST_CASE( "load_npy rejects a foreign dtype", "[load_npy]" )
{
    std::filesystem::create_directories( "tmp" );

    // well-formed 1x2 float32 file loaded into matrix<double> (the E04 acceptance)
    std::string const path_f4 = "tmp/s2_neg_f4.npy";
    write_bytes( path_f4.c_str(), make_v1( dict_header( "<f4", "(1, 2)" ), { 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x40 } ) );

    // well-formed 1x2 big-endian float64 file loaded into matrix<double> (silent misload pre-fix)
    std::string const path_be = "tmp/s2_neg_be.npy";
    write_bytes( path_be.c_str(), make_v1( dict_header( ">f8", "(1, 2)" ), std::vector< std::uint8_t >( 16, 0x41 ) ) );

    feng::matrix< double > m;
    REQUIRE( m.load_npy( "./images/64.npy" ) );
    std::size_t const r0 = m.row();
    std::size_t const c0 = m.col();
    double const v00 = m[0][0];
    double const v12 = m[1][2];

    REQUIRE( !m.load_npy( path_f4.c_str() ) );
    REQUIRE( !m.load_npy( path_be.c_str() ) );

    REQUIRE( m.row() == r0 );
    REQUIRE( m.col() == c0 );
    REQUIRE( m[0][0] == v00 );
    REQUIRE( m[1][2] == v12 );

    std::filesystem::remove( path_f4 );
    std::filesystem::remove( path_be );
}

