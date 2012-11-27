#include <vg.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>
#include <valarray>

int main()
{
    using namespace feng;

    unsigned int const n = 120;
    vg::vg<double> v;
    matrix<double> A(n, n);
    matrix<double> V(n, 1);
    std::copy( v.begin(), v.begin()+n*n, A.begin() );
    
    auto const lambda = eigen_power_iteration( A, V.begin() );

    std::cout << "the max eigen value is " << lambda << "\n";

    std::cout << "Av - lambda v = \n" << A * V - lambda * V;

    return 0;
}



