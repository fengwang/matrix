// S9-R2, D-034: inverse of an integral matrix, member and free, fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 3, 1 };
    auto m = a.inverse();
    auto f = feng::inverse( a );
    (void)m;
    (void)f;
}
