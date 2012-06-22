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

    const unsigned long n = 3;
    matrix<double> A( n, n );

    variate_generator<double> v(-1.0, 1.0);
    copy( v.begin(), v.begin()+n*n, A.begin() );
    for( unsigned long i = 1; i < n; ++i )
        copy( A.upper_diag_begin(i), A.upper_diag_end(i), A.lower_diag_begin(i) );

    matrix<double> Q, D;

    householder( A, Q, D );

    std::cout << "\nQ*Q^T= \n" << Q * Q.transpose();
    std::cout << "\nA - Q*D*Q^T= \n" << A - Q * D * Q.transpose();

    std::cout << "\nQ=\n" << Q;
    std::cout << "\nD=\n" << D;

    return 0;
}

