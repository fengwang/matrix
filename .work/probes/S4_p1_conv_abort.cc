// S4 pre-flight probe p1 (C9, pre-fix evidence): conv "same" with a 1x1 kernel must
// currently ABORT in a debug (assert-live) build — the copy-pasted assert checks `rb > 1`
// twice, rejecting the well-defined 1x1 kernel.
// Build (debug, asserts live): g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4_p1 .work/probes/S4_p1_conv_abort.cc
// Pre-fix expectation: assertion failure + abort (exit 134). Post-fix: prints the result.

#include "../../matrix.hpp"

#include <cstdio>

using feng::matrix;

int main()
{
    matrix<double> const A{ 2, 2, { 1.0, 2.0, 3.0, 4.0 } };
    matrix<double> const K{ 1, 1, { 0.5 } };
    auto const C = feng::conv( A, K, std::string{ "same" } );
    std::printf( "conv 1x1 same: [%g %g; %g %g]\n", C[0][0], C[0][1], C[1][0], C[1][1] );
    std::printf( "PASS S4_p1\n" );
    return 0;
}
