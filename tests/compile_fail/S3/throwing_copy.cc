// S3-R3: a type whose copy constructor is not noexcept is not a matrix_element (D-012).
// expect: matrix_element
// expect-gcc: required for the satisfaction of 'matrix_element<Type>'
// expect-clang: does not satisfy 'matrix_element'
#include "../../../matrix.hpp"

struct throwing_copy
{
    throwing_copy() noexcept = default;
    throwing_copy( throwing_copy const& ) noexcept( false ) {}
    throwing_copy( throwing_copy&& ) noexcept = default;
    throwing_copy& operator=( throwing_copy const& ) noexcept = default;
    throwing_copy& operator=( throwing_copy&& ) noexcept = default;
};

int main()
{
    feng::matrix<throwing_copy> m;
    (void)m;
}
