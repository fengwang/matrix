// S9-R2, D-034: the legacy singular_value_decomposition and svd of an integral matrix fail the linalg_element constraint.
// expect: linalg_element
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<int> const a{ 3, 2, 1 };
    auto f = feng::singular_value_decomposition( a );
    auto g = feng::svd( a );
    (void)f;
    (void)g;
}
