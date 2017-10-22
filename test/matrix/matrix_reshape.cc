#include <f/matrix/matrix.hpp>

#include <iostream>
#include <algorithm>

int main()
{
    f::matrix<double> A( 4, 5 );

    std::fill( A.begin(), A.end(), -1.0 );
    std::fill( A.anti_diag_begin(), A.anti_diag_end(), 3.4 );

    std::cout << "A=\n";
    std::cout << A << "\n";

    A.reshape( 5, 4 );
    std::cout << "\nafter reshape(5, 4), A = \n" << A << "\n\n";

    A.reshape( 10, 2 );
    std::cout << "\nafter reshape(10, 2), A = \n" << A << "\n\n";

    A.reshape( 1, 20 );
    std::cout << "\nafter reshape(1, 20), A = \n" << A << "\n\n";


    return 0;
}

