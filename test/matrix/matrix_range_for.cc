#include <f/matrix/matrix.hpp>

#include <vector>
#include <iostream>
#include <algorithm>

int main()
{
    if ( 1 )
    {
        f::matrix<double> A( 4, 5 );

        std::fill( A.begin(), A.end(), -1.0 );
        std::fill( A.anti_diag_begin(), A.anti_diag_end(), 3.4 );

        std::cout << A << std::endl;

        for ( auto& a : A )
            a += 1.0;

        std::cout << A << std::endl;

    }

    return 0;
}

