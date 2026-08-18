#include "../../matrix.hpp"
int main(){ feng::matrix<double> const m{2,3,{1,2,3,4,5,6}}; feng::matrix<double> a; (void) feng::cholesky_decomposition(m,a); return 0; }
