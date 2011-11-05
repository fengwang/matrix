#include <matrix.hpp>
#include <vg.hpp> //variate generator
#include <valarray>
#include <iostream>
#include <algorithm>

int main()
{
    using namespace feng;
    using namespace std;
    using namespace vg;


    const unsigned long n = 7;
    matrix<double> A( n, n );

    variate_generator<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*n, A.begin() );
    for( unsigned long i = 1; i < n; ++i )
        copy( A.upper_diag_begin(i), A.upper_diag_end(i), A.lower_diag_begin(i) );
    for_each( A.diag_begin(), A.diag_end(), [](double& d) { d += 10; } );
    const auto L = cholesky_decomposition()(A);

    cout << "\nA= \n" << A;
    cout << "\nL= \n" << L;
    cout << "\nA - L L^{T} = \n" << A - L * (~L) << "\n";

    return 0;
}

