// S5-R1 (PR-7, F07): the NPY loader validates every header field; S5-R4 (PR-2, D-011): it is transactional.
// numpy-made fixtures live in ./tests/fixtures/s5/ (tests/fixtures/s5/make_npy.py); the suite runs from the repo root.
#include <algorithm>
#include <bit>
#include <cctype>
#include <complex>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <unistd.h> // getpid: per-process temp names

namespace s5_r1
{
    inline std::string fixture( char const* name ) { return std::string{ "./tests/fixtures/s5/" } + name; }

    // The process id keeps two suite binaries run by hand at once from sharing a temp path.
    inline std::string temp_path( std::string const& name )
    {
        return ( std::filesystem::temp_directory_path() / ( "feng_s5_r1_" + std::to_string( ::getpid() ) + "_" + name ) ).string();
    }

    inline std::string temp_file( std::string const& name, std::string const& bytes )
    {
        auto const path = s5_r1::temp_path( name );
        std::ofstream ofs( path, std::ios::binary | std::ios::trunc );
        ofs.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) );
        ofs.close();
        return path;
    }

    // An NPY file with the given header text (a newline is appended) and payload; version 1 uses a 2-byte length.
    inline std::string npy_bytes( std::string header, std::string const& payload, int version = 1 )
    {
        header += '\n';
        std::string out{ "\x93NUMPY", 6 };
        out += static_cast<char>( version );
        out += '\0';
        std::size_t const n = header.size();
        out += static_cast<char>( n & 0xff );
        out += static_cast<char>( ( n >> 8 ) & 0xff );
        if ( version != 1 )
        {
            out += static_cast<char>( ( n >> 16 ) & 0xff );
            out += static_cast<char>( ( n >> 24 ) & 0xff );
        }
        return out + header + payload;
    }

    template < typename T >
    std::string payload_of( std::vector<T> const& v )
    {
        std::string s( v.size() * sizeof( T ), '\0' );
        if ( !v.empty() ) std::memcpy( s.data(), v.data(), s.size() );
        return s;
    }

    inline feng::matrix<double> prefilled()
    {
        feng::matrix<double> m{ 2, 2 };
        m[0][0] = 1.0; m[0][1] = 2.0; m[1][0] = 3.0; m[1][1] = 4.0;
        return m;
    }

    inline bool is_prefilled( feng::matrix<double> const& m )
    {
        return m.row() == 2 && m.col() == 2 && m[0][0] == 1.0 && m[0][1] == 2.0 && m[1][0] == 3.0 && m[1][1] == 4.0;
    }

    // Loads the synthetic file into a pre-filled 2x2 matrix<double>; true when the load failed and kept it unchanged.
    inline bool rejected( std::string const& name, std::string const& bytes )
    {
        auto m = prefilled();
        bool const ok = m.load_npy( temp_file( name, bytes ) );
        return !ok && is_prefilled( m );
    }

    inline std::string const six_doubles = payload_of( std::vector<double>{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } );
}

TEST_CASE( "S5 load_npy rejects a 3-byte file and keeps the destination", "[S5][S5-R4]" )
{
    auto m = s5_r1::prefilled();
    std::ostringstream captured;
    auto* const old = std::cerr.rdbuf( captured.rdbuf() );
    bool const ok = m.load_npy( s5_r1::temp_file( "three_bytes.npy", std::string{ "\x93NU", 3 } ) );
    std::cerr.rdbuf( old );
    REQUIRE( !ok );
    REQUIRE( s5_r1::is_prefilled( m ) );
    std::string const text = captured.str();
    REQUIRE( text.find( "load_npy" ) != std::string::npos );
    REQUIRE( std::count( text.begin(), text.end(), '\n' ) == 1 );
}

TEST_CASE( "S5 load_npy fails on a missing file and keeps the destination", "[S5][S5-R4]" )
{
    auto m = s5_r1::prefilled();
    auto const path = s5_r1::temp_path( "does_not_exist.npy" );
    std::filesystem::remove( path );
    REQUIRE( !m.load_npy( path ) );
    REQUIRE( s5_r1::is_prefilled( m ) );
    REQUIRE( !m.load_npy( std::filesystem::temp_directory_path().string() ) ); // a directory
    REQUIRE( s5_r1::is_prefilled( m ) );
}

