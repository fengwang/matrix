// S4 pre-flight probe p0c (C8, pre-fix compile evidence): variance(matrix<int>...) must
// currently be a hard compile error — mean(matrix<int>) returns unsigned long (int/uint64
// integer division), so `m - mean(m)` has no viable operator- overload.
// Build: g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4_p0c .work/probes/S4_p0c_variance_int.cc
// Pre-fix expectation: compile failure (recorded as TDD red). Post-fix: compiles, value 0.25.

#include "../../matrix.hpp"

#include <cstdio>

using feng::matrix;

int main()
{
    matrix<int> const mi{ 1, 2, { 1, 2 } };
    auto const v = feng::variance( mi );
    std::printf( "variance int 1x2 {1,2} = %g\n", double( v ) );
    return 0;
}
