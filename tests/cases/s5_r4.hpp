// S5-R4 (PR-2, PR-7, D-011): every file writer returns false with one stderr line, and never aborts, on a directory,
// open, write or close failure, and true on success; D-022: save_as_npy writes a v1.0 header numpy reads and that
// load_npy loads back. S5-R3: the free three-channel save_as_bmp loads back to its channels. Temporary files go under
// std::filesystem::temp_directory_path().
#include <bit>
#include <complex>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h> // getpid: per-process temp names

namespace s5_r4
{
    // The process id keeps two suite binaries run by hand at once from sharing a temp path.
    inline std::string temp_path( std::string const& name )
    {
        return ( std::filesystem::temp_directory_path() / ( "feng_s5_r4_" + std::to_string( ::getpid() ) + "_" + name ) ).string();
    }

    inline std::vector<char> file_bytes( std::string const& path )
    {
        std::ifstream ifs( path, std::ios::binary );
        return std::vector<char>{ std::istreambuf_iterator<char>( ifs ), std::istreambuf_iterator<char>() };
    }

    // Redirects std::cerr into a string for the lifetime of the object.
    struct capture_cerr
    {
        std::ostringstream text;
        std::streambuf* old;
        capture_cerr() : old( std::cerr.rdbuf( text.rdbuf() ) ) {}
        ~capture_cerr() { std::cerr.rdbuf( old ); }
        std::string str() const { return text.str(); }
    };

    struct writer
    {
        char const* name;      // what the stderr line must name
        char const* extension; // the writer's file extension
        std::function<bool( std::string const& )> call;
    };

    inline feng::matrix<double> const& sample()
    {
        static feng::matrix<double> const m = []
        {
            feng::matrix<double> x{ 3, 4 };
            for ( std::size_t k = 0; k != x.size(); ++k ) x.data()[k] = static_cast<double>( ( k * 5 ) % 7 ) - 1.5;
            return x;
        }();
        return m;
    }

    // Every writer overload: member txt, binary, bmp, png, pgm, npy (string and char const*), and the free bmp ones.
    inline std::vector<writer> writers()
    {
        auto const& m = sample();
        return {
            { "save_as_txt", ".txt", [&m]( std::string const& p ) { return m.save_as_txt( p ); } },
            { "save_as_txt", ".txt", [&m]( std::string const& p ) { return m.save_as_txt( p.c_str() ); } },
            { "save_as_binary", ".bin", [&m]( std::string const& p ) { return m.save_as_binary( p ); } },
            { "save_as_binary", ".bin", [&m]( std::string const& p ) { return m.save_as_binary( p.c_str() ); } },
            { "save_as_bmp", ".bmp", [&m]( std::string const& p ) { return m.save_as_bmp( p ); } },
            { "save_as_bmp", ".bmp", [&m]( std::string const& p ) { return m.save_as_bmp( p.c_str() ); } },
            { "save_as_png", ".png", [&m]( std::string const& p ) { return m.save_as_png( p ); } },
            { "save_as_pgm", ".pgm", [&m]( std::string const& p ) { return m.save_as_pgm( p ); } },
            { "save_as_pgm", ".pgm", [&m]( std::string const& p ) { return m.save_as_pgm( p.c_str() ); } },
            { "save_as_npy", ".npy", [&m]( std::string const& p ) { return m.save_as_npy( p ); } },
            { "save_as_npy", ".npy", [&m]( std::string const& p ) { return m.save_as_npy( p.c_str() ); } },
            { "save_as_bmp", ".bmp", [&m]( std::string const& p ) { return feng::save_as_bmp( p, m, m, m ); } },
            { "save_as_bmp", ".bmp", [&m]( std::string const& p ) { return feng::save_as_bmp( p, m, std::string{ "jet" } ); } },
        };
    }

    inline void require_failure( writer const& w, std::string const& path )
    {
        INFO( w.name << " into " << path );
        capture_cerr cap;
        bool const ok = w.call( path );
        std::string const text = cap.str();
        REQUIRE( !ok );
        REQUIRE( text.find( w.name ) != std::string::npos );
        REQUIRE( text.find( path ) != std::string::npos );
        REQUIRE( text.find( '\n' ) == text.size() - 1 ); // one line
    }
} // namespace s5_r4

