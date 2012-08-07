#include <matrix.hpp>
#include <iostream>

int main()
{
    using namespace feng;
    using namespace std;

    std::cout << feng::make_diag( 1.1, 1.2, 1.3, 1.4, 1.5 ) << std::endl;

    std::cout << feng::make_diag( 1, 2, 3, 4, 8, 16.6 ) << std::endl;

    std::cout << feng::diag( feng::magic(5) ) << std::endl;

    std::cout << feng::diag( feng::magic(3), 2 ) << std::endl;

    std::cout << feng::diag( feng::magic(3), -2 ) << std::endl;

    return 0;
}

