// S9-R1, D-034: discarding the bool of a loader is diagnosed; under -Werror=unused-result it is an error.
// flags: -Werror=unused-result
// expect: nodiscard
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<double> m;
    m.load_npy( "./images/64.npy" );
}