TEST_CASE( "S5 save_as_png into a missing directory returns false", "[S5][S5-R4]" )
{
    s5_r4::capture_cerr cap;
    bool const ok = s5_r4::sample().save_as_png( "/nonexistent-dir/x.png" );
    std::string const text = cap.str();
    REQUIRE( !ok );
    REQUIRE( text.find( "save_as_png" ) != std::string::npos );
    REQUIRE( text.find( "/nonexistent-dir/x.png" ) != std::string::npos );
}

TEST_CASE( "S5 every writer reports a write failure on /dev/full", "[S5][S5-R4]" )
{
    // write or close failure: a symlink to /dev/full opens but every write fails with ENOSPC
    // the write-failure path must be exercised: a host without /dev/full fails this test rather than skipping it
    REQUIRE( std::filesystem::exists( "/dev/full" ) );
    for ( auto const& w : s5_r4::writers() )
    {
        auto const link = s5_r4::temp_path( std::string{ "full" } + w.extension );
        std::filesystem::remove( link );
        std::filesystem::create_symlink( "/dev/full", link );
        s5_r4::require_failure( w, link );
        std::filesystem::remove( link );
    }

    // open and directory failures: the parent path is a regular file
    auto const plain = s5_r4::temp_path( "plain_file" );
    std::filesystem::remove_all( plain );
    {
        std::ofstream ofs( plain );
        ofs << "not a directory\n";
    }
    for ( auto const& w : s5_r4::writers() )
    {
        s5_r4::require_failure( w, plain + "/x" + w.extension );          // open failure
        s5_r4::require_failure( w, plain + "/sub/x" + w.extension );      // directory failure
    }
    std::filesystem::remove( plain );

    // directory failure: the parent directory does not exist and cannot be created
    for ( auto const& w : s5_r4::writers() )
        s5_r4::require_failure( w, std::string{ "/nonexistent-dir/x" } + w.extension );
}

TEST_CASE( "S5 every writer returns true on success", "[S5][S5-R4]" )
{
    int k = 0;
    for ( auto const& w : s5_r4::writers() )
    {
        auto const path = s5_r4::temp_path( "ok_" + std::to_string( k++ ) + w.extension );
        std::filesystem::remove( path );
        INFO( w.name << " into " << path );
        REQUIRE( w.call( path ) );
        REQUIRE( std::filesystem::file_size( path ) > 0 );
        std::filesystem::remove( path );
    }
}

namespace s5_r4
{
    template < typename T >
    void npy_round_trip( feng::matrix<T> const& a, std::string const& name )
    {
        INFO( name );
        auto const path = temp_path( name );
        REQUIRE( a.save_as_npy( path ) );
        auto const bytes = file_bytes( path );
        REQUIRE( bytes.size() >= 10 );
        std::size_t const header_len = static_cast<unsigned char>( bytes[8] ) | ( static_cast<std::size_t>( static_cast<unsigned char>( bytes[9] ) ) << 8 );
        REQUIRE( ( 10 + header_len ) % 64 == 0 );
        REQUIRE( bytes[9 + header_len] == '\n' );
        REQUIRE( bytes.size() == 10 + header_len + sizeof( T ) * a.size() );
        feng::matrix<T> b{ 1, 1 };
        REQUIRE( b.load_npy( path ) );
        REQUIRE( b.row() == a.row() );
        REQUIRE( b.col() == a.col() );
        for ( std::size_t k = 0; k != a.size(); ++k ) REQUIRE( b.data()[k] == a.data()[k] );
        std::filesystem::remove( path );
    }
} // namespace s5_r4

TEST_CASE( "S5 save_as_npy output loads back through load_npy", "[S5][S5-R4]" )
{
    feng::matrix<double> d{ 2, 3 };
    feng::matrix<float> f{ 3, 2 };
    feng::matrix<std::int32_t> i{ 2, 4 };
    feng::matrix<std::complex<double>> c{ 2, 2 };
    for ( std::size_t k = 0; k != d.size(); ++k ) d.data()[k] = 1.0 / ( 1.0 + static_cast<double>( k ) ) - 0.3;
    for ( std::size_t k = 0; k != f.size(); ++k ) f.data()[k] = static_cast<float>( k ) * -1.25f + 0.1f;
    for ( std::size_t k = 0; k != i.size(); ++k ) i.data()[k] = static_cast<std::int32_t>( k * 70001 ) - 2147483647;
    for ( std::size_t k = 0; k != c.size(); ++k ) c.data()[k] = std::complex<double>{ 0.5 * static_cast<double>( k ), -1.0 / ( 1.0 + static_cast<double>( k ) ) };
    s5_r4::npy_round_trip( d, "rt_f8.npy" );
    s5_r4::npy_round_trip( f, "rt_f4.npy" );
    s5_r4::npy_round_trip( i, "rt_i4.npy" );
    s5_r4::npy_round_trip( c, "rt_c16.npy" );
    feng::matrix<std::int32_t> empty{ 0, 3 };
    s5_r4::npy_round_trip( empty, "rt_empty.npy" );
}

