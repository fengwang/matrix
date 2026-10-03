// S9-R3, D-033: one half of the two-TU program of `tools/check.sh config`. The lane compiles it with
// -DCONFIG_EXPECT_PARALLEL=0|1 next to the configuration macros under test; main checks feng::parallel_mode in this
// TU and in tu_b.cc and the value tu_b computes with a matrix. Exit 0 only when all three hold.
#include "../../matrix.hpp"

#include <cstdint>
#include <cstdio>

#ifndef CONFIG_EXPECT_PARALLEL
#error "compile with -DCONFIG_EXPECT_PARALLEL=0 or 1"
#endif

std::uint_least64_t tu_b_parallel_mode();
double tu_b_trace_of_sum();

int main()
{
    int rc = 0;
    if ( feng::parallel_mode != CONFIG_EXPECT_PARALLEL )
    {
        std::printf( "tu_a: parallel_mode %d, expected %d\n", int( feng::parallel_mode ), int( CONFIG_EXPECT_PARALLEL ) );
        rc = 1;
    }
    if ( tu_b_parallel_mode() != feng::parallel_mode )
    {
        std::printf( "tu_b: parallel_mode %d, tu_a %d\n", int( tu_b_parallel_mode() ), int( feng::parallel_mode ) );
        rc = 1;
    }
    double const t = tu_b_trace_of_sum();
    if ( t != 12.0 )
    {
        std::printf( "tu_b: trace %g, expected 12\n", t );
        rc = 1;
    }
    if ( rc == 0 ) std::printf( "config: ok\n" );
    return rc;
}
