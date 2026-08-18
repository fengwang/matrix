// S4 pre-flight probe p3 (C10, pre-existing row>col evidence).
// rref on an OVER-DETERMINED system (row > col) with asserts OFF (NDEBUG, release semantics)
// under ASan. The algorithm loops i over range(row) and dereferences col_begin(i) for
// i >= col — a strided read one element past the end for 3x2. This is PRE-EXISTING UB
// reachable in release builds before the C10 fix; the probe records the before state so
// the after state can be shown byte-identical (the fix changes only the precondition).
// Build (release semantics + ASan): g++ -std=c++20 -DNDEBUG -DPARALLEL -O1 -fsanitize=address -o .work/probe_s4_p3 .work/probes/S4_p3_wide_asan.cc

#include "../../matrix.hpp"

#include <cstdio>

using feng::matrix;

int main()
{
    matrix<double> const m{ 3, 2, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
    auto const r = feng::rref( m );
    if ( !r.has_value() )
    {
        std::printf( "rref 3x2 (row>col): nullopt\n" );
        return 0;
    }
    for ( std::size_t i = 0; i < r->row(); ++i )
        for ( std::size_t j = 0; j < r->col(); ++j )
            std::printf( "%g ", ( *r )[ i ][ j ] );
    std::printf( "\n" );
    return 0;
}
