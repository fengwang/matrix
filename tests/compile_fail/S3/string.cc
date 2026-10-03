// S3-R3: std::string has a throwing copy constructor, so it is not a matrix_element (D-012).
// expect: matrix_element
// expect-gcc: required for the satisfaction of 'matrix_element<Type>'
// expect-clang: does not satisfy 'matrix_element'
#include <string>

#include "../../../matrix.hpp"

int main()
{
    feng::matrix<std::string> m;
    (void)m;
}
