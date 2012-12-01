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

    vg::variate_generator<double> v(-1.0, 1.0);
    //copy( v.begin(), v.begin()+n*n, A.begin() );
    //generate( A.begin(), A.end(), v );
    generate( A.begin(), A.end(), vg::variate_generator<double>( -1.0, 1.0 ) );

    cout << "\nA= \n" << A;
    //lu_decomposition<matrix<double> > ld( A );
    lu_decomposition( A, L, U );
    std::cout << "\nlower is \n " << L;
    std::cout << "\nupper is \n " << U;

    std::cout << "\nl*u-A= \n" << L*U - A << "\n";

    return 0;
}

