// S9-R1, D-034: discarding the bool of a writer is diagnosed; under -Werror=unused-result it is an error.
// flags: -Werror=unused-result
// expect: nodiscard
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<double> const m{ 2, 2, 1.0 };
    m.save_as_npy( "s9_nodiscard_save.npy" );
}
