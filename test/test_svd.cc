#include <matrix.hpp>
#include <vg.hpp> //variate generator
#include <valarray>
#include <iostream>
#include <algorithm>

int main()
{
    using namespace dynamic;
    using namespace std;
    using namespace vg;


    const unsigned long n = 7;
    const unsigned long m = 7;
    matrix<double> A( n, m );

    variate_generator<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*m, A.begin() );

    cout << "\nA= \n" << A;
    singular_value_decomposition<matrix<double> > svd( A );

    auto u = svd.u();
    auto w = svd.w();
    auto V = svd.v();

    matrix<double> W( n, n );
    copy( &(w[0]), &(w[0])+n, W.diag_begin() ); 

    cout << "\nA - U W V^{T} = \n" << A - u * W * (~V) << "\n";

    return 0;
}