TEST_CASE( "S5 save_as_npy writes the bytes numpy writes", "[S5][S5-R4]" )
{
    feng::matrix<double> base{ 2, 3 };
    double const values[] = { 1.5, -2.25, 3.0, 4.125, 5.0, -6.5 };
    std::copy( std::begin( values ), std::end( values ), base.data() );
    auto const path = s5_r4::temp_path( "numpy_f8.npy" );
    REQUIRE( base.save_as_npy( path ) );
    auto const ours = s5_r4::file_bytes( path );
    auto const numpys = s5_r4::file_bytes( "./tests/fixtures/s5/f8_le.npy" );
    REQUIRE( numpys.size() == 128 + 6 * 8 );
    REQUIRE( ours.size() == numpys.size() );
    std::size_t const compare = std::endian::native == std::endian::little ? numpys.size() : 128; // payload is native order
    for ( std::size_t k = 0; k != compare; ++k )
    {
        if ( k == 18 && std::endian::native != std::endian::little ) continue; // the '<' of the descr
        INFO( "byte " << k );
        REQUIRE( ours[k] == numpys[k] );
    }
    std::filesystem::remove( path );

    feng::matrix<std::uint8_t> u{ 1, 2, std::uint8_t{ 7 } };
    REQUIRE( u.save_as_npy( path ) );
    auto const ub = s5_r4::file_bytes( path );
    std::string const header( ub.begin() + 10, ub.begin() + 10 + 60 );
    REQUIRE( header.find( "{'descr': '|u1', 'fortran_order': False, 'shape': (1, 2), }" ) == 0 );
    std::filesystem::remove( path );
}

TEST_CASE( "S5 a free three-channel save_as_bmp file loads back to its channels", "[S5][S5-R3]" )
{
    // channels with minimum 0 and maximum 255 are written unscaled, so load_bmp must return them exactly
    feng::matrix<double> r{ 3, 4 }, g{ 3, 4 }, b{ 3, 4 };
    for ( std::size_t k = 0; k != r.size(); ++k )
    {
        r.data()[k] = static_cast<double>( k * 23 );
        g.data()[k] = static_cast<double>( ( 11 - k ) * 19 );
        b.data()[k] = static_cast<double>( ( k * 7 ) % 12 * 21 );
    }
    for ( auto* m : { &r, &g, &b } )
    {
        ( *m )[0][1] = 0.0;
        ( *m )[2][2] = 255.0;
    }
    auto const path = s5_r4::temp_path( "rgb.bmp" );
    REQUIRE( feng::save_as_bmp( path, r, g, b ) );
    auto const img = feng::load_bmp( path );
    REQUIRE( img );
    for ( auto const& channel : *img ) { REQUIRE( channel.row() == 3 ); REQUIRE( channel.col() == 4 ); }
    for ( std::size_t y = 0; y != 3; ++y )
        for ( std::size_t x = 0; x != 4; ++x )
        {
            INFO( "pixel " << y << ", " << x );
            REQUIRE( ( *img )[0][y][x] == static_cast<std::uint8_t>( r[y][x] ) );
            REQUIRE( ( *img )[1][y][x] == static_cast<std::uint8_t>( g[y][x] ) );
            REQUIRE( ( *img )[2][y][x] == static_cast<std::uint8_t>( b[y][x] ) );
        }
    std::filesystem::remove( path );
}

namespace s5_r4
{
    inline std::string bytes_file( std::string const& name, std::string const& bytes )
    {
        auto const path = temp_path( name );
        std::ofstream ofs( path, std::ios::binary | std::ios::trunc );
        ofs.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) );
        return path;
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

    // One stderr line naming the loader.
    inline bool one_line_naming( std::string const& text, char const* loader )
    {
        return text.find( loader ) != std::string::npos && !text.empty() && text.find( '\n' ) == text.size() - 1;
    }
} // namespace s5_r4

