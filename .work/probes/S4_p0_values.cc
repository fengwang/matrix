// S4 pre-flight probe p0 (C8 + P2b, pre-fix evidence, part 1).
// Prints current (pre-fix) return types and values of mean for matrix<int>/matrix<float>/
// matrix<double>, of variance/standard_deviation for float/double (int variance/stddev do
// NOT COMPILE pre-fix — recorded separately in S4_p0b / prefix_p0.log), and the pre-fix
// cholesky_decomposition output on a non-PD and a PD input (void return, NaN expected
// on non-PD).
// Build: g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4_p0 .work/probes/S4_p0_values.cc && .work/probe_s4_p0

#include "../../matrix.hpp"

#include <cstdio>
#include <type_traits>

using feng::matrix;

template < typename T >
void print_type( char const* name, T const& )
{
    std::printf( "%-32s int=%d float=%d double=%d ulong=%d\n", name,
                 int( std::is_same_v< T, int > ), int( std::is_same_v< T, float > ), int( std::is_same_v< T, double > ), int( std::is_same_v< T, unsigned long > ) );
}

int main()
{
    matrix<int> const mi{ 1, 2, { 1, 2 } };
    auto const mi_mean = feng::mean( mi );
    print_type( "mean(matrix<int>)", mi_mean );
    std::printf( "int 1x2 {1,2}: mean=%lu (unsigned long = truncated, review expected 1.5)\n",
                 static_cast< unsigned long >( mi_mean ) );

    matrix<int> const mi2{ 2, 2, { 1, 2, 1, 2 } };
    auto const mi2_mean = feng::mean( mi2 );
    std::printf( "int 2x2 {1,2;1,2}: mean=%lu\n", static_cast< unsigned long >( mi2_mean ) );

    matrix<float> const mf{ 1, 2, { 1.0f, 2.0f } };
    auto const mf_mean = feng::mean( mf );
    auto const mf_var  = feng::variance( mf );
    auto const mf_std  = feng::standard_deviation( mf );
    print_type( "mean(matrix<float>)", mf_mean );
    print_type( "variance(matrix<float>)", mf_var );
    print_type( "standard_deviation(matrix<float>)", mf_std );
    std::printf( "float 1x2 {1,2}: mean=%g variance=%g std=%g\n", double( mf_mean ), double( mf_var ), double( mf_std ) );

    matrix<double> const md{ 1, 2, { 1.0, 2.0 } };
    auto const md_mean = feng::mean( md );
    auto const md_var  = feng::variance( md );
    auto const md_std  = feng::standard_deviation( md );
    print_type( "mean(matrix<double>)", md_mean );
    print_type( "variance(matrix<double>)", md_var );
    print_type( "standard_deviation(matrix<double>)", md_std );
    std::printf( "double 1x2 {1,2}: mean=%g variance=%g std=%g\n", double( md_mean ), double( md_var ), double( md_std ) );

    // pre-fix cholesky: void; non-PD input expected to silently produce NaN
    matrix<double> const npd{ 2, 2, { 1.0, 2.0, 2.0, 1.0 } };
    matrix<double> a;
    feng::cholesky_decomposition( npd, a );
    std::printf( "cholesky pre-fix [[1,2],[2,1]] -> a[0][0]=%g a[0][1]=%g a[1][1]=%g (NaN expected)\n", a[0][0], a[0][1], a[1][1] );

    matrix<double> const pd{ 2, 2, { 4.0, 2.0, 2.0, 3.0 } };
    matrix<double> b;
    feng::cholesky_decomposition( pd, b );
    std::printf( "cholesky pre-fix [[4,2],[2,3]] -> a[0][0]=%g a[0][1]=%g a[1][1]=%g (valid factor, no failure channel)\n", b[0][0], b[0][1], b[1][1] );

    std::printf( "PASS S4_p0 (pre-fix values recorded)\n" );
    return 0;
}
