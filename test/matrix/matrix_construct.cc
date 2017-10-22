#include <f/matrix/matrix.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>

int main()
{

    const unsigned long r = 10;
    const unsigned long r0 = 3;
    const unsigned long r1 = 7;
    const unsigned long c = 13;
    const unsigned long c0 = 2;
    const unsigned long c1 = 11;

    auto m = f::rand<double>( r, c );

    std::cout << "Generated random matrix is :\n" << m << "\n";

    f::matrix<double,124> g( m, r0, r1, c0, c1 );
    std::cout << "Subset (" << r0 << ", " << r1 << ") (" << c0 << ", " << c1 << ") is :\n" << g << "\n";

    {
        std::cout << "Upper diagonal:\n";
        f::matrix<double> mat{ 5, 7 };
        auto generator = [](){ double val = 0.0; return [=]() mutable { val += 1.0; return val; }; }();
        std::generate( mat.begin(), mat.end(), generator );
        std::copy( mat.upper_diag_begin(2), mat.upper_diag_end(2), std::ostream_iterator<double>( std::cout, "\t" ) );
        std::cout << "\n\n\n";
        std::cout << mat << "\n";
    }
    {
        auto const& generator = [](){ double val = 0.0; return [=]() mutable { val += 1.0; return val; }; }();
        f::matrix<double> mat{5,7};
        std::fill( mat.begin(), mat.end(), 0.0 );
        std::generate( mat.upper_diag_rbegin(3), mat.upper_diag_rend(3), generator );
        std::cout << "\n\n" << mat << "\n";
    }

    return 0;
}

