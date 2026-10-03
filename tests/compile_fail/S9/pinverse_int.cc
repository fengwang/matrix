// S9-R2, D-034: pinverse of an integral matrix, status and value forms, fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 2, 1 };
    feng::matrix<int> out;
    auto st = feng::pinverse( a, out );
    auto p = feng::pinverse( a );
    (void)st;
    (void)p;
}
