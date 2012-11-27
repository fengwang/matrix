#include <matrix.hpp>
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

    auto m = feng::rand<double>( r, c );

    std::cout << "Generated random matrix is :\n" << m << "\n";

    feng::matrix<double,124> g( m, r0, r1, c0, c1 );
    std::cout << "Subset (" << r0 << ", " << r1 << ") (" << c0 << ", " << c1 << ") is :\n" << g << "\n";  

    return 0; 
}

