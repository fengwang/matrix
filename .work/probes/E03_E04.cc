// E03/E04 probes — Session 2 (finding S1: load_npy input boundary)
//
// Build (contract deterministic check):
//   g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1 -o .work/probe_s2 .work/probes/E03_E04.cc
// Run:
//   .work/probe_s2            # all cases (post-fix: prints PASS E03 / PASS E04, exit 0)
//   .work/probe_s2 <case>...  # selected cases (pre-flight reproduction runs)
//
// -DNDEBUG is deliberate (project contract §4): better_assert is silent, real OOB is observable.
// Pre-fix expectations (probe-first, P5; recorded in .work/evidence/):
//   e03_3b / e03_11b / e03_12b / e03_ffff / e04_f4 / e03_short: ASan heap-buffer-overflow read
//   e03_noshape / e03_1d / e03_16digit / e03_negshape: std::terminate (stoul throws out of noexcept)
//   e03_be / e03_vf8: loads misinterpreted bytes and returns true (no dtype check) — reported as FAIL
//   e03_exact / e03_fortran / e03_v2: PASS even pre-fix (happy-path / convention pins)

# include "../../matrix.hpp"
# include <cmath>
# include <cstdint>
# include <cstdio>
# include <cstring>
# include <filesystem>
# include <fstream>
# include <string>
# include <vector>

namespace fs = std::filesystem;

static int failures = 0;

static bool write_file( std::string const& path, std::vector< std::uint8_t > const& bytes )
{
    std::ofstream out( path, std::ios::binary | std::ios::trunc );
    if ( !out )
        return false;
    out.write( reinterpret_cast< char const* >( bytes.data() ), static_cast< std::streamsize >( bytes.size() ) );
    return static_cast< bool >( out );
}

static std::vector< std::uint8_t > payload_of( double const* values, std::size_t n )
{
    std::vector< std::uint8_t > out( n * 8 );
    std::memcpy( out.data(), values, n * 8 );
    return out;
}

// Minimal npy v1 file (library convention): 6B magic + version(1,0) + 2B LE header length + header + payload.
static std::vector< std::uint8_t > make_v1( std::string const& header, std::vector< std::uint8_t > const& payload )
{
    std::vector< std::uint8_t > file{ 0x93, 'N', 'U', 'M', 'P', 'Y', 1, 0 };
    file.push_back( static_cast< std::uint8_t >( header.size() & 0xFF ) );
    file.push_back( static_cast< std::uint8_t >( ( header.size() >> 8 ) & 0xFF ) );
    file.insert( file.end(), header.begin(), header.end() );
    file.insert( file.end(), payload.begin(), payload.end() );
    return file;
}

// npy v2 file under the library's existing convention (4B LE header length, prefix 12).
static std::vector< std::uint8_t > make_v2( std::string const& header, std::vector< std::uint8_t > const& payload )
{
    std::vector< std::uint8_t > file{ 0x93, 'N', 'U', 'M', 'P', 'Y', 2, 0 };
    std::uint32_t const len = static_cast< std::uint32_t >( header.size() );
    file.push_back( static_cast< std::uint8_t >( len & 0xFF ) );
    file.push_back( static_cast< std::uint8_t >( ( len >> 8 ) & 0xFF ) );
    file.push_back( static_cast< std::uint8_t >( ( len >> 16 ) & 0xFF ) );
    file.push_back( static_cast< std::uint8_t >( ( len >> 24 ) & 0xFF ) );
    file.insert( file.end(), header.begin(), header.end() );
    file.insert( file.end(), payload.begin(), payload.end() );
    return file;
}

static std::string header_with( std::string const& descr, std::string const& shape, bool fortran = false )
{
    return "{'descr': '" + descr + "', 'fortran_order': " + ( fortran ? "True" : "False" ) + ", 'shape': " + shape + ", }";
}

struct case_result
{
    const char* id;
    bool expected_ok;
    bool actual_ok;
    bool content_ok;
};

// Runs one case: writes file, loads, checks ok (+ content if expected_ok), cleans up.
static case_result run_load_case( char const* id, std::string const& file_name, std::vector< std::uint8_t > const& bytes, feng::matrix<double>& m )
{
    fs::create_directories( ".work/probes_s2" );
    case_result r{ id, false, false, true };
    if ( !bytes.empty() )
    {
        if ( !write_file( file_name, bytes ) )
        {
            std::printf( "FAIL %s: could not write %s\n", id, file_name.c_str() );
            r.content_ok = false;
            return r;
        }
    }
    r.actual_ok = m.load_npy( file_name.c_str() );
    fs::remove( file_name );
    return r;
}

