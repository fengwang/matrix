// S8 oracle fixtures (PR-11, PR-14): the committed numpy/scipy files in tests/fixtures/s8/ match the S8 APIs
// within the absolute tolerance recorded on each manifest line (max |got - ref| over the elements; 0 is exact).
// The suite runs from the repo root (AGENTS.md), so the paths are relative to it. Prints `ORACLE S8 FIXTURES <n>`
// for tools/check.sh oracle, one line per TEST_CASE (the lane sums them). Ops are dispatched by name in s8fx::run
// (conv) and s8fx::run_fourier (fft2, ifft2, fftshift, ifftshift); the element type is the case-name field after
// the op: f4, f8, i4, c8 or c16. The result type of fft and ifft is checked against D-031 at compile time, and the
// reference file must load as that type, so numpy's result dtype agrees with D-031 too.
#include <cmath>
#include <complex>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace s8fx
{
    inline std::string const dir = "tests/fixtures/s8/";

    struct line
    {
        std::string name, op;
        double tol = 0;
        std::vector< std::string > files;
    };

    template< typename T >
    std::complex< long double > widen( T const& v )
    {
        return { static_cast< long double >( std::real( v ) ), static_cast< long double >( std::imag( v ) ) };
    }

    // max |got - ref| (complex modulus for complex elements); +inf on a shape mismatch
    template< typename T >
    double max_err( feng::matrix< T > const& got, feng::matrix< T > const& ref )
    {
        if ( got.row() != ref.row() || got.col() != ref.col() ) return HUGE_VAL;
        long double e = 0;
        for ( std::size_t i = 0; i != ref.size(); ++i )
        {
            long double const d = std::abs( widen( got.data()[i] ) - widen( ref.data()[i] ) );
            if ( !( d <= e ) ) e = d; // NaN propagates
        }
        return static_cast< double >( e );
    }

    template< typename T >
    bool load( std::string const& f, feng::matrix< T >& m )
    {
        return m.load_npy( dir + f );
    }

    // loads `want` files into `in`; false with why set when the count or a load fails
    template< typename T >
    bool load_all( line const& c, std::size_t want, std::vector< feng::matrix< T > >& in, std::string& why )
    {
        if ( c.files.size() != want ) { why = "expected " + std::to_string( want ) + " files"; return false; }
        in.resize( want );
        for ( std::size_t i = 0; i != want; ++i )
            if ( !load( c.files[i], in[i] ) ) { why = "cannot load " + dir + c.files[i]; return false; }
        return true;
    }

    // computes the op on the loaded inputs; returns the error, or -1 (with why set) when a file or the op failed
    template< typename T >
    double run( line const& c, std::string& why )
    {
        std::vector< feng::matrix< T > > in;
        if ( c.op == "conv_full" || c.op == "conv_same" || c.op == "conv_valid" )
        {
            if ( !load_all( c, 3, in, why ) ) return -1;
            std::string const mode = c.op.substr( 5 );
            return max_err( feng::conv( in[0], in[1], mode ), in[2] );
        }
        why = "unknown op " + c.op;
        return -1;
    }

    // D-031: float, double and long double give complex of the same real type, complex<T> stays, integers give
    // complex<double>
    template< typename T > struct fft_result { using type = std::complex< double >; };
    template<> struct fft_result< float > { using type = std::complex< float >; };
    template<> struct fft_result< long double > { using type = std::complex< long double >; };
    template< typename T > struct fft_result< std::complex< T > > { using type = std::complex< T >; };

    // fft2 / ifft2: files A C, C = numpy.fft.fft2(A) or ifft2(A) of D-031's type; fftshift / ifftshift: files A C of
    // A's type
    template< typename T >
    double run_fourier( line const& c, std::string& why )
    {
        using X = typename fft_result< T >::type;
        static_assert( std::is_same_v< decltype( feng::fft( std::declval< feng::matrix< T > const& >() ) ), feng::matrix< X > > );
        static_assert( std::is_same_v< decltype( feng::ifft( std::declval< feng::matrix< T > const& >() ) ), feng::matrix< X > > );
        static_assert( std::is_same_v< decltype( feng::fftshift( std::declval< feng::matrix< T > const& >() ) ), feng::matrix< T > > );
        static_assert( std::is_same_v< decltype( feng::ifftshift( std::declval< feng::matrix< T > const& >() ) ), feng::matrix< T > > );
        if ( c.files.size() != 2 ) { why = "expected 2 files"; return -1; }
        feng::matrix< T > a;
        if ( !load( c.files[0], a ) ) { why = "cannot load " + dir + c.files[0]; return -1; }
        if ( c.op == "fft2" || c.op == "ifft2" )
        {
            feng::matrix< X > ref;
            if ( !load( c.files[1], ref ) ) { why = "cannot load " + dir + c.files[1] + " as D-031's result type"; return -1; }
            return max_err( c.op == "fft2" ? feng::fft( a ) : feng::ifft( a ), ref );
        }
        if ( c.op == "fftshift" || c.op == "ifftshift" )
        {
            feng::matrix< T > ref;
            if ( !load( c.files[1], ref ) ) { why = "cannot load " + dir + c.files[1]; return -1; }
            return max_err( c.op == "fftshift" ? feng::fftshift( a ) : feng::ifftshift( a ), ref );
        }
        why = "unknown op " + c.op;
        return -1;
    }

    // the manifest lines whose op is one of `ops`
    inline std::vector< line > read_manifest( std::vector< std::string > const& ops )
    {
        std::ifstream in( dir + "manifest.txt" );
        REQUIRE( in.good() );
        std::vector< line > cases;
        std::string text;
        while ( std::getline( in, text ) )
        {
            if ( text.empty() || text[0] == '#' ) continue;
            std::istringstream is( text );
            line c;
            std::string f;
            is >> c.name >> c.op >> c.tol;
            INFO( "manifest line: " << text );
            REQUIRE( !is.fail() );
            while ( is >> f ) c.files.push_back( f );
            for ( auto const& op : ops )
                if ( c.op == op ) cases.push_back( c );
        }
        REQUIRE( cases.size() > 0 );
        return cases;
    }

    // case names are <op>_<type>_<shapes> (tests/fixtures/s8/make_fixtures.py)
    inline std::string type_of( line const& c )
    {
        std::size_t const at = c.op.size() + 1;
        return c.name.substr( at, c.name.find( '_', at ) - at );
    }

    // checks every case with `dispatch( case, type, why )` returning the error; prints the ORACLE line
    template< typename Dispatch >
    void check_cases( std::vector< line > const& cases, Dispatch dispatch )
    {
        std::size_t checked = 0;
        for ( auto const& c : cases )
        {
            std::string why;
            double const err = dispatch( c, type_of( c ), why );
            INFO( "case " << c.name << " (" << c.op << "): error " << err << ", tol " << c.tol << ( why.empty() ? "" : ", " ) << why );
            CHECK( why.empty() );
            CHECK( err >= 0 );
            CHECK( err <= c.tol );
            if ( why.empty() && err >= 0 && err <= c.tol ) ++checked;
        }
        CHECK( checked == cases.size() );
        std::cout << "ORACLE S8 FIXTURES " << checked << "\n";
    }

    inline double fourier( line const& c, std::string const& type, std::string& why )
    {
        if ( type == "f4" ) return run_fourier< float >( c, why );
        if ( type == "f8" ) return run_fourier< double >( c, why );
        if ( type == "i4" ) return run_fourier< int >( c, why );
        if ( type == "c8" ) return run_fourier< std::complex< float > >( c, why );
        if ( type == "c16" ) return run_fourier< std::complex< double > >( c, why );
        why = "unknown element type '" + type + "'";
        return -1;
    }
}

TEST_CASE( "S8-R1 conv matches the committed scipy fixtures", "[S8][S8-R1][oracle]" )
{
    auto const cases = s8fx::read_manifest( { "conv_full", "conv_same", "conv_valid" } );
    s8fx::check_cases( cases, []( s8fx::line const& c, std::string const& type, std::string& why ) -> double
    {
        if ( type == "f8" ) return s8fx::run< double >( c, why );
        if ( type == "i4" ) return s8fx::run< int >( c, why );
        why = "unknown element type '" + type + "'";
        return -1;
    } );
}

TEST_CASE( "S8-R2 fft and ifft match the committed numpy fixtures", "[S8][S8-R2][oracle]" )
{
    s8fx::check_cases( s8fx::read_manifest( { "fft2", "ifft2" } ), s8fx::fourier );
}

TEST_CASE( "S8-R3 shifts match the committed numpy fixtures", "[S8][S8-R3][oracle]" )
{
    s8fx::check_cases( s8fx::read_manifest( { "fftshift", "ifftshift" } ), s8fx::fourier );
}
