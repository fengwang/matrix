// S9-R2, D-034: svd_factor of an integral matrix fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 2, 1 };
    auto f = feng::svd_factor( a );
    (void)f;
}
