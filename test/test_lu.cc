#include <iostream>
#include <algorithm>
#include <matrix.hpp>
#include <vg.hpp> //variate generator

int main()
{
    using namespace dynamic;
    using namespace std;
    using namespace vg;


    const unsigned long n = 7;
    matrix<double> A( n, n );

    variate_generator<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*n, A.begin() );

    cout << "\nA= \n" << A;
    lu_decomposition<matrix<double> > ld( A );
    std::cout << "\nlower is \n " << ld.lower();
    std::cout << "\nupper is \n " << ld.upper();

    std::cout << "\nl*u-A= \n" << ld.lower() * ld.upper() - A << "\n";

    return 0;
}

