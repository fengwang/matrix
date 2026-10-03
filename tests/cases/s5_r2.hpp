// S5-R2 (PR-7, F08): text and native binary inputs are bounded and typed; S5-R4 (PR-2, D-011): their loaders are
// transactional. Temporary files go under std::filesystem::temp_directory_path().
#include <algorithm>
#include <complex>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h> // getpid: per-process temp names

namespace s5_r2
{
    // The process id keeps two suite binaries run by hand at once from sharing a temp path.
    inline std::string temp_path( std::string const& name )
    {
        return ( std::filesystem::temp_directory_path() / ( "feng_s5_r2_" + std::to_string( ::getpid() ) + "_" + name ) ).string();
    }

    inline std::string temp_file( std::string const& name, std::string const& bytes )
    {
        auto const path = temp_path( name );
        std::ofstream ofs( path, std::ios::binary | std::ios::trunc );
        ofs.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) );
        ofs.close();
        return path;
    }

    template < typename T = double >
    feng::matrix<T> prefilled()
    {
        feng::matrix<T> m{ 2, 2 };
        m[0][0] = T{ 1 }; m[0][1] = T{ 2 }; m[1][0] = T{ 3 }; m[1][1] = T{ 4 };
        return m;
    }

    template < typename T = double >
    bool is_prefilled( feng::matrix<T> const& m )
    {
        return m.row() == 2 && m.col() == 2 && m[0][0] == T{ 1 } && m[0][1] == T{ 2 } && m[1][0] == T{ 3 } && m[1][1] == T{ 4 };
    }

    // Loads the text into a pre-filled 2x2 matrix<T>; true when the load failed and left it unchanged.
    template < typename T = double >
    bool txt_rejected( std::string const& name, std::string const& text )
    {
        auto m = prefilled<T>();
        bool const ok = m.load_txt( temp_file( name, text ) );
        return !ok && is_prefilled<T>( m );
    }

    template < typename T = double >
    bool bin_rejected( std::string const& name, std::string const& bytes )
    {
        auto m = prefilled<T>();
        bool const ok = m.load_binary( temp_file( name, bytes ) );
        return !ok && is_prefilled<T>( m );
    }

    // A native binary image: two size_type counts, then the raw elements.
    template < typename T >
    std::string binary_bytes( std::uint64_t r, std::uint64_t c, std::vector<T> const& v )
    {
        using size_type = typename feng::matrix<T>::size_type;
        size_type const rr = static_cast<size_type>( r );
        size_type const cc = static_cast<size_type>( c );
        std::string s( 2 * sizeof( size_type ) + v.size() * sizeof( T ), '\0' );
        std::memcpy( s.data(), &rr, sizeof( rr ) );
        std::memcpy( s.data() + sizeof( rr ), &cc, sizeof( cc ) );
        if ( !v.empty() ) std::memcpy( s.data() + 2 * sizeof( size_type ), v.data(), v.size() * sizeof( T ) );
        return s;
    }

    template < typename T >
    bool same( feng::matrix<T> const& a, feng::matrix<T> const& b )
    {
        return a.row() == b.row() && a.col() == b.col() && std::equal( a.begin(), a.end(), b.begin() );
    }
}

