#include <template/oscillate_function.hpp>

#include <iostream>
#include <cmath>
#include <functional>

int main()
{
    auto const& F = []( double x, double y) { return std::sin(x)*std::cos(y); };

    auto const& f = []( double x ) { return x + 0.1; }; 

    using namespace feng;

    oscillate_function<0, double, double, double> const osf( F, f );

    std::cout << osf( 1.0, 2.0 ) << "\n";
    std::cout << F( 1.1, 2.0 ) << "\n";

    oscillate_function< 1, double, double, double > const osf2( F, f );

    std::cout << osf2( 1.0, 2.0 ) << "\n";
    std::cout << F( 1.0, 2.1 ) << "\n";

    auto const& FF = []( double x, double y, double z, double a ) { return x * std::cos(y) - std::sin(z)/std::exp(a); };

    oscillate_function< 0, double, double, double, double, double > const o0( FF, f );
    std::cout << o0( 1.0, 2.0, 3.0, 4.0 ) - FF( 1.1, 2.0, 3.0, 4.0 ) << "\n";

    oscillate_function< 1, double, double, double, double, double > const o1( FF, f );
    std::cout << o1( 1.0, 2.0, 3.0, 4.0 ) - FF( 1., 2.10, 3.0, 4.0 ) << "\n";

    oscillate_function< 2, double, double, double, double, double > const o2( FF, f );
    std::cout << o2( 1.0, 2.0, 3.0, 4.0 ) - FF( 1., 2.0, 3.10, 4.0 ) << "\n";

    oscillate_function< 3, double, double, double, double, double > const o3( FF, f );
    std::cout << o3( 1.0, 2.0, 3.0, 4.0 ) - FF( 1., 2.0, 3.0, 4.10 ) << "\n";







    return 0;
}

