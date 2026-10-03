// bench/bench.cc: the S10-R1/S10-R2 (D-008) benchmark harness, built twice by `tools/check.sh bench compare` (once on
// the stage start commit's matrix.hpp, once on the working tree) with the same flags; it uses only API present at the
// baseline. One translation unit: matrix.hpp plus the standard library.
//
// Each workload builds its inputs from std::mt19937_64 seeded with a constant plus a hash of its name (without a par/
// prefix), then:
//   warmup: runs a batch of `reps` operations, doubling reps (from 1, capped) until one batch lasts >= 5 ms (1 ms with
//           --smoke); an operation slower than that keeps reps = 1;
//   timing: n samples of one batch each on steady_clock, printed as `SAMPLES <name> reps <r> <t1> ... <tn>` in seconds
//           per operation.
// Every result feeds a checksum sink, printed as `CHECKSUM <x>`, so nothing is optimized away. The run starts with
// `INFO cpu <model>`, `INFO cores <n>`, `INFO compiler <__VERSION__>` and `INFO flags <BENCH_FLAGS>`.
// CLI: --list (names, one per line; with --smoke the smoke subset, one small size per family), --only <name> (repeat
// to run several), --samples <n> (default 10; 3 with --smoke), --smoke.
// Parallel variant (S10-T6): built with -DFENG_MATRIX_PARALLEL the harness offers only the par/ set (in_par_set:
// elementwise and statistics at every size, gemm 8/16/17/18/32/64/256/1024/tall/wide, gemv 8/32/64/1024), each
// name prefixed `par/` (e.g. par/add/4x4); --list, --smoke and --only work on those prefixed names.
#include "matrix.hpp"

#include <chrono>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

#ifndef BENCH_FLAGS
#define BENCH_FLAGS "unknown"
#endif

namespace
{
    using dmat = feng::matrix< double >;
    using cmat = feng::matrix< std::complex< double > >;

    double sink = 0.0;

    void touch( double x ) { sink += x; }
    void touch( std::complex< double > x ) { sink += x.real() + x.imag(); }
    template < typename M >
    void touch_matrix( M const& m )
    {
        if ( m.size() != 0 ) touch( m[m.row() / 2][m.col() / 2] );
    }

    std::uint64_t name_hash( std::string const& s )
    {
        std::uint64_t h = 1469598103934665603ULL;
        for ( unsigned char c : s ) { h ^= c; h *= 1099511628211ULL; }
        return h;
    }

    dmat random_dmat( std::mt19937_64& gen, std::size_t r, std::size_t c, double lo, double hi )
    {
        std::uniform_real_distribution< double > dist{ lo, hi };
        dmat m( r, c );
        for ( auto& v : m ) v = dist( gen );
        return m;
    }

    cmat random_cmat( std::mt19937_64& gen, std::size_t r, std::size_t c )
    {
        std::uniform_real_distribution< double > dist{ -1.0, 1.0 };
        cmat m( r, c );
        for ( auto& v : m ) v = std::complex< double >{ dist( gen ), dist( gen ) };
        return m;
    }

    using op_type = std::function< void() >;

    struct workload
    {
        std::string name;
        bool smoke;
        std::function< op_type( std::mt19937_64& ) > setup; // builds the inputs, returns one operation
    };

    // The workloads of the parallel build (see the header comment); names without the par/ prefix.
    bool in_par_set( std::string const& name )
    {
        static char const* const fixed[] = { "gemm/8", "gemm/16", "gemm/17", "gemm/18", "gemm/32", "gemm/64",
                                             "gemm/256", "gemm/1024", "gemm/tall", "gemm/wide",
                                             "gemv/8", "gemv/32", "gemv/64", "gemv/1024" };
        for ( char const* f : fixed )
            if ( name == f ) return true;
        static char const* const families[] = { "add/", "scale/", "map/", "sum/", "mean/", "variance/", "max/" };
        for ( char const* f : families )
            if ( name.rfind( f, 0 ) == 0 ) return true;
        return false;
    }

    std::vector< workload > make_workloads_all();

    std::vector< workload > make_workloads()
    {
#ifdef FENG_MATRIX_PARALLEL
        std::vector< workload > par;
        for ( auto& w : make_workloads_all() )
            if ( in_par_set( w.name ) ) par.push_back( { "par/" + w.name, w.smoke, std::move( w.setup ) } );
        return par;
#else
        return make_workloads_all();
#endif
    }