TEST_CASE( "S5 load_npy rejects a shape whose byte size overflows", "[S5][S5-R1]" )
{
    auto const bytes = s5_r1::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (1099511627776, 1099511627776), }",
                                         s5_r1::payload_of( std::vector<double>{ 1.0 } ) );
    REQUIRE( s5_r1::rejected( "huge_shape.npy", bytes ) );
    REQUIRE( s5_r1::rejected( "huge_dim.npy", s5_r1::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (99999999999999999999999, 1), }", s5_r1::six_doubles ) ) );
}

TEST_CASE( "S5 load_npy requires the dtype to match the element type exactly", "[S5][S5-R1]" )
{
    auto m = s5_r1::prefilled();
    REQUIRE( !m.load_npy( s5_r1::fixture( "f4_le.npy" ) ) );
    REQUIRE( s5_r1::is_prefilled( m ) );

    feng::matrix<float> f;
    REQUIRE( f.load_npy( s5_r1::fixture( "f4_le.npy" ) ) );
    REQUIRE( f.row() == 2 ); REQUIRE( f.col() == 3 );
    REQUIRE( f[0][0] == 1.5f ); REQUIRE( f[0][1] == -2.25f ); REQUIRE( f[0][2] == 3.0f );
    REQUIRE( f[1][0] == 4.125f ); REQUIRE( f[1][1] == 5.0f ); REQUIRE( f[1][2] == -6.5f );

    feng::matrix<std::int64_t> i8;
    REQUIRE( !i8.load_npy( s5_r1::fixture( "f8_le.npy" ) ) );
    feng::matrix<std::uint32_t> u4;
    REQUIRE( !u4.load_npy( s5_r1::fixture( "i4_be.npy" ) ) );
    feng::matrix<std::int8_t> i1;
    REQUIRE( !i1.load_npy( s5_r1::fixture( "u1.npy" ) ) );
}

TEST_CASE( "S5 load_npy loads little- and big-endian numpy files to numpy's values", "[S5][S5-R1]" )
{
    for ( char const* name : { "f8_le.npy", "f8_be.npy" } )
    {
        auto m = s5_r1::prefilled();
        REQUIRE( m.load_npy( s5_r1::fixture( name ) ) );
        REQUIRE( m.row() == 2 ); REQUIRE( m.col() == 3 );
        REQUIRE( m[0][0] == 1.5 ); REQUIRE( m[0][1] == -2.25 ); REQUIRE( m[0][2] == 3.0 );
        REQUIRE( m[1][0] == 4.125 ); REQUIRE( m[1][1] == 5.0 ); REQUIRE( m[1][2] == -6.5 );
    }
    feng::matrix<std::int32_t> i;
    REQUIRE( i.load_npy( s5_r1::fixture( "i4_be.npy" ) ) );
    REQUIRE( i.row() == 2 ); REQUIRE( i.col() == 3 );
    REQUIRE( i[0][0] == 1 ); REQUIRE( i[0][1] == -2 ); REQUIRE( i[0][2] == 3 );
    REQUIRE( i[1][0] == 70000 ); REQUIRE( i[1][1] == -80000 ); REQUIRE( i[1][2] == 2147483647 );
    for ( char const* name : { "c16_le.npy", "c16_be.npy" } )
    {
        feng::matrix<std::complex<double>> c;
        REQUIRE( c.load_npy( s5_r1::fixture( name ) ) );
        REQUIRE( c.row() == 2 ); REQUIRE( c.col() == 2 );
        REQUIRE( c[0][0] == std::complex<double>{ 1.0, 2.0 } ); REQUIRE( c[0][1] == std::complex<double>{ -3.5, 0.25 } );
        REQUIRE( c[1][0] == std::complex<double>{ 0.0, -1.0 } ); REQUIRE( c[1][1] == std::complex<double>{ 7.0, 0.0 } );
    }
    feng::matrix<std::uint8_t> u;
    REQUIRE( u.load_npy( s5_r1::fixture( "u1.npy" ) ) );
    REQUIRE( u.row() == 2 ); REQUIRE( u.col() == 3 );
    REQUIRE( u[0][0] == 0 ); REQUIRE( u[0][1] == 1 ); REQUIRE( u[0][2] == 255 );
    REQUIRE( u[1][0] == 128 ); REQUIRE( u[1][1] == 7 ); REQUIRE( u[1][2] == 42 );
}

