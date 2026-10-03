// S7-R2, D-027: det of an integral matrix fails the linalg_element constraint (S9-R2, D-034).
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 3, 1 };
    auto d = a.det();
    (void)d;
}