TEST_CASE( "S5 load_txt rejects empty and ragged text and keeps the destination", "[S5][S5-R2][S5-R4]" )
{
    REQUIRE( s5_r2::txt_rejected( "empty.txt", "" ) );
    REQUIRE( s5_r2::txt_rejected( "blank.txt", "  \n\t\r\n,;\n\n" ) );
    REQUIRE( s5_r2::txt_rejected( "ragged.txt", "1 2 3\n4 5\n" ) );
    REQUIRE( s5_r2::txt_rejected( "ragged2.txt", "1\n2 3\n" ) );

    auto m = s5_r2::prefilled();
    std::ostringstream captured;
    auto* const old = std::cerr.rdbuf( captured.rdbuf() );
    bool const ok = m.load_txt( s5_r2::temp_file( "ragged3.txt", "1,2\n3\n" ) );
    std::cerr.rdbuf( old );
    REQUIRE( !ok );
    REQUIRE( s5_r2::is_prefilled( m ) );
    std::string const text = captured.str();
    REQUIRE( text.find( "load_txt" ) != std::string::npos );
    REQUIRE( std::count( text.begin(), text.end(), '\n' ) == 1 );

    auto const missing = s5_r2::temp_path( "does_not_exist.txt" );
    std::filesystem::remove( missing );
    REQUIRE( !m.load_txt( missing ) );
    REQUIRE( !m.load_txt( std::filesystem::temp_directory_path().string() ) );
    REQUIRE( s5_r2::is_prefilled( m ) );
}