TEST_CASE( "S5 load_npy loads a Fortran-order 2x3 as the logical matrix", "[S5][S5-R1]" )
{
    feng::matrix<double> m;
    REQUIRE( m.load_npy( s5_r1::fixture( "fortran_2x3.npy" ) ) );
    REQUIRE( m.row() == 2 ); REQUIRE( m.col() == 3 );
    REQUIRE( m[0][0] == 1.5 ); REQUIRE( m[0][1] == -2.25 ); REQUIRE( m[0][2] == 3.0 );
    REQUIRE( m[1][0] == 4.125 ); REQUIRE( m[1][1] == 5.0 ); REQUIRE( m[1][2] == -6.5 );
}

TEST_CASE( "S5 load_npy loads a 1-D (3,) array as 1x3 and rejects rank 0 and 3", "[S5][S5-R1]" )
{
    feng::matrix<double> m;
    REQUIRE( m.load_npy( s5_r1::fixture( "one_d.npy" ) ) );
    REQUIRE( m.row() == 1 ); REQUIRE( m.col() == 3 );
    REQUIRE( m[0][0] == 10.0 ); REQUIRE( m[0][1] == 20.0 ); REQUIRE( m[0][2] == 30.0 );

    auto p = s5_r1::prefilled();
    REQUIRE( !p.load_npy( s5_r1::fixture( "three_d.npy" ) ) );
    REQUIRE( s5_r1::is_prefilled( p ) );
    REQUIRE( s5_r1::rejected( "rank0.npy", s5_r1::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (), }",
                                                             s5_r1::payload_of( std::vector<double>{ 1.0 } ) ) ) );
}

TEST_CASE( "S5 load_npy rejects object and structured dtypes", "[S5][S5-R1]" )
{
    for ( char const* name : { "object.npy", "structured.npy" } )
    {
        auto m = s5_r1::prefilled();
        REQUIRE( !m.load_npy( s5_r1::fixture( name ) ) );
        REQUIRE( s5_r1::is_prefilled( m ) );
    }
}

