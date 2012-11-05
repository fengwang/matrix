#include <matrix.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>

int main()
{
    const unsigned int r = 3;
    const unsigned int c = 4;
    feng::matrix<int> a( r, c, 5 ); 

    auto d = a.data();

    std::cout << "\na.data()=\n";
    std::copy( d, d+r*c, std::ostream_iterator<int>(std::cout, "\t") );
    std::cout << "\na=\n" << a << "\n";

    return 0; 
}
