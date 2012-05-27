#include <vg.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>
#include <matrix/sparse_matrix.hpp>
#include <valarray>

int main()
{
    using namespace feng;

    unsigned int const n = 7;
    vg::variate_generator<double> v;
    matrix<double> A(n, n);
    matrix<double> B(n, n);
    matrix<double> C(n, n);
    sparse_matrix<double> C_(n, n);

    std::copy( v.begin(), v.begin()+n*n, A.begin() );
    std::copy( v.begin(), v.begin()+n*n, B.begin() );

    vg::variate_generator<int> N( 0, n );

    for ( std::size_t i = 0; i < 4; ++i )
    {
        auto const r = N();
        auto const c = N();
        C[r][c] = v();
        C_(r,c) = C[r][c];
    }

    //std::cerr << "\nA=\n" << A << "\n";
    //std::cerr << "\nC=\n" << C << "\n";
    //std::cerr << "\nC'=\n" << C_ << "\n";

#if 1
    std::cout << "\nA*C = \n" << A*C;
    std::cout << "\nA*C' = \n" << A*C_;
#endif
#if 1
    std::cout << "\nC*A = \n" << C*A;
    std::cout << "\nC'*A = \n" << C_*A;
#endif

#if 1
    std::cout << "A*C - A*C' = \n";
    std::cout << A*C - A*C_ << std::endl;

    std::cout << "\nC*A = \n" << C*A;
    std::cout << "\nC'*A = \n" << C_*A;

    std::cout << "C*A - C'*A = \n";
    std::cout << C*A - C_*A << std::endl;

    //std::cout << "C'*C' - C*C = \n";
    //std::cout << C_*C_ - C*C << std::endl;
#endif

    return 0;
}