TEST_CASE( "S5 load_npy accepts the dict literal variants numpy may write", "[S5][S5-R1]" )
{
    using s5_r1::npy_bytes;
    using s5_r1::six_doubles;
    for ( std::string const& header : {
              std::string{ "{\"descr\": \"<f8\", \"fortran_order\": False, \"shape\": (2, 3)}" },
              std::string{ "{'shape': (2,3), 'descr': '=f8', 'fortran_order': False}   " },
              std::string{ "{'fortran_order': False, 'shape': ( 2 , 3 , ), 'descr': '<f8', }          " } } )
    {
        for ( int version : { 1, 2, 3 } )
        {
            feng::matrix<double> m;
            REQUIRE( m.load_npy( s5_r1::temp_file( "variant.npy", npy_bytes( header, six_doubles, version ) ) ) );
            REQUIRE( m.row() == 2 ); REQUIRE( m.col() == 3 );
            REQUIRE( m[0][0] == 1.0 ); REQUIRE( m[1][2] == 6.0 );
        }
    }
    feng::matrix<double> e;
    REQUIRE( e.load_npy( s5_r1::temp_file( "empty.npy", npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (0, 3), }", "" ) ) ) );
    REQUIRE( e.row() == 0 ); REQUIRE( e.col() == 3 );
}

TEST_CASE( "S5 load_npy rejects malformed headers and payload sizes", "[S5][S5-R1]" )
{
    using s5_r1::npy_bytes;
    using s5_r1::rejected;
    using s5_r1::six_doubles;
    auto const good = npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3), }", six_doubles );
    REQUIRE( !rejected( "good.npy", good ) );

    std::string bad_magic = good; bad_magic[1] = 'X';
    REQUIRE( rejected( "bad_magic.npy", bad_magic ) );
    std::string bad_version = good; bad_version[6] = 4;
    REQUIRE( rejected( "bad_version.npy", bad_version ) );
    std::string bad_minor = good; bad_minor[7] = 1;
    REQUIRE( rejected( "bad_minor.npy", bad_minor ) );
    REQUIRE( rejected( "only_magic.npy", good.substr( 0, 9 ) ) );
    REQUIRE( rejected( "cut_header.npy", good.substr( 0, 30 ) ) );
    std::string long_header = good; long_header[8] = '\xff'; long_header[9] = '\x7f';
    REQUIRE( rejected( "long_header.npy", long_header ) );
    REQUIRE( rejected( "short_payload.npy", good.substr( 0, good.size() - 1 ) ) );
    REQUIRE( rejected( "long_payload.npy", good + "x" ) );

    for ( std::string const& header : {
              std::string{ "{'descr': '<f8', 'fortran_order': False}" },                                   // missing key
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3), 'extra': 1}" },      // extra key
              std::string{ "{'descr': '<f8', 'descr': '<f8', 'fortran_order': False, 'shape': (2, 3)}" }, // duplicate key
              std::string{ "{'descr': '<f8', 'fortran_order': 0, 'shape': (2, 3)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': false, 'shape': (2, 3)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (-2, -3)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (+2, 3)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2x, 3)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (6)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (1, 2, 3)}" },
              std::string{ "{'descr': '|f8', 'fortran_order': False, 'shape': (2, 3)}" },
              std::string{ "{'descr': 'f8', 'fortran_order': False, 'shape': (2, 3)}" },
              std::string{ "{'descr': '<f8 ', 'fortran_order': False, 'shape': (2, 3)}" },
              std::string{ "{'descr': '<f8\", 'fortran_order': False, 'shape': (2, 3)}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3)} x" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3)" },
              std::string{ "'descr': '<f8', 'fortran_order': False, 'shape': (2, 3)}" },
              std::string{ "{'descr': '<f8',, 'fortran_order': False, 'shape': (2, 3)}" } } )
        REQUIRE( rejected( "bad_header.npy", npy_bytes( header, six_doubles ) ) );

    for ( char const* descr : { "<f2", "<f16", "|b1", "<M8", "|V8", "|S8", "<U2", "<i8" } )
    {
        auto const header = std::string{ "{'descr': '" } + descr + "', 'fortran_order': False, 'shape': (2, 3), }";
        REQUIRE( rejected( "bad_dtype.npy", npy_bytes( header, six_doubles ) ) );
    }

    feng::matrix<std::int8_t> i1;
    REQUIRE( i1.load_npy( s5_r1::temp_file( "i1.npy", npy_bytes( "{'descr': '|i1', 'fortran_order': False, 'shape': (1, 2), }", std::string{ "\x01\xff", 2 } ) ) ) );
    REQUIRE( i1.row() == 1 ); REQUIRE( i1.col() == 2 );
    REQUIRE( i1[0][0] == 1 ); REQUIRE( i1[0][1] == -1 );
}

// ---- S5-T7 (S5-R1): every fixture, every truncation, the dtype x element-type grid, malformed numbers ----
namespace s5_r1
{
    using all_element_types = std::tuple< std::int8_t, std::int16_t, std::int32_t, std::int64_t, std::uint8_t, std::uint16_t,
                                          std::uint32_t, std::uint64_t, float, double, std::complex< float >,
                                          std::complex< double >, long double >;

    template < typename T >
    feng::matrix< T > prefilled_of()
    {
        feng::matrix< T > m{ 2, 2 };
        for ( std::size_t k = 0; k != 4; ++k ) m.data()[k] = static_cast< T >( k + 1 );
        return m;
    }

    template < typename T >
    bool is_prefilled_of( feng::matrix< T > const& m )
    {
        if ( m.row() != 2 || m.col() != 2 ) return false;
        for ( std::size_t k = 0; k != 4; ++k )
            if ( !( m.data()[k] == static_cast< T >( k + 1 ) ) ) return false;
        return true;
    }

    inline std::vector< std::uint8_t > file_bytes( std::string const& path )
    {
        std::ifstream ifs( path, std::ios::binary );
        return std::vector< std::uint8_t >( std::istreambuf_iterator< char >( ifs ), std::istreambuf_iterator< char >() );
    }

