// S9-R2, D-034: the legacy pinv and svd_inverse of an integral matrix fail the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 2, 1 };
    auto p = feng::pinv( a );
    auto q = feng::svd_inverse( a );
    (void)p;
    (void)q;
}
