#include <f/variate_generator/variate_generator.hpp>
#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/backward_substitution.hpp>

#include <iostream>
#include <algorithm>
#include <numeric>

int main()
{
    using namespace f;

    unsigned int N = 10;

    variate_generator<double> v;

    matrix<double> A(N,N);
    matrix<double> x(N,1);
    matrix<double> x_(N,1);

    std::copy( v.begin(), v.begin()+N*N, A.begin() );
    std::copy( v.begin(), v.begin()+N, x.begin() );

    for ( unsigned int i = 0; i != N; ++i )
        for ( unsigned int j = 0; j != i; ++j )
            //A[j][i] = 0;
            A[i][j] = 0;

    std::cout << "\nA=\n" << A;
    std::cout << "\nx=\n" << x;

    matrix<double> b = A * x;
    backward_substitution( A, x_, b );
    //forward_substitution( A, x_, b );


    std:: cout << "\nAx-b=\n" << A * x_ - b;
    
    return 0;
}
