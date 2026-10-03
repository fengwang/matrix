// S5-R5 (PR-7): libFuzzer harness for matrix_details::parse_npy; traps when a failed parse changed a pre-filled
// destination or a successful one is inconsistent (checks in fuzz_checks.hpp). No file I/O.
#include "../../matrix.hpp"
#include "fuzz_checks.hpp"

extern "C" int LLVMFuzzerTestOneInput( std::uint8_t const* data, std::size_t size )
{
    if ( fuzz_checks::check_npy( data, size ) != nullptr ) __builtin_trap();
    return 0;
}
