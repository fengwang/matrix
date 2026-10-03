// S9-R2, D-034: row_echelon of an integral matrix fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 4, 1 };
    auto r = feng::row_echelon( a );
    (void)r;
}
