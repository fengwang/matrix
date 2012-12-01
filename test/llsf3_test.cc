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
    matrix<double> Theta; // A[m][1]
    matrix<double> X; // X[n][m]
    matrix<double> Psi; // Y[n][1]

    Theta.load( "A.dat" );
    X.load( "X.dat" );
    Psi.load( "Y.dat" );

    matrix<double> theta( Theta.row(), Theta.col() );

    linear_lease_square_fit( X, Psi.begin(), theta.begin() );

    std::cout << "Actual parameter Theta is :\n" << Theta << "\n";
    std::cout << "Fitted parameter theta is :\n" << theta << "\n";
    std::cout << "Theta - theta is :\n" << Theta - theta << "\n";

    return 0;
}