int main( int argc, char** argv )
{
    std::vector< std::string > select;
    for ( int i = 1; i < argc; ++i )
        select.push_back( argv[ i ] );

    std::string const dir = ".work/probes_s2/";
    auto wanted = [&select]( char const* id ) { return select.empty() || std::find( select.begin(), select.end(), id ) != select.end(); };

    // ---------- E03: malformed/truncated files must return false, ASan-clean ----------

    if ( wanted( "e03_3b" ) )
    {
        feng::matrix<double> m;
        case_result const r = run_load_case( "e03_3b", dir + "e03_3b.npy", std::vector< std::uint8_t >{ 0x93, 'N', 'U' }, m );
        std::printf( "%s e03_3b: 3-byte file (truncated magic) -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_11b" ) )
    {
        feng::matrix<double> m;
        // 11 bytes: magic + version 1 + header_length 0xFFFF + 3 bytes. Fails min-size (12) check.
        std::vector< std::uint8_t > file{ 0x93, 'N', 'U', 'M', 'P', 'Y', 1, 0, 0xFF, 0xFF, 'a', 'b', 'c' };
        case_result const r = run_load_case( "e03_11b", dir + "e03_11b.npy", file, m );
        std::printf( "%s e03_11b: 11-byte file -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_12b" ) )
    {
        feng::matrix<double> m;
        // Exactly 12 bytes: magic + version 1 + header_length 2 + 2 header bytes (no shape token).
        std::vector< std::uint8_t > file{ 0x93, 'N', 'U', 'M', 'P', 'Y', 1, 0, 2, 0, '{', '}' };
        case_result const r = run_load_case( "e03_12b", dir + "e03_12b.npy", file, m );
        std::printf( "%s e03_12b: 12-byte file, no shape token -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_ffff" ) )
    {
        feng::matrix<double> m;
        // v2 convention, header_length = 0xFFFFFFFF (overflow of offset arithmetic pre-fix).
        std::vector< std::uint8_t > file{ 0x93, 'N', 'U', 'M', 'P', 'Y', 2, 0, 0xFF, 0xFF, 0xFF, 0xFF, '{', 'a', 'b', 'c' };
        case_result const r = run_load_case( "e03_ffff", dir + "e03_ffff.npy", file, m );
        std::printf( "%s e03_ffff: v2 header_length 0xFFFFFFFF -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_trunchdr" ) )
    {
        feng::matrix<double> m;
        // E03 second file: valid magic, v1, header_length claims 80, only 11 header bytes present (21B file).
        std::vector< std::uint8_t > file{ 0x93, 'N', 'U', 'M', 'P', 'Y', 1, 0, 80, 0, '{', '\'', 'd', 'e', 's', 'c', 'r', '\'', ':', ' ' };
        case_result const r = run_load_case( "e03_trunchdr", dir + "e03_trunchdr.npy", file, m );
        std::printf( "%s e03_trunchdr: truncated header (claims 80B) -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_noshape" ) )
    {
        feng::matrix<double> m;
        std::string const h = "{'descr': '<f8', 'fortran_order': False, }";
        case_result const r = run_load_case( "e03_noshape", dir + "e03_noshape.npy", make_v1( h, {} ), m );
        std::printf( "%s e03_noshape: missing 'shape' token -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_1d" ) )
    {
        feng::matrix<double> m;
        double const v[2] = { 1.0, 2.0 };
        case_result const r = run_load_case( "e03_1d", dir + "e03_1d.npy", make_v1( header_with( "<f8", "(2,)" ), payload_of( v, 2 ) ), m );
        std::printf( "%s e03_1d: 1-D shape (2,) -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_negshape" ) )
    {
        feng::matrix<double> m;
        double const v[2] = { 1.0, 2.0 };
        case_result const r = run_load_case( "e03_negshape", dir + "e03_negshape.npy", make_v1( header_with( "<f8", "(-1, 2)" ), payload_of( v, 2 ) ), m );
        std::printf( "%s e03_negshape: negative shape (-1, 2) -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_16digit" ) )
    {
        feng::matrix<double> m;
        double const v[2] = { 1.0, 2.0 };
        // 30-digit row shape: stoul would throw std::out_of_range pre-fix (terminate under noexcept).
        case_result const r = run_load_case( "e03_16digit", dir + "e03_16digit.npy", make_v1( header_with( "<f8", "(999999999999999999999999999999, 2)" ), payload_of( v, 2 ) ), m );
        std::printf( "%s e03_16digit: 30-digit shape (stoul overflow) -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    // ---------- E04: dtype must match the target value_type ----------

    if ( wanted( "e04_f4" ) )
    {
        feng::matrix<double> m;
        std::vector< std::uint8_t > const payload( 8, 0x3F ); // 8 raw bytes, float32 file
        case_result const r = run_load_case( "e04_f4", dir + "e04_f4.npy", make_v1( header_with( "<f4", "(1, 2)" ), payload ), m );
        std::printf( "%s e04_f4: float32 1x2 into matrix<double> -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e04_u1" ) )
    {
        feng::matrix<double> m;
        std::vector< std::uint8_t > const payload{ 1, 2, 3, 4, 5, 6 };
        case_result const r = run_load_case( "e04_u1", dir + "e04_u1.npy", make_v1( header_with( "|u1", "(2, 3)" ), payload ), m );
        std::printf( "%s e04_u1: uint8 2x3 into matrix<double> -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_be" ) )
    {
        feng::matrix<double> m;
        double const v[2] = { 1.0, 2.0 };
        case_result const r = run_load_case( "e03_be", dir + "e03_be.npy", make_v1( header_with( ">f8", "(1, 2)" ), payload_of( v, 2 ) ), m );
        std::printf( "%s e03_be: big-endian '>f8' into matrix<double> -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_vf8" ) )
    {
        feng::matrix<double> m;
        double const v[2] = { 1.0, 2.0 };
        case_result const r = run_load_case( "e03_vf8", dir + "e03_vf8.npy", make_v1( header_with( "Vf8", "(1, 2)" ), payload_of( v, 2 ) ), m );
        std::printf( "%s e03_vf8: native-endian 'Vf8' into matrix<double> -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    // ---------- boundary + happy-path pins (must PASS pre- and post-fix) ----------

    if ( wanted( "e03_missing" ) )
    {
        feng::matrix<double> m;
        case_result const r = run_load_case( "e03_missing", dir + "does_not_exist.npy", {}, m );
        std::printf( "%s e03_missing: missing file -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_exact" ) )
    {
        feng::matrix<double> m;
        double const v[1] = { 7.25 };
        case_result const r = run_load_case( "e03_exact", dir + "e03_exact.npy", make_v1( header_with( "<f8", "(1, 1)" ), payload_of( v, 1 ) ), m );
        bool const content = m.row() == 1 && m.col() == 1 && std::abs( m[ 0 ][ 0 ] - 7.25 ) < 1e-12;
        std::printf( "%s e03_exact: payload ends exactly at file tail -> ok=%d content=%d\n", ( r.actual_ok && content ) ? "ok  " : "FAIL", static_cast< int >( r.actual_ok ), static_cast< int >( content ) );
        if ( !r.actual_ok || !content ) ++failures;
    }

    if ( wanted( "e03_short" ) )
    {
        feng::matrix<double> m;
        double const v[1] = { 7.25 };
        std::vector< std::uint8_t > short_payload = payload_of( v, 1 );
        short_payload.pop_back(); // 7 of 8 bytes: truncated payload
        case_result const r = run_load_case( "e03_short", dir + "e03_short.npy", make_v1( header_with( "<f8", "(1, 1)" ), short_payload ), m );
        std::printf( "%s e03_short: payload 1 byte short -> ok=%d\n", r.actual_ok ? "FAIL" : "ok  ", static_cast< int >( r.actual_ok ) );
        if ( r.actual_ok ) ++failures;
    }

    if ( wanted( "e03_fortran" ) )
    {
        feng::matrix<double> m;
        // fortran_order True, 2x3 logical array F = [[1,2,3],[4,5,6]]: payload = column-major [1,4,2,5,3,6].
        double const v[6] = { 1.0, 4.0, 2.0, 5.0, 3.0, 6.0 };
        case_result const r = run_load_case( "e03_fortran", dir + "e03_fortran.npy", make_v1( header_with( "<f8", "(2, 3)", true ), payload_of( v, 6 ) ), m );
        bool const content = m.row() == 3 && m.col() == 2
            && std::abs( m[ 0 ][ 0 ] - 1.0 ) < 1e-12 && std::abs( m[ 0 ][ 1 ] - 4.0 ) < 1e-12
            && std::abs( m[ 1 ][ 0 ] - 2.0 ) < 1e-12 && std::abs( m[ 1 ][ 1 ] - 5.0 ) < 1e-12
            && std::abs( m[ 2 ][ 0 ] - 3.0 ) < 1e-12 && std::abs( m[ 2 ][ 1 ] - 6.0 ) < 1e-12;
        std::printf( "%s e03_fortran: fortran_order True 2x3 (transpose pin) -> ok=%d content=%d\n", ( r.actual_ok && content ) ? "ok  " : "FAIL", static_cast< int >( r.actual_ok ), static_cast< int >( content ) );
        if ( !r.actual_ok || !content ) ++failures;
    }

    if ( wanted( "e03_v2" ) )
    {
        feng::matrix<double> m;
        double const v[2] = { 1.5, 2.5 };
        case_result const r = run_load_case( "e03_v2", dir + "e03_v2.npy", make_v2( header_with( "<f8", "(1, 2)" ), payload_of( v, 2 ) ), m );
        bool const content = m.row() == 1 && m.col() == 2 && std::abs( m[ 0 ][ 0 ] - 1.5 ) < 1e-12 && std::abs( m[ 0 ][ 1 ] - 2.5 ) < 1e-12;
        std::printf( "%s e03_v2: v2-convention 1x2 f8 (convention pin) -> ok=%d content=%d\n", ( r.actual_ok && content ) ? "ok  " : "FAIL", static_cast< int >( r.actual_ok ), static_cast< int >( content ) );
        if ( !r.actual_ok || !content ) ++failures;
    }

    if ( failures == 0 )
    {
        std::printf( "PASS E03\n" );
        std::printf( "PASS E04\n" );
        return 0;
    }
    std::printf( "FAIL: %d case(s) not as expected\n", failures );
    return 1;
}
