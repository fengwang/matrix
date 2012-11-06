#include <matrix.hpp>
#include <iostream>
#include <cmath>

int main()
{
    const unsigned int r = 3;
    const unsigned int c = 4;

    auto const m = feng::rand( r, c );

    std::cout << "rand matrix:\n" << m;

    std::cout << "\napplying sin to matrix: \n" << m.apply( [](double d){ return std::sin(d); } );

    std::cout << "\n\nand the difference with sin(m):\n" << feng::sin(m) - m.apply( [](double d){ return std::sin(d); } ) << "\n";

    return 0; 
}
