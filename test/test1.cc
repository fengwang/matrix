#include <matrix.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>

int main()
{
    feng::matrix<int> a( 3, 4, 5 ); 

    a(1,2) = 1;
    a(0,1) = a(1,2);

    auto b = a.to_vector();

    std::copy( b.begin(), b.end(), std::ostream_iterator<int>(std::cout, "\n"));

    return 0; 
}
