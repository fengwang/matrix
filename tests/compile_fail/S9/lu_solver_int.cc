// S9-R2, D-034: the legacy lu_solver of an integral matrix fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 3, 1 };
    feng::matrix<int> const b{ 3, 1, 1 };
    auto x = feng::lu_solver( a, b );
    (void)x;
}