    // In-memory parse of `bytes` into a pre-filled matrix<T>: 1 parsed (`loaded` receives the result), 0 failed with
    // the destination unchanged, -1 failed but changed the destination.
    template < typename T >
    int parses( std::string const& bytes, feng::matrix< T >& loaded )
    {
        std::vector< std::uint8_t > const buffer( bytes.begin(), bytes.end() ); // exact size, so ASan sees overreads
        auto m = prefilled_of< T >();
        if ( !feng::matrix_details::parse_npy< T >( buffer.data(), buffer.size(), m ) ) return is_prefilled_of( m ) ? 0 : -1;
        loaded = m;
        return 1;
    }

    // Loads fixture `name` into matrix<T> and compares it with numpy's values (row-major).
    template < typename T >
    bool loads_to( char const* name, std::size_t r, std::size_t c, std::vector< T > const& expected )
    {
        auto m = prefilled_of< T >();
        if ( !m.load_npy( fixture( name ) ) ) return false;
        if ( m.row() != r || m.col() != c || expected.size() != r * c ) return false;
        for ( std::size_t k = 0; k != expected.size(); ++k )
            if ( !( m.data()[k] == expected[k] ) ) return false;
        return true;
    }

    // True when fixture `name` fails, with the destination unchanged, for every element type except `Except`.
    template < typename Except = void >
    bool rejected_by_all_but( char const* name )
    {
        bool all = true;
        std::apply( [&]( auto... tag )
                    {
                        ( [&]( auto t )
                          {
                              using U = decltype( t );
                              if constexpr ( !std::is_same_v< U, Except > )
                              {
                                  auto m = prefilled_of< U >();
                                  if ( m.load_npy( fixture( name ) ) || !is_prefilled_of( m ) ) all = false;
                              }
                          }( tag ), ... );
                    },
                    all_element_types{} );
        return all;
    }

    template < typename T >
    constexpr char kind_of() noexcept
    {
        if constexpr ( std::is_same_v< T, float > || std::is_same_v< T, double > ) return 'f';
        else if constexpr ( std::is_same_v< T, std::complex< float > > || std::is_same_v< T, std::complex< double > > ) return 'c';
        else if constexpr ( std::is_integral_v< T > && std::is_signed_v< T > ) return 'i';
        else if constexpr ( std::is_integral_v< T > ) return 'u';
        else return '?'; // long double: no NPY dtype (D-021)
    }
}

