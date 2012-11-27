#include <iostream>
#include <iomanip>
#include <algorithm>
#include <matrix.hpp>
#include <vg.hpp> //variate generator

int main()
{
    using namespace feng;
    using namespace std;

    const unsigned long n = 110;
    const unsigned long m = 60;
    matrix<double> A( n, m );
    matrix<double> x;
    matrix<double> w;
    matrix<double> v;

    vg::vg<double> vg(-10.0, 10.0);
    copy( vg.begin(), vg.begin()+n*m, A.begin() );

    singular_value_decomposition( A, x, w, v);

    std::cout << "\nx is \n " << x;
    std::cout << "\nw is \n " << w;
    std::cout << "\nv is \n " << v;

    std::cout << "\nx x^t is \n " << ~x * x ;
    std::cout << "\nv v^t is \n " << ~v * v ;

    std::cout.precision(1);
    std::cout << "\nA-x*w*v^t = \n" << A - x * w * (~v);

    return 0;
}

