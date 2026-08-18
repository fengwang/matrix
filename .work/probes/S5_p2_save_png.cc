// S5 pre/post E15 probe: save_as_png to a guaranteed-unwritable path.
// Pre-fix: null FILE* UB (expected crash: SIGSEGV / non-zero exit).
// Post-fix: silent no-op, exit 0, prints PASS E15.
// Build: g++ -std=c++20 -DPARALLEL -O1 -o .work/evidence/seed_S5_p2 .work/probes/S5_p2_save_png.cc
#include "../../matrix.hpp"

#include <cstdio>
#include <filesystem>

int main()
{
    feng::matrix< double > const m{ 4, 4, 1.0 };
    bool const ok = m.save_as_png( "/nonexistent_dir_s5/x.png" );
    std::printf( "save_as_png unwritable path returned %d, no crash\n", int( ok ) );

    // positive control: a writable path must still produce a PNG (guard must not break happy path)
    bool const ok2 = m.save_as_png( ".work/evidence/s5_positive_control.png" );
    bool const exists = std::filesystem::exists( ".work/evidence/s5_positive_control.png" );
    if ( !ok2 || !exists )
    {
        std::printf( "FAIL E15: positive control (writable path) broken\n" );
        return 1;
    }
    std::printf( "PASS E15\n" );
    return 0;
}
