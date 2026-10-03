// S4-R3: make_mutable_view of a const owner is deleted (F06).
// expect: deleted
// expect-gcc: use of deleted function 'feng::mutable_matrix_view<Type, Alloc> feng::make_mutable_view(const matrix<Type, Alloc>&,
// expect-clang: call to deleted function 'make_mutable_view'
#include "../../../matrix.hpp"

#include <memory>
#include <utility>

int main()
{
    using range = std::pair<std::size_t, std::size_t>;
    feng::matrix<double> m{ 3, 3 };
    feng::matrix<double> const& cm = m;
    (void)cm;
    auto v = feng::make_mutable_view( cm, { 0, 1 }, { 0, 1 } );
    (void)v;
}
