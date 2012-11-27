#include <least_square_fit.hpp>
#include <vector>
#include <iostream>
#include <functional>
#include <cmath>
#include <algorithm>
#include <iterator>

using namespace feng;
using namespace std;

int main()
{
    // y = a0 + a1 * cos(x) + a2 * sin(x) + a3 * tan(x) + a4 * exp(x);
    matrix<double> A; // A[m][1]
    matrix<double> X; // X[n][m]
    matrix<double> Y; // Y[n][1]

    A.load( "A.dat" );
    X.load( "X.dat" );
    Y.load( "Y.dat" );

    matrix<double> a( A.row(), A.col() );

    linear_lease_square_fit( X, Y.begin(), a.begin() );

    std::cout << "Actual parameter A is :\n" << A << "\n";
    std::cout << "Fitted parameter A' is :\n" << a << "\n";
    std::cout << "A - A' is :\n" << A - a << "\n";

    return 0;
}

