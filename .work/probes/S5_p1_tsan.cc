// S5 pre-fix TSan probe (executable RED for C11): two threads fill matrices
// concurrently with rand -> data race on the global srand/rand state.
// Build: g++ -std=c++20 -DPARALLEL -fsanitize=thread -O1 -o .work/evidence/seed_S5_p1 .work/probes/S5_p1_tsan.cc
// Expectation pre-fix: TSan "WARNING: ThreadSanitizer: data race". Post-fix: no report.
#include "../../matrix.hpp"

#include <cstdio>
#include <thread>

int main()
{
    std::thread t1 = std::thread( []()
    {
        for ( int i = 0; i != 400; ++i )
            feng::matrix< double > const m = feng::rand< double >( 16, 16, 0 );
    } );
    std::thread t2 = std::thread( []()
    {
        for ( int i = 0; i != 400; ++i )
            feng::matrix< double > const m = feng::rand< double >( 16, 16, 0 );
    } );
    t1.join();
    t2.join();
    std::printf( "T SAN CLEAN (no race reported before this line)\n" );
    return 0;
}
