// S4 adversarial verification — attack probes (asserts live, -O1, no fast-math).
#include "../../matrix.hpp"
#include <cmath>
#include <cstdio>
namespace { int failures = 0;
    void ck( bool ok, char const* w ){ if (!ok){ std::printf("FAIL: %s\n", w); ++failures; } } }
int main()
{
    // A1: negative leading diagonal -> guard at i=0 (before any off-diagonal work)
    {
        feng::matrix<double> const m{ 2, 2, { -1.0, 0.0, 0.0, 1.0 } };
        feng::matrix<double> a;
        ck( feng::cholesky_decomposition( m, a ) == false, "A1 neg diag -> false" );
        ck( a[0][0] == -1.0 && a[1][1] == 1.0, "A1 a defined (input preserved)" );
    }
    // A2: PSD rank-2 3x3 -> guard must fire at the DEEPEST step (i=2)
    {
        feng::matrix<double> const m{ 3, 3, { 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 1.0, 2.0, 2.0 } };
        feng::matrix<double> a;
        ck( feng::cholesky_decomposition( m, a ) == false, "A2 PSD 3x3 -> false at i=2" );
        ck( a[2][2] == 2.0, "A2 a[2][2] preserved (no NaN)" );
        ck( a[0][0] == 1.0 && a[1][1] == 1.0 && a[1][0] == 1.0 && a[2][0] == 1.0 && a[2][1] == 1.0, "A2 earlier steps completed" );
    }
    // A5: rref 1x1
    {
        ck( feng::rref( feng::matrix<double>{ 1, 1, { 5.0 } } ).has_value(), "A5 1x1 {5} -> value" );
        ck( !feng::rref( feng::matrix<double>{ 1, 1, { 0.0 } } ).has_value(), "A5 1x1 {0} -> nullopt" );
    }
    // A6: rref 1x2 wide with off-diagonal pivot (swap in wide geometry)
    {
        auto const r = feng::rref( feng::matrix<double>{ 1, 2, { 2.0, 4.0 } } );
        ck( r.has_value() && ( *r )[0][0] == 1.0 && ( *r )[0][1] == 2.0, "A6 rref({2,4}) -> {1,2}" );
    }
    // A8: LARGE int matrix mean — exercises the PARALLEL reduce path (n > cores) on the promoted type
    {
        feng::matrix<int> big( 128, 128 );
        int v = 1;
        for ( auto& x : big ) x = v++;
        double const mu = feng::mean( big ); // 1..16384 -> 8192.5
        ck( std::abs( mu - 8192.5 ) < 1.0e-9, "A8 large int mean (parallel path)" );
        std::printf( "A8 mean = %g\n", mu );
    }
    // A9: all-equal int matrix (zero deviations exactly)
    {
        feng::matrix<int> const eq{ 1, 4, { 7, 7, 7, 7 } };
        ck( feng::variance( eq ) == 0.0 && feng::standard_deviation( eq ) == 0.0, "A9 equal int -> 0/0" );
    }
    // A15: conv 1x1 kernel on 1x1 input
    {
        feng::matrix<double> const C = feng::conv( feng::matrix<double>{ 1, 1, { 3.0 } }, feng::matrix<double>{ 1, 1, { 0.25 } }, std::string{ "same" } );
        ck( C.row() == 1 && C.col() == 1 && C[0][0] == 0.75, "A15 1x1 x 1x1 -> 0.75" );
    }
    if ( failures ) { std::printf( "ADV: %d FAILURES\n", failures ); return 1; }
    std::printf( "ADV-PASS\n" );
    return 0;
}
