// S9-R1, D-034: discarding a pure function's result, free or member, is diagnosed; under -Werror=unused-result it
// is an error.
// flags: -Werror=unused-result
// expect: nodiscard
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<double> const a{ 2, 3, 1.0 };
    feng::transpose( a );
    a.transpose();
}
