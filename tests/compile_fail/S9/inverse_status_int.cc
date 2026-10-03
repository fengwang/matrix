// S9-R2, D-034: the status overload inverse( A, out ) on an integral matrix fails the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<long> const a{ 2, 2, 1L };
    feng::matrix<long> out;
    auto st = feng::inverse( a, out );
    (void)st;
}