TEST_CASE( "S5 load_npy loads every numpy fixture to numpy's values", "[S5][S5-R1]" )
{
    using namespace s5_r1;
    using cf = std::complex< float >;
    using cd = std::complex< double >;
    std::vector< double > const base{ 1.5, -2.25, 3.0, 4.125, 5.0, -6.5 };
    std::vector< float > const basef{ 1.5f, -2.25f, 3.0f, 4.125f, 5.0f, -6.5f };
    std::vector< std::int8_t > const i1{ -128, -1, 0, 1, 64, 127 };
    std::vector< std::int16_t > const i2{ -32768, -2, 0, 1, 300, 32767 };
    std::vector< std::int32_t > const i4{ 1, -2, 3, 70000, -80000, 2147483647 };
    std::vector< std::int64_t > const i8{ std::numeric_limits< std::int64_t >::min(), -1, 0, 1, 1099511627776LL, std::numeric_limits< std::int64_t >::max() };
    std::vector< std::uint8_t > const u1{ 0, 1, 255, 128, 7, 42 };
    std::vector< std::uint16_t > const u2{ 0, 1, 65535, 256, 4660, 65280 };
    std::vector< std::uint32_t > const u4{ 0, 1, 4294967295u, 65536, 305419896u, 4278190080u };
    std::vector< std::uint64_t > const u8{ 0, 1, 18446744073709551615ULL, 4294967296ULL, 0x0123456789ABCDEFULL, 0xFF00000000000000ULL };
    std::vector< cf > const c8{ { 1.0f, 2.0f }, { -3.5f, 0.25f }, { 0.0f, -1.0f }, { 7.0f, 0.0f } };
    std::vector< cd > const c16{ { 1.0, 2.0 }, { -3.5, 0.25 }, { 0.0, -1.0 }, { 7.0, 0.0 } };

    // Each loadable fixture loads to numpy's values into its element type and into no other.
    for ( char const* name : { "f8_le.npy", "f8_be.npy", "f8_v2.npy", "f8_v3.npy", "fortran_2x3.npy" } )
    {
        REQUIRE( loads_to< double >( name, 2, 3, base ) );
        REQUIRE( rejected_by_all_but< double >( name ) );
    }
    REQUIRE( loads_to< double >( "one_d.npy", 1, 3, { 10.0, 20.0, 30.0 } ) );
    REQUIRE( rejected_by_all_but< double >( "one_d.npy" ) );
    for ( char const* name : { "f4_le.npy", "f4_be.npy" } )
    {
        REQUIRE( loads_to< float >( name, 2, 3, basef ) );
        REQUIRE( rejected_by_all_but< float >( name ) );
    }
    REQUIRE( loads_to< std::int8_t >( "i1.npy", 2, 3, i1 ) );
    REQUIRE( rejected_by_all_but< std::int8_t >( "i1.npy" ) );
    for ( char const* name : { "i2_le.npy", "i2_be.npy" } )
    {
        REQUIRE( loads_to< std::int16_t >( name, 2, 3, i2 ) );
        REQUIRE( rejected_by_all_but< std::int16_t >( name ) );
    }
    for ( char const* name : { "i4_le.npy", "i4_be.npy" } )
    {
        REQUIRE( loads_to< std::int32_t >( name, 2, 3, i4 ) );
        REQUIRE( rejected_by_all_but< std::int32_t >( name ) );
    }
    for ( char const* name : { "i8_le.npy", "i8_be.npy" } )
    {
        REQUIRE( loads_to< std::int64_t >( name, 2, 3, i8 ) );
        REQUIRE( rejected_by_all_but< std::int64_t >( name ) );
    }
    REQUIRE( loads_to< std::uint8_t >( "u1.npy", 2, 3, u1 ) );
    REQUIRE( rejected_by_all_but< std::uint8_t >( "u1.npy" ) );
    for ( char const* name : { "u2_le.npy", "u2_be.npy" } )
    {
        REQUIRE( loads_to< std::uint16_t >( name, 2, 3, u2 ) );
        REQUIRE( rejected_by_all_but< std::uint16_t >( name ) );
    }
    for ( char const* name : { "u4_le.npy", "u4_be.npy" } )
    {
        REQUIRE( loads_to< std::uint32_t >( name, 2, 3, u4 ) );
        REQUIRE( rejected_by_all_but< std::uint32_t >( name ) );
    }
    for ( char const* name : { "u8_le.npy", "u8_be.npy" } )
    {
        REQUIRE( loads_to< std::uint64_t >( name, 2, 3, u8 ) );
        REQUIRE( rejected_by_all_but< std::uint64_t >( name ) );
    }
    for ( char const* name : { "c8_le.npy", "c8_be.npy" } )
    {
        REQUIRE( loads_to< cf >( name, 2, 2, c8 ) );
        REQUIRE( rejected_by_all_but< cf >( name ) );
    }
    for ( char const* name : { "c16_le.npy", "c16_be.npy" } )
    {
        REQUIRE( loads_to< cd >( name, 2, 2, c16 ) );
        REQUIRE( rejected_by_all_but< cd >( name ) );
    }
    // 3-D, object, structured, bool and f2 fixtures fail for every element type.
    for ( char const* name : { "three_d.npy", "object.npy", "structured.npy", "bool.npy", "f2.npy" } )
        REQUIRE( rejected_by_all_but( name ) );
}

TEST_CASE( "S5 load_npy fails at every truncation of a valid v1.0 and v2.0 file", "[S5][S5-R1]" )
{
    using namespace s5_r1;
    for ( char const* name : { "f8_le.npy", "f8_v2.npy" } )
    {
        auto const bytes = file_bytes( fixture( name ) );
        REQUIRE( bytes.size() == 176 );
        {
            auto whole = prefilled_of< double >();
            REQUIRE( feng::matrix_details::parse_npy< double >( bytes.data(), bytes.size(), whole ) );
        }
        for ( std::size_t n = 0; n != bytes.size(); ++n )
        {
            std::vector< std::uint8_t > const prefix( bytes.begin(), bytes.begin() + static_cast< std::ptrdiff_t >( n ) );
            auto m = prefilled_of< double >();
            INFO( name << " cut at " << n );
            REQUIRE( !feng::matrix_details::parse_npy< double >( prefix.data(), prefix.size(), m ) );
            REQUIRE( is_prefilled_of( m ) );
        }
    }
    // The file loader agrees for a few cuts (empty file, inside the magic, the header and the payload).
    for ( std::size_t n : { std::size_t{ 0 }, std::size_t{ 5 }, std::size_t{ 60 }, std::size_t{ 175 } } )
    {
        auto const bytes = file_bytes( fixture( "f8_v2.npy" ) );
        REQUIRE( rejected( "cut_v2.npy", std::string( bytes.begin(), bytes.begin() + static_cast< std::ptrdiff_t >( n ) ) ) );
    }
}