TEST_CASE( "S5 every loader fails on every failure class and keeps a pre-filled destination", "[S5][S5-R4]" )
{
    using size_type = feng::matrix<double>::size_type;
    std::string short_binary( 2 * sizeof( size_type ) + sizeof( double ), '\0' ); // declares 2 x 3, holds one element
    size_type const two = 2, three = 3;
    std::memcpy( short_binary.data(), &two, sizeof( two ) );
    std::memcpy( short_binary.data() + sizeof( two ), &three, sizeof( three ) );
    std::string npy_f4 = std::string( "\x93NUMPY\x01\x00", 8 ) + '\x76' + '\x00' + "{'descr': '<f4', 'fortran_order': False, 'shape': (1, 1), }";
    npy_f4.resize( 128 - 1, ' ' );
    npy_f4 += '\n';
    npy_f4 += std::string( 4, '\0' ); // a '<f4' file never loads into a matrix<double>
    {
        // the same bytes are a well-formed '<f4' file: they load into a matrix<float>, so the double rejection is the dtype
        feng::matrix<float> f{ 2, 2 };
        REQUIRE( npy_f4.size() == 128 + 4 );
        REQUIRE( f.load_npy( s5_r4::bytes_file( "f4_ok.npy", npy_f4 ) ) );
        REQUIRE( f.row() == 1 );
        REQUIRE( f.col() == 1 );
        REQUIRE( f[0][0] == 0.0f );
    }
    std::string bmp_rle( 58, '\0' );   // a 1 x 1 BMP with compression 1 (RLE8)
    bmp_rle[0] = 'B'; bmp_rle[1] = 'M'; bmp_rle[10] = 54; bmp_rle[14] = 40; bmp_rle[18] = 1; bmp_rle[22] = 1;
    bmp_rle[26] = 1; bmp_rle[28] = 24; bmp_rle[30] = 1;

    auto const missing = s5_r4::temp_path( "missing_input" );
    std::filesystem::remove_all( missing );
    auto const directory = std::filesystem::temp_directory_path().string();
    struct failure { char const* what; std::string txt, bin, npy, bmp; };
    std::vector<failure> const failures{
        { "missing file", missing, missing, missing, missing },
        { "directory", directory, directory, directory, directory },
        { "empty file", s5_r4::bytes_file( "empty.txt", "" ), s5_r4::bytes_file( "empty.bin", "" ), s5_r4::bytes_file( "empty.npy", "" ), s5_r4::bytes_file( "empty.bmp", "" ) },
        { "3-byte file", s5_r4::bytes_file( "three.txt", "abc" ), s5_r4::bytes_file( "three.bin", "abc" ), s5_r4::bytes_file( "three.npy", "abc" ), s5_r4::bytes_file( "three.bmp", "BMa" ) },
        { "rejected input", s5_r4::bytes_file( "ragged.txt", "1 2\n3\n" ), s5_r4::bytes_file( "short.bin", short_binary ), s5_r4::bytes_file( "f4.npy", npy_f4 ),
          s5_r4::bytes_file( "rle.bmp", bmp_rle ) },
    };

    for ( auto const& f : failures )
    {
        INFO( f.what );
        {
            auto m = s5_r4::prefilled();
            s5_r4::capture_cerr cap;
            bool const ok = m.load_txt( f.txt );
            std::string const text = cap.str();
            REQUIRE( !ok );
            REQUIRE( s5_r4::is_prefilled( m ) );
            REQUIRE( s5_r4::one_line_naming( text, "load_txt" ) );
        }
        {
            auto m = s5_r4::prefilled();
            s5_r4::capture_cerr cap;
            bool const ok = m.load_binary( f.bin );
            std::string const text = cap.str();
            REQUIRE( !ok );
            REQUIRE( s5_r4::is_prefilled( m ) );
            REQUIRE( s5_r4::one_line_naming( text, "load_binary" ) );
        }
        {
            auto m = s5_r4::prefilled();
            s5_r4::capture_cerr cap;
            bool const ok = m.load_npy( f.npy );
            std::string const text = cap.str();
            REQUIRE( !ok );
            REQUIRE( s5_r4::is_prefilled( m ) );
            REQUIRE( s5_r4::one_line_naming( text, "load_npy" ) );
        }
        {
            // operator>> reports through the stream state (failbit), not stderr
            auto m = s5_r4::prefilled();
            std::ifstream ifs( f.txt );
            ifs >> m;
            REQUIRE( ifs.fail() );
            REQUIRE( s5_r4::is_prefilled( m ) );
        }
        {
            s5_r4::capture_cerr cap;
            auto const img = feng::load_bmp( f.bmp );
            std::string const text = cap.str();
            REQUIRE( !img );
            REQUIRE( s5_r4::one_line_naming( text, "load_bmp" ) );
        }
    }
}
