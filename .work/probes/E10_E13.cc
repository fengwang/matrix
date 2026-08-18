// S4 deterministic check (contract `deterministic_check`, verbatim build):
//   g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4 .work/probes/E10_E13.cc && .work/probe_s4
// Prints PASS. Built WITHOUT -DNDEBUG (asserts live: E11/E12 would abort if the
// pre-fix preconditions survived) and at -O1 (no fast-math: exact comparisons).
// row>col rref is excluded by design: pre-existing release-reachable OOB
// (ASan probe pair S4_p3_wide_asan.cc), out of S4 scope.

#include "../../matrix.hpp"

#include <cmath>
#include <cstdio>
#include <type_traits>

namespace
{
    int failures = 0;

    void check( bool ok, char const* what )
    {
        if ( !ok )
        {
            std::printf( "FAIL: %s\n", what );
            ++failures;
        }
    }
}

int main()
{
    // ---- E10: C8 statistics (integer / float / double -> double) ----
    {
        feng::matrix<int> const m12{ 1, 2, { 1, 2 } };
        static_assert( std::is_same_v< decltype( feng::mean( m12 ) ), double > );
        static_assert( std::is_same_v< decltype( feng::variance( m12 ) ), double > );
        static_assert( std::is_same_v< decltype( feng::standard_deviation( m12 ) ), double > );
        check( feng::mean( m12 ) == 1.5, "E10 mean(1x2 int)" );
        check( feng::variance( m12 ) == 0.25, "E10 variance(1x2 int)" );
        check( feng::standard_deviation( m12 ) == std::sqrt( 0.5 ), "E10 std(1x2 int) = sqrt(0.5) (n-1 kept)" );

        feng::matrix<int> const m22{ 2, 2, { 1, 2, 1, 2 } };
        check( feng::mean( m22 ) == 1.5, "E10 mean(2x2 int)" );
        check( feng::variance( m22 ) == 0.25, "E10 variance(2x2 int)" );
        check( feng::standard_deviation( m22 ) == std::sqrt( 1.0 / 3.0 ), "E10 std(2x2 int) = sqrt(1/3)" );

        feng::matrix<int> const m11{ 1, 1, { 7 } };
        check( feng::mean( m11 ) == 7.0, "E10 mean(1x1 int)" );
        check( feng::variance( m11 ) == 0.0, "E10 variance(1x1 int)" );
        check( feng::standard_deviation( m11 ) == 0.0, "E10 std(1x1 int)" );

        feng::matrix<float> const f12{ 1, 2, { 1.0f, 2.0f } };
        static_assert( std::is_same_v< decltype( feng::mean( f12 ) ), double > );
        static_assert( std::is_same_v< decltype( feng::variance( f12 ) ), double > );
        static_assert( std::is_same_v< decltype( feng::standard_deviation( f12 ) ), double > );
        check( feng::mean( f12 ) == 1.5, "E10 mean(1x2 float)" );
        check( feng::variance( f12 ) == 0.25, "E10 variance(1x2 float)" );
        check( feng::standard_deviation( f12 ) == std::sqrt( 0.5 ), "E10 std(1x2 float)" );

        feng::matrix<double> const d12{ 1, 2, { 1.0, 2.0 } };
        check( feng::mean( d12 ) == 1.5, "E10 mean(1x2 double)" );
        check( feng::variance( d12 ) == 0.25, "E10 variance(1x2 double)" );
        check( feng::standard_deviation( d12 ) == std::sqrt( 0.5 ), "E10 std(1x2 double)" );
    }

    // ---- E11: C9 conv same-mode 1x1 kernel (scaling; no abort, asserts live) ----
    {
        feng::matrix<double> const A{ 2, 2, { 1.0, 2.0, 3.0, 4.0 } };
        feng::matrix<double> const K{ 1, 1, { 0.5 } };
        feng::matrix<double> const C = feng::conv( A, K, std::string{ "same" } );
        check( C.row() == 2 && C.col() == 2, "E11 shape" );
        check( C[0][0] == 0.5 && C[0][1] == 1.0 && C[1][0] == 1.5 && C[1][1] == 2.0, "E11 1x1 scaling" );

        feng::matrix<double> const B{ 2, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const K12{ 1, 2, { 1.0, 1.0 } };
        feng::matrix<double> const C12 = feng::conv( B, K12, std::string{ "same" } );
        check( C12[0][0] == 1.0 && C12[0][1] == 3.0 && C12[0][2] == 5.0
            && C12[1][0] == 4.0 && C12[1][1] == 9.0 && C12[1][2] == 11.0, "E11 rb==1,cb==2" );

        feng::matrix<double> const D{ 3, 2, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const K21{ 2, 1, { 1.0, 1.0 } };
        feng::matrix<double> const C21 = feng::conv( D, K21, std::string{ "same" } );
        check( C21[0][0] == 1.0 && C21[0][1] == 2.0
            && C21[1][0] == 4.0 && C21[1][1] == 6.0
            && C21[2][0] == 8.0 && C21[2][1] == 10.0, "E11 rb==2,cb==1" );
    }

    // ---- E12: C10 rref square system (no abort, asserts live) ----
    {
        feng::matrix<double> const m{ 2, 2, { 2.0, 0.0, 0.0, 3.0 } };
        auto const r = feng::rref( m );
        check( r.has_value(), "E12 rref square has value" );
        if ( r.has_value() )
        {
            double const want[2][2] = { { 1.0, 0.0 }, { 0.0, 1.0 } };
            bool ok = true;
            for ( unsigned long i = 0; i != 2; ++i )
                for ( unsigned long j = 0; j != 2; ++j )
                    ok = ok && std::abs( ( *r )[i][j] - want[i][j] ) < 1.0e-10;
            check( ok, "E12 rref(diag{2,3}) == I" );
        }

        feng::matrix<double> const sing{ 2, 2, { 1.0, 2.0, 2.0, 4.0 } };
        check( !feng::rref( sing ).has_value(), "E12 rref singular -> nullopt" );

        feng::matrix<double> const wide{ 2, 3, { 1.0, 0.0, 2.0, 0.0, 1.0, 3.0 } };
        auto const rw = feng::rref( wide );
        check( rw.has_value() && ( *rw )[0][0] == 1.0 && ( *rw )[1][1] == 1.0 && ( *rw )[0][2] == 2.0 && ( *rw )[1][2] == 3.0, "E12 rref wide regression" );
    }

    // ---- E13: P2 cholesky bool + PD guard ----
    {
        feng::matrix<double> a;
        feng::matrix<double> const npd{ 2, 2, { 1.0, 2.0, 2.0, 1.0 } };
        check( feng::cholesky_decomposition( npd, a ) == false, "E13 non-PD -> false" );
        check( a.row() == 2 && a.col() == 2 && a[0][0] == 1.0 && a[1][0] == 2.0 && a[1][1] == 1.0, "E13 non-PD leaves a defined (no NaN)" );

        feng::matrix<double> b;
        feng::matrix<double> const pd{ 2, 2, { 4.0, 2.0, 2.0, 3.0 } };
        check( feng::cholesky_decomposition( pd, b ) == true, "E13 PD -> true" );
        check( b[0][0] == 2.0 && b[0][1] == 0.0 && b[1][0] == 1.0 && std::abs( b[1][1] - std::sqrt( 2.0 ) ) < 1.0e-12, "E13 PD factor" );
        double const p00 = b[0][0] * b[0][0] + b[0][1] * b[0][1];
        double const p01 = b[0][0] * b[1][0] + b[0][1] * b[1][1];
        double const p11 = b[1][0] * b[1][0] + b[1][1] * b[1][1];
        check( std::abs( p00 - 4.0 ) < 1.0e-10 && std::abs( p01 - 2.0 ) < 1.0e-10 && std::abs( p11 - 3.0 ) < 1.0e-10, "E13 PD a.a^T ~ m" );

        feng::matrix<double> c;
        feng::matrix<double> const pss{ 2, 2, { 1.0, 1.0, 1.0, 1.0 } };
        check( feng::cholesky_decomposition( pss, c ) == false, "E13 PSD-singular -> false" );

        feng::matrix<double> z;
        feng::matrix<double> const zero{ 1, 1, { 0.0 } };
        check( feng::cholesky_decomposition( zero, z ) == false, "E13 1x1 {0} -> false" );

        feng::matrix<double> o;
        feng::matrix<double> const one{ 1, 1, { 4.0 } };
        check( feng::cholesky_decomposition( one, o ) == true && o[0][0] == 2.0, "E13 1x1 {4} -> true, 2.0" );
    }

    if ( failures != 0 )
    {
        std::printf( "E10_E13: %d FAILURES\n", failures );
        return 1;
    }
    std::printf( "PASS\n" );
    return 0;
}
