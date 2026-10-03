// S9-R1, D-034: discarding a factorization is diagnosed; under -Werror=unused-result it is an error.
// flags: -Werror=unused-result
// expect: nodiscard
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<double> const a{ 2, 2, { 2.0, 0.0, 0.0, 2.0 } };
    feng::lu_factor( a );
}
