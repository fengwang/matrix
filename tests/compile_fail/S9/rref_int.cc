// S9-R2, D-034: the legacy rref and gauss_jordan_elimination of an integral matrix fail the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 4, 1 };
    auto r = feng::rref( a );
    auto g = feng::gauss_jordan_elimination( a );
    (void)r;
    (void)g;
}
