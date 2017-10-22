#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/singular_value_decomposition.hpp>

#include <iostream>

int main()
{
    const unsigned int n = 15;
    const unsigned int m = 12;

    f::matrix<double> A = f::rand<double>(n, m);
    f::matrix<double> u;
    f::matrix<double> w;
    f::matrix<double> v;

    if ( 0 == f::singular_value_decomposition( A, u, v, w ) )
    {
        std::cout << "Successfully solved the SVD\n";
        std::cout << "A=\n" << A << "\n\n";
        std::cout << "u=\n" << u << "\n\n";
        std::cout << "w=\n" << w << "\n\n";
        std::cout << "v=\n" << v << "\n\n";

        std::for_each( v.begin(), v.end(), []( auto& val ){ if (std::abs(val) > 1.0e-10) val = 1.0/val; } );

        std::cout <<  w * v.transpose() * u.transpose() * A << "\n\n";
        //std::cout <<  A * w * v.transpose() * u.transpose() << "\n\n";

        std::cout << f::svd_inverse( A ) * A << "\n";

    }
    else
        std::cout << "Failed to solve the svd.\n";

    return 0;
}

