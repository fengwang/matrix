// S4 pre-flight probe p0b (C8, pre-fix compile question Q1).
// Question: does standard_deviation(matrix<int>...) compile pre-fix?
// The body has two return statements: `typename Mat::value_type{}` (size<=1 branch) and
// std::sqrt(...) which is double for an int matrix if the sum/size division promotes.
// If the two branches deduce different auto return types this TU is a hard compile error.
// Build: g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4_p0b .work/probes/S4_p0b_stddev_int.cc

#include "../../matrix.hpp"

#include <cstdio>

using feng::matrix;

int main()
{
    matrix<int> const mi{ 1, 2, { 1, 2 } };
    auto const s = feng::standard_deviation( mi );
    std::printf( "stddev pre-fix int 1x2 {1,2} = %g\n", double( s ) );
    return 0;
}
