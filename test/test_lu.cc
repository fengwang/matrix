#include <iostream>
#include <algorithm>
#include <matrix.hpp>
#include <vg.hpp> //variate generator

int main()
{
    using namespace feng;
    using namespace std;

    const unsigned long n = 13;
    matrix<double> A( n, n );
    matrix<double> L( n, n );
    matrix<double> U( n, n );

    vg::vg<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*n, A.begin() );

    cout << "\nA= \n" << A;
    //lu_decomposition<matrix<double> > ld( A );
    lu_decomposition( A, L, U );
    std::cout << "\nlower is \n " << L;
    std::cout << "\nupper is \n " << U;

    std::cout << "\nl*u-A= \n" << L*U - A << "\n";

    return 0;
}

