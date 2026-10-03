// S3-R3: bool is not a matrix_element (D-012, D-017).
// expect: matrix_element
// expect-gcc: required for the satisfaction of 'matrix_element<Type>'
// expect-clang: does not satisfy 'matrix_element'
#include "../../../matrix.hpp"

int main()
{
    feng::matrix<bool> m;
    (void)m;
}
