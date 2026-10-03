// S4-R3: the matrix_view constructor from a temporary owner is deleted (F06).
// expect: deleted
// expect-gcc: use of deleted function 'feng::matrix_view<Type, Alloc>::matrix_view(matrix_type&&,
// expect-clang: call to deleted constructor of 'feng::matrix_view<double, std::allocator<double>>'
#include "../../../matrix.hpp"

#include <memory>
#include <utility>

int main()
{
    using range = std::pair<std::size_t, std::size_t>;
    feng::matrix<double> m{ 3, 3 };
    feng::matrix<double> const& cm = m;
    (void)cm;
    feng::matrix_view<double, std::allocator<double>> v( feng::matrix<double>{ 3, 3 }, range{ 0, 1 }, range{ 0, 1 } );
    (void)v;
}
