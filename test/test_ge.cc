#include <matrix.hpp>
#include <vg.hpp> //variate generator

#include <algorithm>
#include <iostream>

int main()
{
    using namespace feng;
    using namespace std;

    const unsigned long n = 15;
    const unsigned long m = 3;
    matrix<double> A( n, n );
    matrix<double> x( n, m );
    matrix<double> b( n, m );

    vg::variate_generator<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*n, A.begin() );
    copy( v.begin(), v.begin()+n*m, x.begin() );
    b = A * x;

    cout << "\nA= \n" << A;
    cout << "\nb= \n" << b;
    cout << "\nx= \n" << x;

    //matrix<double> x_ = gauss_jordan_elimination( A, b );
    matrix<double> x_;
    gauss_jordan_elimination( A, x_, b );
    cout << "\nx_= \n" << x_;

    cout << "\nx-x_ is \n" << x-x_ << "\n";

    return 0;
}