    std::vector< workload > make_workloads_all()
    {
        std::vector< workload > w;
        auto gemm = []( std::size_t m, std::size_t k, std::size_t n )
        {
            return [m, k, n]( std::mt19937_64& gen ) -> op_type
            {
                auto a = std::make_shared< dmat >( random_dmat( gen, m, k, -1.0, 1.0 ) );
                auto b = std::make_shared< dmat >( random_dmat( gen, k, n, -1.0, 1.0 ) );
                return [a, b] { touch_matrix( ( *a ) * ( *b ) ); };
            };
        };
        for ( std::size_t n : { 8, 16, 17, 18, 32, 64, 128, 256, 512, 1024 } )
            w.push_back( { "gemm/" + std::to_string( n ), n == 32, gemm( n, n, n ) } );
        w.push_back( { "gemm/tall", false, gemm( 4096, 64, 64 ) } );
        w.push_back( { "gemm/wide", false, gemm( 64, 64, 4096 ) } );
        for ( std::size_t n : { 8, 16, 32, 64, 128, 256, 512, 1024 } )
            w.push_back( { "gemv/" + std::to_string( n ), n == 32, gemm( n, n, 1 ) } );

        auto shaped = []( std::size_t n ) { return std::to_string( n ) + "x" + std::to_string( n ); };
        for ( std::size_t n : { 4, 32, 100, 1000 } )
        {
            bool const sm = n == 32;
            auto const sz = shaped( n );
            w.push_back( { "add/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                auto b = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a, b] { touch_matrix( ( *a ) + ( *b ) ); }; } } );
            w.push_back( { "scale/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a] { touch_matrix( ( *a ) * 1.0001 ); }; } } );
            w.push_back( { "map/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a] { touch_matrix( feng::sqrt( *a ) ); }; } } );
            w.push_back( { "sum/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a] { touch( feng::sum( *a ) ); }; } } );
            w.push_back( { "mean/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a] { touch( feng::mean( *a ) ); }; } } );
            w.push_back( { "variance/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a] { touch( feng::variance( *a ) ); }; } } );
            w.push_back( { "max/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, 0.5, 1.5 ) );
                return [a] { touch( feng::max( *a ) ); }; } } );
        }

        for ( std::size_t n : { 64, 1024 } )
        {
            bool const sm = n == 64;
            auto const sz = std::to_string( n );
            w.push_back( { "transpose/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, -1.0, 1.0 ) );
                return [a] { touch_matrix( a->transpose() ); }; } } );
            // a view of the n x n block at (n/4, n/3) of a 2n x 2n owner
            w.push_back( { "view_copy/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, 2 * n, 2 * n, -1.0, 1.0 ) );
                return [a, n] {
                    dmat const c{ feng::make_view( *a, { n / 4, n / 4 + n }, { n / 3, n / 3 + n } ) };
                    touch_matrix( c ); }; } } );
            w.push_back( { "view_sum/" + sz, sm, [n]( std::mt19937_64& gen ) -> op_type {
                auto a = std::make_shared< dmat >( random_dmat( gen, 2 * n, 2 * n, -1.0, 1.0 ) );
                return [a, n] {
                    auto const v = feng::make_view( *a, { n / 4, n / 4 + n }, { n / 3, n / 3 + n } );
                    touch( std::accumulate( v.begin(), v.end(), 0.0 ) ); }; } } );
        }

        for ( std::size_t n : { 128, 256, 125, 250 } )
            w.push_back( { "fft/" + std::to_string( n ), n == 128 || n == 125, [n]( std::mt19937_64& gen ) -> op_type {
                auto x = std::make_shared< cmat >( random_cmat( gen, n, n ) );
                return [x] { touch_matrix( feng::fft( *x ) ); }; } } );

        auto conv = []( std::size_t n, std::size_t k )
        {
            return [n, k]( std::mt19937_64& gen ) -> op_type
            {
                auto a = std::make_shared< dmat >( random_dmat( gen, n, n, -1.0, 1.0 ) );
                auto b = std::make_shared< dmat >( random_dmat( gen, k, k, -1.0, 1.0 ) );
                return [a, b] { touch_matrix( feng::conv( *a, *b ) ); };
            };
        };
        w.push_back( { "conv/64k5", true, conv( 64, 5 ) } );
        w.push_back( { "conv/256k7", false, conv( 256, 7 ) } );
        return w;
    }

    std::string cpu_model()
    {
        std::ifstream in{ "/proc/cpuinfo" };
        std::string line;
        while ( std::getline( in, line ) )
            if ( line.rfind( "model name", 0 ) == 0 )
            {
                auto const p = line.find( ':' );
                if ( p != std::string::npos )
                {
                    auto s = line.substr( p + 1 );
                    s.erase( 0, s.find_first_not_of( " \t" ) );
                    return s;
                }
            }
        return "unknown";
    }

    void run_workload( workload const& w, int samples, double target )
    {
        // par/ workloads hash the unprefixed name, so they get the same inputs as their serial twins
        std::string const base_name = w.name.rfind( "par/", 0 ) == 0 ? w.name.substr( 4 ) : w.name;
        std::mt19937_64 gen{ 20261001ULL + name_hash( base_name ) };
        op_type const op = w.setup( gen );
        auto const batch = [&]( long reps )
        {
            auto const t0 = std::chrono::steady_clock::now();
            for ( long r = 0; r != reps; ++r ) op();
            auto const t1 = std::chrono::steady_clock::now();
            return std::chrono::duration< double >( t1 - t0 ).count();
        };
        long reps = 1;
        long const reps_cap = 1L << 24;
        while ( batch( reps ) < target && reps < reps_cap ) reps *= 2;
        std::printf( "SAMPLES %s reps %ld", w.name.c_str(), reps );
        for ( int s = 0; s != samples; ++s ) std::printf( " %.6e", batch( reps ) / static_cast< double >( reps ) );
        std::printf( "\n" );
        std::fflush( stdout );
    }
}

int main( int argc, char** argv )
{
    bool list = false, smoke = false;
    int samples = -1;
    std::vector< std::string > only;
    for ( int i = 1; i < argc; ++i )
    {
        if ( std::strcmp( argv[i], "--list" ) == 0 ) list = true;
        else if ( std::strcmp( argv[i], "--smoke" ) == 0 ) smoke = true;
        else if ( std::strcmp( argv[i], "--only" ) == 0 && i + 1 < argc ) only.emplace_back( argv[++i] );
        else if ( std::strcmp( argv[i], "--samples" ) == 0 && i + 1 < argc )
        {
            samples = std::atoi( argv[++i] );
            if ( samples < 1 ) { std::fprintf( stderr, "bench: --samples needs a positive count\n" ); return 2; }
        }
        else { std::fprintf( stderr, "bench: unknown argument '%s'\nusage: bench [--list] [--only <name>]... [--samples <n>] [--smoke]\n", argv[i] ); return 2; }
    }
    if ( samples < 0 ) samples = smoke ? 3 : 10;

    auto const all = make_workloads();
    if ( list )
    {
        for ( auto const& w : all )
            if ( !smoke || w.smoke ) std::printf( "%s\n", w.name.c_str() );
        return 0;
    }

    std::vector< workload const* > chosen;
    if ( only.empty() )
    {
        for ( auto const& w : all )
            if ( !smoke || w.smoke ) chosen.push_back( &w );
    }
    else
        for ( auto const& name : only )
        {
            workload const* found = nullptr;
            for ( auto const& w : all )
                if ( w.name == name ) found = &w;
            if ( !found ) { std::fprintf( stderr, "bench: unknown workload '%s'\n", name.c_str() ); return 2; }
            chosen.push_back( found );
        }

    std::printf( "INFO cpu %s\n", cpu_model().c_str() );
    std::printf( "INFO cores %u\n", std::thread::hardware_concurrency() );
    std::printf( "INFO compiler %s\n", __VERSION__ );
    std::printf( "INFO flags %s\n", BENCH_FLAGS );
    std::fflush( stdout );
    double const target = smoke ? 0.001 : 0.005;
    for ( auto const* w : chosen ) run_workload( *w, samples, target );
    std::printf( "CHECKSUM %.17g\n", sink );
    return 0;
}
