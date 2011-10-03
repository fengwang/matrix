#include <iostream>
#include <algorithm>
#include <matrix.hpp>
#include <vg.hpp> //variate generator

int main()
{
    using namespace dynamic;
    using namespace std;
    using namespace vg;


    const unsigned long n = 15;
    const unsigned long m = 10;
    matrix<double> A( n, n );
    matrix<double> x( n, m );
    matrix<double> b( n, m );

    variate_generator<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*n, A.begin() );
    copy( v.begin(), v.begin()+n*m, x.begin() );
    b = A * x;

    cout << "\nA= \n" << A;
    cout << "\nb= \n" << b;
    cout << "\nx= \n" << x;

    matrix<double> x_ = gaussian_elimination()( A, b );
    cout << "\nx_= \n" << x_;

    return 0;
}