TEST_CASE( "S5 load_npy loads a dtype only into the element type of the same kind and size", "[S5][S5-R1]" )
{
    using namespace s5_r1;
    struct dtype { char kind; std::size_t size; };
    dtype const dtypes[] = { { 'i', 1 }, { 'i', 2 }, { 'i', 4 }, { 'i', 8 }, { 'u', 1 }, { 'u', 2 }, { 'u', 4 }, { 'u', 8 },
                             { 'f', 4 }, { 'f', 8 }, { 'c', 8 }, { 'c', 16 } };
    std::size_t accepted = 0;
    for ( char const order : { '<', '>', '=', '|' } )
        for ( dtype const d : dtypes )
        {
            std::string const descr = std::string{ order, d.kind } + std::to_string( d.size );
            std::string payload( 2 * d.size, '\0' );
            for ( std::size_t k = 0; k != payload.size(); ++k ) payload[k] = static_cast< char >( k + 1 );
            auto const bytes = npy_bytes( "{'descr': '" + descr + "', 'fortran_order': False, 'shape': (1, 2), }", payload );
            bool const foreign = ( order == '>' && std::endian::native == std::endian::little )
                              || ( order == '<' && std::endian::native == std::endian::big );
            std::apply( [&]( auto... tag )
                        {
                            ( [&]( auto t )
                              {
                                  using T = decltype( t );
                                  bool const expect = kind_of< T >() == d.kind && sizeof( T ) == d.size && ( order != '|' || d.size == 1 );
                                  feng::matrix< T > loaded;
                                  int const result = parses< T >( bytes, loaded );
                                  INFO( descr << " into an element of " << sizeof( T ) << " bytes, kind " << kind_of< T >() );
                                  REQUIRE( result == ( expect ? 1 : 0 ) );
                                  if ( result != 1 ) return;
                                  ++accepted;
                                  REQUIRE( loaded.row() == 1 );
                                  REQUIRE( loaded.col() == 2 );
                                  // Foreign order reverses each component (each half of a complex element).
                                  std::string want = payload;
                                  std::size_t const unit = d.kind == 'c' ? d.size / 2 : d.size;
                                  if ( foreign )
                                      for ( std::size_t u = 0; u != want.size(); u += unit )
                                          std::reverse( want.begin() + static_cast< std::ptrdiff_t >( u ),
                                                        want.begin() + static_cast< std::ptrdiff_t >( u + unit ) );
                                  REQUIRE( std::memcmp( loaded.data(), want.data(), want.size() ) == 0 );
                              }( tag ), ... );
                        },
                        all_element_types{} );
        }
    REQUIRE( accepted == 12 * 3 + 2 ); // '<', '>' and '=' for every dtype; '|' only for i1 and u1
}

