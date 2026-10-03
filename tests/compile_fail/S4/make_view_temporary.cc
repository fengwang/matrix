// S4-R3: make_view of a temporary owner is a deleted overload (F06).
// expect: deleted
// expect-gcc: use of deleted function 'feng::matrix_view<Type, Alloc> feng::make_view(matrix<Type, Alloc>&&
// expect-clang: call to deleted function 'make_view'
#include "../../../matrix.hpp"

int main()
{
    auto v = feng::make_view( feng::matrix<double>{ 3, 3 }, { 0, 1 }, { 0, 1 } );
    (void)v;
}