TEST_CASE( "S5 load_txt rejects truncated and oversized tokens", "[S5][S5-R2][S5-R4]" )
{
    REQUIRE( s5_r2::txt_rejected( "trunc_exp.txt", "1 2\n3 1.5e\n" ) );
    REQUIRE( s5_r2::txt_rejected( "junk.txt", "1 2\n3 4x\n" ) );
    REQUIRE( s5_r2::txt_rejected( "double_plus.txt", "1 ++2\n" ) );
    REQUIRE( s5_r2::txt_rejected( "plus_minus.txt", "1 +-2\n" ) );
    REQUIRE( s5_r2::txt_rejected( "huge_double.txt", "1 1e999\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::uint8_t>( "u8_300.txt", "1 300\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::uint8_t>( "u8_neg.txt", "1 -1\n" ) );
    REQUIRE( s5_r2::txt_rejected<int>( "int_fraction.txt", "1 2.5\n" ) );
    REQUIRE( s5_r2::txt_rejected<int>( "int_huge.txt", "1 99999999999999999999\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::int8_t>( "i8_128.txt", "128\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::complex<double>>( "cx_open.txt", "(1,2 3\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::complex<double>>( "cx_one.txt", "(1)\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::complex<double>>( "cx_three.txt", "(1,2,3)\n" ) );
}

TEST_CASE( "S5 load_txt accepts separators, blank lines, CRLF, a plus sign and complex pairs", "[S5][S5-R2]" )
{
    feng::matrix<double> m;
    REQUIRE( m.load_txt( s5_r2::temp_file( "seps.txt", "\n1,2;3\r\n  \n+4\t5 -6.25e1\r\n\n" ) ) );
    REQUIRE( m.row() == 2 ); REQUIRE( m.col() == 3 );
    REQUIRE( m[0][0] == 1.0 ); REQUIRE( m[0][1] == 2.0 ); REQUIRE( m[0][2] == 3.0 );
    REQUIRE( m[1][0] == 4.0 ); REQUIRE( m[1][1] == 5.0 ); REQUIRE( m[1][2] == -62.5 );

    feng::matrix<std::uint8_t> u;
    REQUIRE( u.load_txt( s5_r2::temp_file( "u8.txt", "0 255\n+7 32" ) ) );
    REQUIRE( u.row() == 2 ); REQUIRE( u.col() == 2 );
    REQUIRE( u[0][0] == 0 ); REQUIRE( u[0][1] == 255 ); REQUIRE( u[1][0] == 7 ); REQUIRE( u[1][1] == 32 );

    feng::matrix<std::complex<double>> z;
    REQUIRE( z.load_txt( s5_r2::temp_file( "cx.txt", "(1,2) 3\n(-1.5,+0.5),(4, 0)\n" ) ) );
    REQUIRE( z.row() == 2 ); REQUIRE( z.col() == 2 );
    REQUIRE( z[0][0] == std::complex<double>( 1, 2 ) ); REQUIRE( z[0][1] == std::complex<double>( 3, 0 ) );
    REQUIRE( z[1][0] == std::complex<double>( -1.5, 0.5 ) ); REQUIRE( z[1][1] == std::complex<double>( 4, 0 ) );
}

TEST_CASE( "S5 save_as_txt output loads back for double, int and uint8_t", "[S5][S5-R2]" )
{
    {
        feng::matrix<double> a{ 2, 3 };
        a[0][0] = 0.1; a[0][1] = 1.0 / 3.0; a[0][2] = -2.5e-300;
        a[1][0] = 1.0e300; a[1][1] = std::numeric_limits<double>::min(); a[1][2] = -0.0;
        auto const path = s5_r2::temp_path( "rt_double.txt" );
        REQUIRE( a.save_as_txt( path ) );
        feng::matrix<double> b;
        REQUIRE( b.load_txt( path ) );
        REQUIRE( s5_r2::same( a, b ) );
    }
    {
        feng::matrix<int> a{ 2, 2 };
        a[0][0] = std::numeric_limits<int>::min(); a[0][1] = std::numeric_limits<int>::max(); a[1][0] = 0; a[1][1] = -7;
        auto const path = s5_r2::temp_path( "rt_int.txt" );
        REQUIRE( a.save_as_txt( path ) );
        feng::matrix<int> b;
        REQUIRE( b.load_txt( path ) );
        REQUIRE( s5_r2::same( a, b ) );
    }
    {
        feng::matrix<std::uint8_t> a{ 2, 3 };
        a[0][0] = 0; a[0][1] = 9; a[0][2] = 10; a[1][0] = 32; a[1][1] = 44; a[1][2] = 255;
        auto const path = s5_r2::temp_path( "rt_u8.txt" );
        REQUIRE( a.save_as_txt( path ) );
        feng::matrix<std::uint8_t> b;
        REQUIRE( b.load_txt( path ) );
        REQUIRE( s5_r2::same( a, b ) );
    }
    {
        feng::matrix<std::int8_t> a{ 1, 3 };
        a[0][0] = -128; a[0][1] = 0; a[0][2] = 127;
        auto const path = s5_r2::temp_path( "rt_i8.txt" );
        REQUIRE( a.save_as_txt( path ) );
        feng::matrix<std::int8_t> b;
        REQUIRE( b.load_txt( path ) );
        REQUIRE( s5_r2::same( a, b ) );
    }
}

TEST_CASE( "S5 operator>> parses text and sets failbit without touching the matrix on bad input", "[S5][S5-R2][S5-R4]" )
{
    feng::matrix<double> m;
    std::istringstream good{ "1\t2\t\n3\t4\t\n" };
    good >> m;
    REQUIRE( !good.fail() );
    REQUIRE( s5_r2::is_prefilled( m ) );

    std::istringstream ragged{ "1 2\n3\n" };
    ragged >> m;
    REQUIRE( ragged.fail() );
    REQUIRE( s5_r2::is_prefilled( m ) );

    std::istringstream empty{ "" };
    empty >> m;
    REQUIRE( empty.fail() );
    REQUIRE( s5_r2::is_prefilled( m ) );
}

TEST_CASE( "S5 load_binary rejects short, long and overflowing-count files into a pre-filled matrix", "[S5][S5-R2][S5-R4]" )
{
    REQUIRE( s5_r2::bin_rejected( "empty.bin", "" ) );
    REQUIRE( s5_r2::bin_rejected( "three.bin", "abc" ) );
    auto const header_only = s5_r2::binary_bytes<double>( 2, 2, {} );
    REQUIRE( s5_r2::bin_rejected( "one_count.bin", header_only.substr( 0, 8 ) ) );
    REQUIRE( s5_r2::bin_rejected( "short.bin", s5_r2::binary_bytes<double>( 2, 2, { 1, 2, 3 } ) ) );
    REQUIRE( s5_r2::bin_rejected( "long.bin", s5_r2::binary_bytes<double>( 2, 2, { 1, 2, 3, 4, 5 } ) ) );
    auto trailing = s5_r2::binary_bytes<double>( 2, 2, { 1, 2, 3, 4 } );
    trailing += '\0';
    REQUIRE( s5_r2::bin_rejected( "trailing.bin", trailing ) );
    REQUIRE( s5_r2::bin_rejected( "count_overflow.bin", s5_r2::binary_bytes<double>( 1ull << 32, 1ull << 32, { 1 } ) ) );
    REQUIRE( s5_r2::bin_rejected( "bytes_overflow.bin", s5_r2::binary_bytes<double>( 1ull << 62, 1, { 1 } ) ) );
    REQUIRE( s5_r2::bin_rejected( "max_counts.bin", s5_r2::binary_bytes<double>( ~0ull, ~0ull, { 1 } ) ) );
    REQUIRE( s5_r2::bin_rejected( "huge_rows.bin", s5_r2::binary_bytes<double>( 1ull << 40, 1, { 1 } ) ) );

    auto m = s5_r2::prefilled();
    std::ostringstream captured;
    auto* const old = std::cerr.rdbuf( captured.rdbuf() );
    bool const ok = m.load_binary( s5_r2::temp_file( "short2.bin", s5_r2::binary_bytes<double>( 2, 2, { 1 } ) ) );
    std::cerr.rdbuf( old );
    REQUIRE( !ok );
    REQUIRE( s5_r2::is_prefilled( m ) );
    std::string const text = captured.str();
    REQUIRE( text.find( "load_binary" ) != std::string::npos );
    REQUIRE( std::count( text.begin(), text.end(), '\n' ) == 1 );

    auto const missing = s5_r2::temp_path( "does_not_exist.bin" );
    std::filesystem::remove( missing );
    REQUIRE( !m.load_binary( missing ) );
    REQUIRE( s5_r2::is_prefilled( m ) );
}

TEST_CASE( "S5 load_binary round-trips 0x3, 3x0 and filled matrices", "[S5][S5-R2]" )
{
    for ( auto const& [r, c] : { std::pair<unsigned, unsigned>{ 0, 3 }, { 3, 0 }, { 2, 3 } } )
    {
        feng::matrix<double> a{ r, c };
        for ( std::size_t k = 0; k != a.size(); ++k ) a.data()[k] = 0.5 * static_cast<double>( k ) - 1.0;
        auto const path = s5_r2::temp_path( "rt_" + std::to_string( r ) + "x" + std::to_string( c ) + ".bin" );
        REQUIRE( a.save_as_binary( path ) );
        auto b = s5_r2::prefilled();
        REQUIRE( b.load_binary( path ) );
        REQUIRE( b.row() == r );
        REQUIRE( b.col() == c );
        REQUIRE( s5_r2::same( a, b ) );
    }
    feng::matrix<std::complex<float>> z{ 1, 2 };
    z[0][0] = { 1.0f, -2.0f }; z[0][1] = { 0.25f, 8.0f };
    auto const path = s5_r2::temp_path( "rt_cf.bin" );
    REQUIRE( z.save_as_binary( path ) );
    feng::matrix<std::complex<float>> w;
    REQUIRE( w.load_binary( path ) );
    REQUIRE( s5_r2::same( z, w ) );
}

namespace s5_r2
{
    // Records the largest single request (in bytes) any destination matrix made through this allocator.
    inline std::size_t largest_request = 0;

    template < typename T >
    struct max_request_allocator
    {
        using value_type = T;
        max_request_allocator() noexcept = default;
        template < typename U > max_request_allocator( max_request_allocator<U> const& ) noexcept {}
        T* allocate( std::size_t n )
        {
            largest_request = std::max( largest_request, n * sizeof( T ) );
            return std::allocator<T>{}.allocate( n );
        }
        void deallocate( T* p, std::size_t n ) noexcept { std::allocator<T>{}.deallocate( p, n ); }
        template < typename U > friend bool operator == ( max_request_allocator const&, max_request_allocator<U> const& ) noexcept { return true; }
    };

    template < typename T >
    using tracked_matrix = feng::matrix< T, max_request_allocator<T> >;

    // A version 1.0 NPY image with the given header dict and payload; the header is padded to a multiple of 64.
    inline std::string npy_bytes( std::string const& dict, std::string const& payload )
    {
        std::string header = dict;
        while ( ( 10 + header.size() + 1 ) % 64 != 0 ) header += ' ';
        header += '\n';
        std::string s = std::string( "\x93NUMPY\x01\x00", 8 );
        s += static_cast<char>( header.size() & 0xff );
        s += static_cast<char>( header.size() >> 8 );
        return s + header + payload;
    }

    template < typename T >
    std::string raw( std::vector<T> const& v )
    {
        std::string s( v.size() * sizeof( T ), '\0' );
        if ( !v.empty() ) std::memcpy( s.data(), v.data(), s.size() );
        return s;
    }

    // Runs `load` on the bytes into a pre-filled tracked matrix; true when the largest destination request is at
    // most sizeof(T) * (file size + 1).
    template < typename T, typename Load >
    bool bounded( std::string const& name, std::string const& bytes, Load load )
    {
        tracked_matrix<T> m{ 2, 2, T{ 1 } };
        auto const path = temp_file( name, bytes );
        largest_request = 0;
        (void)load( m, path );
        return largest_request <= sizeof( T ) * ( bytes.size() + 1 );
    }

    template < typename T >
    std::vector<T> extremes()
    {
        if constexpr ( std::is_integral_v<T> )
            return { std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max(), T{ 0 }, T{ 1 }, static_cast<T>( std::numeric_limits<T>::max() - 1 ), static_cast<T>( std::numeric_limits<T>::lowest() + 1 ) };
        else
            return { std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), std::numeric_limits<T>::denorm_min(),
                     -std::numeric_limits<T>::denorm_min(), std::numeric_limits<T>::epsilon(), T{ 1 } / T{ 3 }, T{ 0 } };
    }

    template < typename T >
    std::vector<T> extreme_elements()
    {
        if constexpr ( std::is_floating_point_v<T> || std::is_integral_v<T> )
            return extremes<T>();
        else
        {
            using F = typename T::value_type;
            auto const e = extremes<F>();
            std::vector<T> v;
            for ( std::size_t k = 0; k != e.size(); ++k ) v.emplace_back( e[k], e[e.size() - 1 - k] );
            return v;
        }
    }

    // A 2 x (n/2) matrix holding the type's extreme values.
    template < typename T >
    feng::matrix<T> extreme_matrix()
    {
        auto const v = extreme_elements<T>();
        feng::matrix<T> a{ 2, static_cast<unsigned>( v.size() / 2 ) };
        std::copy( v.begin(), v.begin() + static_cast<std::ptrdiff_t>( a.size() ), a.begin() );
        return a;
    }

    template < typename T >
    bool txt_round_trips( std::string const& name )
    {
        auto const a = extreme_matrix<T>();
        auto const path = temp_path( name );
        if ( !a.save_as_txt( path ) ) return false;
        feng::matrix<T> b;
        return b.load_txt( path ) && same( a, b );
    }

    template < typename T >
    bool bin_round_trips( std::string const& name )
    {
        auto const a = extreme_matrix<T>();
        auto const path = temp_path( name );
        if ( !a.save_as_binary( path ) ) return false;
        auto b = prefilled<T>();
        return b.load_binary( path ) && same( a, b );
    }

    // Integer type T rejects max+1 and min-1 (given as text).
    template < typename T >
    bool int_out_of_range_rejected( std::string const& above, std::string const& below )
    {
        return txt_rejected<T>( "above.txt", "1 " + above + "\n" ) && txt_rejected<T>( "below.txt", "1 " + below + "\n" );
    }
}

TEST_CASE( "S5 every loader's largest allocation is bounded by the file size", "[S5][S5-R2]" )
{
    auto txt = []( auto& m, std::string const& p ) { return m.load_txt( p ); };
    auto bin = []( auto& m, std::string const& p ) { return m.load_binary( p ); };
    auto npy = []( auto& m, std::string const& p ) { return m.load_npy( p ); };

    // load_txt: valid, ragged, bad token, blank and empty inputs.
    for ( std::string const& text : std::vector<std::string>{ "1 2 3\n4 5 6\n", "1\n", "1,2;3 4\t5\r\n", "1 2\n3\n", "1 2\n3 x\n", "\n\n", "",
                                     std::string( 4096, '1' ) + "\n", "(1,2) (3,4)\n" } )
    {
        REQUIRE( s5_r2::bounded<double>( "bnd.txt", text, txt ) );
        REQUIRE( s5_r2::bounded<std::uint8_t>( "bnd.txt", text, txt ) );
        REQUIRE( s5_r2::bounded<std::complex<long double>>( "bnd.txt", text, txt ) );
    }
    // The densest text: one-character tokens with one-character separators.
    std::string dense;
    for ( int k = 0; k != 1000; ++k ) dense += "1 ";
    REQUIRE( s5_r2::bounded<std::uint8_t>( "dense.txt", dense, txt ) );
    REQUIRE( s5_r2::largest_request == 1000 );
    REQUIRE( s5_r2::bounded<long double>( "dense.txt", dense + "\n" + dense, txt ) );

    // load_binary: valid, empty-shape, short, long and huge declared shapes.
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( 2, 3, { 1, 2, 3, 4, 5, 6 } ), bin ) );
    REQUIRE( s5_r2::largest_request == 6 * sizeof( double ) ); // the destination allocator is the one tracked
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( 0, 3, {} ), bin ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( 2, 3, { 1 } ), bin ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( 1, 1, { 1, 2 } ), bin ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( 1ull << 40, 1ull << 20, { 1 } ), bin ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( 1ull << 28, 1, { 1 } ), bin ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", s5_r2::binary_bytes<double>( ~0ull, ~0ull, {} ), bin ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.bin", "abc", bin ) );
    REQUIRE( s5_r2::bounded<std::uint8_t>( "bnd.bin", s5_r2::binary_bytes<std::uint8_t>( 1ull << 30, 1ull << 30, { 1, 2, 3 } ), bin ) );

    // load_npy: valid C and Fortran order, short payloads and headers declaring huge shapes.
    auto const six = s5_r2::raw<double>( { 1, 2, 3, 4, 5, 6 } );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3), }", six ), npy ) );
    REQUIRE( s5_r2::largest_request == 6 * sizeof( double ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': True, 'shape': (3, 2), }", six ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (6,), }", six ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (1000000000, 1000000000), }", six ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (18446744073709551615, 2), }", six ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (4294967296,), }", six ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (0, 1099511627776), }", "" ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '<f8', 'fortran_order': False, 'shape': (2, 3), }", six.substr( 0, 7 ) ), npy ) );
    REQUIRE( s5_r2::bounded<double>( "bnd.npy", "\x93NUMPY", npy ) );
    REQUIRE( s5_r2::bounded<std::uint8_t>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '|u1', 'fortran_order': False, 'shape': (65536, 65536), }", "abc" ), npy ) );
    REQUIRE( s5_r2::bounded<std::uint8_t>( "bnd.npy", s5_r2::npy_bytes( "{'descr': '|u1', 'fortran_order': False, 'shape': (1, 3), }", "abc" ), npy ) );
}

TEST_CASE( "S5 load_txt delimiters, out-of-range values per type, underflow and complex forms", "[S5][S5-R2][S5-R4]" )
{
    // Each delimiter alone separates tokens; CRLF and blank lines are accepted.
    for ( std::string const& sep : std::vector<std::string>{ ",", ";", " ", "\t", "\r" } )
    {
        feng::matrix<double> m;
        REQUIRE( m.load_txt( s5_r2::temp_file( "sep.txt", "1" + sep + "2\r\n\r\n\n+3" + sep + "-4\r\n" ) ) );
        REQUIRE( m.row() == 2 ); REQUIRE( m.col() == 2 );
        REQUIRE( m[0][0] == 1.0 ); REQUIRE( m[0][1] == 2.0 ); REQUIRE( m[1][0] == 3.0 ); REQUIRE( m[1][1] == -4.0 );
    }

    REQUIRE( s5_r2::int_out_of_range_rejected<std::int8_t>( "128", "-129" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::int16_t>( "32768", "-32769" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::int32_t>( "2147483648", "-2147483649" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::int64_t>( "9223372036854775808", "-9223372036854775809" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::uint8_t>( "256", "-1" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::uint16_t>( "65536", "-1" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::uint32_t>( "4294967296", "-1" ) );
    REQUIRE( s5_r2::int_out_of_range_rejected<std::uint64_t>( "18446744073709551616", "-1" ) );
    REQUIRE( s5_r2::txt_rejected<float>( "f_1e39.txt", "1 1e39\n" ) );
    REQUIRE( s5_r2::txt_rejected<float>( "f_m1e39.txt", "1 -1e39\n" ) );
    REQUIRE( s5_r2::txt_rejected<double>( "d_1e999.txt", "1 1e999\n" ) );
    // Underflow: the implementation rejects 1e-400 into double as out of range (from_chars reports it).
    REQUIRE( s5_r2::txt_rejected<double>( "d_1em400.txt", "1 1e-400\n" ) );
    REQUIRE( s5_r2::txt_rejected<std::complex<double>>( "cx_1e999.txt", "(1,1e999)\n" ) );

    // The largest in-range integers load.
    feng::matrix<std::int64_t> i;
    REQUIRE( i.load_txt( s5_r2::temp_file( "i64.txt", "9223372036854775807 -9223372036854775808\n" ) ) );
    REQUIRE( i[0][0] == std::numeric_limits<std::int64_t>::max() );
    REQUIRE( i[0][1] == std::numeric_limits<std::int64_t>::min() );

    feng::matrix<std::complex<float>> z;
    REQUIRE( z.load_txt( s5_r2::temp_file( "cxf.txt", "(+1.5,-2)\t7\r\n-3;( 0 , 1 )\r\n" ) ) );
    REQUIRE( z.row() == 2 ); REQUIRE( z.col() == 2 );
    REQUIRE( z[0][0] == std::complex<float>( 1.5f, -2.0f ) ); REQUIRE( z[0][1] == std::complex<float>( 7.0f, 0.0f ) );
    REQUIRE( z[1][0] == std::complex<float>( -3.0f, 0.0f ) ); REQUIRE( z[1][1] == std::complex<float>( 0.0f, 1.0f ) );
}

TEST_CASE( "S5 save_as_txt output loads back for every arithmetic and complex type at its extremes", "[S5][S5-R2]" )
{
    REQUIRE( s5_r2::txt_round_trips<std::int8_t>( "rtx_i8.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::int16_t>( "rtx_i16.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::int32_t>( "rtx_i32.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::int64_t>( "rtx_i64.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::uint8_t>( "rtx_u8.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::uint16_t>( "rtx_u16.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::uint32_t>( "rtx_u32.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::uint64_t>( "rtx_u64.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<float>( "rtx_f.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<double>( "rtx_d.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<long double>( "rtx_ld.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::complex<float>>( "rtx_cf.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::complex<double>>( "rtx_cd.txt" ) );
    REQUIRE( s5_r2::txt_round_trips<std::complex<long double>>( "rtx_cld.txt" ) );
}

TEST_CASE( "S5 operator>> reads every element type and leaves the matrix unchanged on a bad token", "[S5][S5-R2][S5-R4]" )
{
    feng::matrix<std::complex<double>> z;
    std::istringstream good{ "(1,2);3\r\n\n4 (5,-6)\n" };
    good >> z;
    REQUIRE( !good.fail() );
    REQUIRE( z.row() == 2 ); REQUIRE( z.col() == 2 );
    REQUIRE( z[0][0] == std::complex<double>( 1, 2 ) ); REQUIRE( z[1][1] == std::complex<double>( 5, -6 ) );

    auto m = s5_r2::prefilled<std::uint8_t>();
    std::istringstream bad_token{ "1 2\n3 x\n" };
    bad_token >> m;
    REQUIRE( bad_token.fail() );
    REQUIRE( s5_r2::is_prefilled<std::uint8_t>( m ) );

    std::istringstream out_of_range{ "1 2\n3 256\n" };
    out_of_range >> m;
    REQUIRE( out_of_range.fail() );
    REQUIRE( s5_r2::is_prefilled<std::uint8_t>( m ) );

    std::istringstream ok{ "4 3\n2 1\n" };
    ok >> m;
    REQUIRE( !ok.fail() );
    REQUIRE( m[0][0] == 4 ); REQUIRE( m[1][1] == 1 );
}

TEST_CASE( "S5 load_binary fails at every truncation and round-trips every element type", "[S5][S5-R2][S5-R4]" )
{
    auto const valid = s5_r2::binary_bytes<double>( 2, 2, { 5, 6, 7, 8 } );
    for ( std::size_t k = 0; k != valid.size(); ++k )
        REQUIRE( s5_r2::bin_rejected( "trunc.bin", valid.substr( 0, k ) ) );
    {
        auto m = s5_r2::prefilled();
        REQUIRE( m.load_binary( s5_r2::temp_file( "full.bin", valid ) ) );
        REQUIRE( m[0][0] == 5.0 ); REQUIRE( m[1][1] == 8.0 );
    }
    REQUIRE( s5_r2::bin_rejected( "trail.bin", valid + "x" ) );
    REQUIRE( s5_r2::bin_rejected( "rc_overflow.bin", s5_r2::binary_bytes<double>( 1ull << 33, 1ull << 31, {} ) ) );
    REQUIRE( s5_r2::bin_rejected( "byte_overflow.bin", s5_r2::binary_bytes<double>( 1ull << 61, 1, {} ) ) );
    REQUIRE( s5_r2::bin_rejected<std::uint8_t>( "u8_overflow.bin", s5_r2::binary_bytes<std::uint8_t>( ~0ull, 2, {} ) ) );

    REQUIRE( s5_r2::bin_round_trips<std::int8_t>( "rtb_i8.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::int16_t>( "rtb_i16.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::int32_t>( "rtb_i32.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::int64_t>( "rtb_i64.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::uint8_t>( "rtb_u8.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::uint16_t>( "rtb_u16.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::uint32_t>( "rtb_u32.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::uint64_t>( "rtb_u64.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<float>( "rtb_f.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<double>( "rtb_d.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<long double>( "rtb_ld.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::complex<float>>( "rtb_cf.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::complex<double>>( "rtb_cd.bin" ) );
    REQUIRE( s5_r2::bin_round_trips<std::complex<long double>>( "rtb_cld.bin" ) );

    for ( auto const& [r, c] : { std::pair<unsigned, unsigned>{ 0, 5 }, { 5, 0 }, { 0, 0 } } )
    {
        feng::matrix<std::complex<long double>> a{ r, c };
        auto const path = s5_r2::temp_path( "rtb_empty.bin" );
        REQUIRE( a.save_as_binary( path ) );
        auto b = s5_r2::prefilled<std::complex<long double>>();
        REQUIRE( b.load_binary( path ) );
        REQUIRE( b.row() == r ); REQUIRE( b.col() == c );
    }
}
