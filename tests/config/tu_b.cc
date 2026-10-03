// S9-R3, D-033: the other half of the two-TU program of `tools/check.sh config`; it uses a matrix.
#include "../../matrix.hpp"

#include <cstdint>

std::uint_least64_t tu_b_parallel_mode() { return feng::parallel_mode; }

double tu_b_trace_of_sum()
{
    feng::matrix<double> a{ 3, 3, 1.0 };
    feng::matrix<double> b{ 3, 3, 3.0 };
    feng::matrix<double> const c = a + b;
    return c( 0, 0 ) + c( 1, 1 ) + c( 2, 2 );
}