TEST_CASE( "S5 load_npy rejects malformed dicts and numbers with leading zeros, signs or overflow", "[S5][S5-R1]" )
{
    using s5_r1::npy_bytes;
    using s5_r1::rejected;
    using s5_r1::six_doubles;
    auto const header = []( std::string const& descr, std::string const& shape )
    { return "{'descr': '" + descr + "', 'fortran_order': False, 'shape': " + shape + ", }"; };
    REQUIRE( !rejected( "control.npy", npy_bytes( header( "<f8", "(2, 3)" ), six_doubles ) ) );
    REQUIRE( !rejected( "control_zero.npy", npy_bytes( header( "<f8", "(0, 3)" ), "" ) ) );
    for ( std::string const& h : {
              header( "<f08", "(2, 3)" ), header( "<f008", "(2, 3)" ), header( "<f8", "(02, 3)" ), header( "<f8", "(2, 03)" ),
              header( "<f8", "(006,)" ), header( "<f8", "(-2, 3)" ), header( "<f8", "(2, -3)" ), header( "<f-8", "(2, 3)" ),
              header( "<f+8", "(2, 3)" ), header( "<f8", "(two, 3)" ), header( "<f8", "(2.0, 3)" ), header( "<f8", "(0x2, 3)" ),
              header( "<f8", "(2e0, 3)" ), header( "<fx", "(2, 3)" ), header( "<f8.0", "(2, 3)" ), header( "<f", "(2, 3)" ),
              header( "<f8", "(18446744073709551616, 1)" ), header( "<f18446744073709551624", "(2, 3)" ),
              header( "<f8", "(2, 3L)" ), header( "<f8", "(2 3)" ), header( "<f8", "(,)" ), header( "<f8", "(2,, 3)" ) } )
    {
        INFO( h );
        REQUIRE( rejected( "bad_number.npy", npy_bytes( h, six_doubles ) ) );
    }
    feng::matrix< std::int32_t > i;
    REQUIRE( !i.load_npy( s5_r1::temp_file( "i04.npy", npy_bytes( header( "<i04", "(1, 1)" ), std::string( 4, '\0' ) ) ) ) );
    REQUIRE( !i.load_npy( s5_r1::temp_file( "i01.npy", npy_bytes( header( "<i4", "(01, 1)" ), std::string( 4, '\0' ) ) ) ) );

    for ( std::string const& h : {
              std::string{ "{'fortran_order': False, 'shape': (2, 3), }" },                                 // missing descr
              std::string{ "{'descr': '<f8', 'shape': (2, 3), }" },                                         // missing fortran_order
              std::string{ "{'descr': '<f8', 'fortran_order': False, }" },                                  // missing shape
              std::string{ "{}" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3), 'version': 1, }" },   // extra key
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'Shape': (2, 3), }" },
              std::string{ "{'descr\": '<f8', 'fortran_order': False, 'shape': (2, 3), }" },                // bad quotes
              std::string{ "{descr: '<f8', 'fortran_order': False, 'shape': (2, 3), }" },
              std::string{ "{'descr': <f8, 'fortran_order': False, 'shape': (2, 3), }" },
              std::string{ "{'descr': '<f8', 'fortran_order': 'False', 'shape': (2, 3), }" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': '(2, 3)', }" },
              std::string{ "{'descr': b'<f8', 'fortran_order': False, 'shape': (2, 3), }" },
              std::string{ "{'descr': '<f8, 'fortran_order': False, 'shape': (2, 3), }" },                  // unterminated
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3" },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3)," },
              std::string{ "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3), } }" },
              std::string{ "{'descr': '<f8' 'fortran_order': False, 'shape': (2, 3), }" },                  // missing comma
              std::string{ "{'descr' '<f8', 'fortran_order': False, 'shape': (2, 3), }" } } )               // missing colon
    {
        INFO( h );
        REQUIRE( rejected( "bad_dict.npy", npy_bytes( h, six_doubles ) ) );
    }
}

TEST_CASE( "S5 matrix.hpp calls no throwing std::sto* conversion", "[S5][S5-R1]" )
{
    std::ifstream ifs( "./matrix.hpp", std::ios::binary );
    REQUIRE( ifs.good() );
    std::string const text{ std::istreambuf_iterator< char >( ifs ), std::istreambuf_iterator< char >() };
    REQUIRE( text.size() > 100000 );
    std::string hits;
    for ( std::size_t p = text.find( "std::sto" ); p != std::string::npos; p = text.find( "std::sto", p + 1 ) )
    {
        std::size_t e = p + 8;
        while ( e != text.size() && ( std::isalnum( static_cast< unsigned char >( text[e] ) ) || text[e] == '_' ) ) ++e;
        std::string const name = text.substr( p + 5, e - p - 5 );
        for ( char const* banned : { "stoi", "stol", "stoll", "stoul", "stoull", "stof", "stod", "stold" } )
            if ( name == banned ) hits += "std::" + name + " at offset " + std::to_string( p ) + "; ";
    }
    INFO( hits );
    REQUIRE( hits.empty() );
}
