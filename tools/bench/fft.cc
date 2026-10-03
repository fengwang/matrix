// tools/bench/fft.cc: the S8-R4 (D-031) complexity benchmark of feng::fft, run by `tools/check.sh bench fft`.
// Times feng::fft on fixed-seed complex<double> inputs: 128x128, 256x256, 512x512 (radix-2) and, for information,
// 125x125, 250x250, 500x500 (Bluestein). Per size one warmup, then 15 timed samples (5 with --smoke) on steady_clock.
// Each sample is a batch of `reps` transforms of the same input, so every sample lasts at least ~20 ms and timer and
// scheduler noise stay small next to it; the warmup picks `reps` (it doubles a batch from one transform until the
// batch lasts 20 ms). The per-transform median (sample time / reps) is printed as `BENCH fft <R>x<C> median <s> s`,
// with `RATIO <a>/<b> <x>` and `INFO bluestein ...` lines.
#include "matrix.hpp"

#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace
{
    double sink = 0.0; // keeps the transforms observable

    double median_seconds( std::size_t n, int samples )
    {
        std::mt19937_64 gen{ 20261001ULL + n };
        std::uniform_real_distribution< double > dist{ -1.0, 1.0 };
        feng::matrix< std::complex< double > > x( n, n );
        for ( auto& v : x ) v = std::complex< double >{ dist( gen ), dist( gen ) };

        auto const run = [&]()
        {
            auto const X = feng::fft( x );
            sink += X[n / 2][n / 3].real();
        };
        auto const batch = [&]( long reps )
        {
            auto const t0 = std::chrono::steady_clock::now();
            for ( long r = 0; r != reps; ++r ) run();
            auto const t1 = std::chrono::steady_clock::now();
            return std::chrono::duration< double >( t1 - t0 ).count();
        };
        // warmup: grows the batch until it lasts at least 20 ms; its final size is the repeat count
        long reps = 1;
        while ( batch( reps ) < 0.020 ) reps *= 2;
        std::vector< double > t;
        t.reserve( static_cast< std::size_t >( samples ) );
        for ( int s = 0; s != samples; ++s ) t.push_back( batch( reps ) / static_cast< double >( reps ) );
        std::sort( t.begin(), t.end() );
        std::size_t const m = t.size() / 2;
        return t.size() % 2 ? t[m] : 0.5 * ( t[m - 1] + t[m] );
    }
}

int main( int argc, char** argv )
{
    int samples = 15;
    for ( int i = 1; i < argc; ++i )
        if ( std::strcmp( argv[i], "--smoke" ) == 0 ) samples = 5;

    double const m128 = median_seconds( 128, samples );
    double const m256 = median_seconds( 256, samples );
    double const m512 = median_seconds( 512, samples );
    std::printf( "BENCH fft 128x128 median %.6f s\n", m128 );
    std::printf( "BENCH fft 256x256 median %.6f s\n", m256 );
    std::printf( "BENCH fft 512x512 median %.6f s\n", m512 );
    std::printf( "RATIO 256/128 %.3f\n", m256 / m128 );
    std::printf( "RATIO 512/256 %.3f\n", m512 / m256 );

    double const b125 = median_seconds( 125, samples );
    double const b250 = median_seconds( 250, samples );
    double const b500 = median_seconds( 500, samples );
    std::printf( "INFO bluestein 125x125 median %.6f s\n", b125 );
    std::printf( "INFO bluestein 250x250 median %.6f s\n", b250 );
    std::printf( "INFO bluestein 500x500 median %.6f s\n", b500 );
    std::printf( "INFO bluestein ratio 250/125 %.3f\n", b250 / b125 );
    std::printf( "INFO bluestein ratio 500/250 %.3f\n", b500 / b250 );
    std::printf( "INFO samples %d checksum %.6g\n", samples, sink );
    return 0;
}
