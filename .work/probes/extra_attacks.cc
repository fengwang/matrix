// S2 adversarial verifier extra attacks (fresh-context pass; NOT part of the E03/E04 probe).
// Builds with the contract ASan flags: -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
# include "../../matrix.hpp"

static int failures = 0;

static void write_file( const char* path, std::vector< std::uint8_t > const& b )
{
    std::ofstream out( path, std::ios::binary );
    out.write( reinterpret_cast< char const* >( b.data() ), ( std::streamsize ) b.size() );
}

static std::vector< std::uint8_t > v1( std::string const& header, std::vector< std::uint8_t > const& payload )
{
    std::vector< std::uint8_t > f;
    f.insert( f.end(), { 0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00 } );
    std::uint16_t const len = ( std::uint16_t ) header.size();
    f.push_back( ( std::uint8_t ) len );
    f.push_back( ( std::uint8_t ) ( len >> 8 ) );
    f.insert( f.end(), header.begin(), header.end() );
    f.insert( f.end(), payload.begin(), payload.end() );
    return f;
}

int main( int argc, char** argv )
{
    std::filesystem::create_directories( "tmp" );
    std::string sel = argc > 1 ? argv[ 1 ] : "all";
    auto run = [&]( char const* id, bool const cond )
    {
        if ( sel != "all" && sel != id )
            return;
        std::printf( "%s %s\n", cond ? "ok  " : "BAD", id );
        if ( !cond )
            ++failures;
    };

    // A1: v2, 12-byte file total: data_prefix 12 leaves 0 bytes for the header.
    // header_length must be 0 -> empty header -> V4 reject. Any length byte >= 1 -> bound reject.
    {
        std::vector< std::uint8_t > b{ 0x93, 'N', 'U', 'M', 'P', 'Y', 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };
        write_file( "tmp/adv_a1.npy", b );
        feng::matrix< double > m;
        run( "a1_v2_12b_len0", !m.load_npy( "tmp/adv_a1.npy" ) );
        b[ 8 ] = 1; // claim 1 header byte: 1 > 12-12=0
        write_file( "tmp/adv_a1.npy", b );
        feng::matrix< double > m2;
        run( "a1_v2_12b_len1", !m2.load_npy( "tmp/adv_a1.npy" ) );
    }

    // A2: v1, header_length = 0 exactly (bound passes inclusively), then V4 rejects.
    {
        std::vector< std::uint8_t > b{ 0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00, 0x00, 0x00, 0xAA, 0xBB };
        write_file( "tmp/adv_a2.npy", b );
        feng::matrix< double > m;
        run( "a2_v1_hlen0", !m.load_npy( "tmp/adv_a2.npy" ) );
    }

    // A3: 1 MB header of '{' + junk: no descr/shape -> fast clean false, no OOM beyond the file.
    {
        std::string big( 1024 * 1024, '{' );
        big += "zz";
        std::vector< std::uint8_t > payload( 32, 0x41 );
        write_file( "tmp/adv_a3.npy", v1( big, payload ) );
        feng::matrix< double > m;
        run( "a3_1mb_junk_header", !m.load_npy( "tmp/adv_a3.npy" ) );
    }

    // A4: directory passed as the file name (OS edge).
    {
        feng::matrix< double > m;
        run( "a4_directory_name", !m.load_npy( "tmp" ) );
    }

    // A5: dtype mismatch on an otherwise valid file must leave a previously valid matrix
    //     untouched (state pinned on non-trivial content).
    {
        std::vector< std::uint8_t > f4_payload = { 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x00, 0x41 };
        write_file( "tmp/adv_a5.npy", v1( "{ 'descr': '<f4', 'fortran_order': False, 'shape': (2, 3), }", f4_payload ) );
        feng::matrix< double > m;
        if ( !m.load_npy( "./images/64.npy" ) )
        {
            std::printf( "BAD a5_setup\n" );
            ++failures;
            return 1;
        }
        double const v00 = m[0][0];
        double const v12 = m[1][2];
        std::size_t const r0 = m.row();
        if ( m.load_npy( "tmp/adv_a5.npy" ) || m.row() != r0 || m.col() != 3 || m[0][0] != v00 || m[1][2] != v12 )
        {
            std::printf( "BAD a5_state_changed\n" );
            ++failures;
        }
        else
            std::printf( "ok   a5_state_unchanged\n" );
    }

    // A6: shape claims a giant matrix (2^30 x 2^30) with a tiny file: must reject before resize.
    {
        std::string const header = "{ 'descr': '<f8', 'fortran_order': False, 'shape': (1073741824, 1073741824), }";
        write_file( "tmp/adv_a6.npy", v1( header, std::vector< std::uint8_t >( 16, 0x41 ) ) );
        feng::matrix< double > m;
        run( "a6_giant_shape_no_oom", !m.load_npy( "tmp/adv_a6.npy" ) );
        run( "a6_shape_unchanged", m.row() == 0 && m.col() == 0 );
    }

    // A7: third hazard: row=2^40, col=2^24 -> product wraps size_t. Must reject in the
    // overflow-checked multiply, before any allocation.
    {
        std::string const header = "{ 'descr': '<f8', 'fortran_order': False, 'shape': (1099511627776, 16777216), }";
        write_file( "tmp/adv_a7.npy", v1( header, std::vector< std::uint8_t >( 16, 0x41 ) ) );
        feng::matrix< double > m;
        run( "a7_wrapping_product_rejected", !m.load_npy( "tmp/adv_a7.npy" ) );
        run( "a7_shape_unchanged", m.row() == 0 && m.col() == 0 );
    }

    std::filesystem::remove_all( "tmp/adv_a1.npy" );
    std::filesystem::remove_all( "tmp/adv_a2.npy" );
    std::filesystem::remove_all( "tmp/adv_a3.npy" );
    std::filesystem::remove_all( "tmp/adv_a5.npy" );
    std::filesystem::remove_all( "tmp/adv_a6.npy" );
    std::filesystem::remove_all( "tmp/adv_a7.npy" );

    std::printf( failures == 0 ? "PASS EXTRA-ATTACKS\n" : "FAIL EXTRA-ATTACKS (%d)\n", failures );
    return failures == 0 ? 0 : 1;
}
