// S9-R2, D-034: det of an integral matrix fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 3, 1 };
    auto d = feng::det( a );
    (void)d;
}
