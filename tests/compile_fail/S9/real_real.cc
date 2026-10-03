// S9-R2, D-034: real of a real matrix fails the ComplexMatrix constraint.
// expect: ComplexMatrix
// expect-gcc: constraints not satisfied
// expect-clang: does not satisfy
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<double> const a{ 2, 2, 1.0 };
    auto c = feng::real( a );
    (void)c;
}
