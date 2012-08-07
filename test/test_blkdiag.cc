#include <matrix.hpp>
#include <iostream>

int main()
{
    using namespace feng;
    using namespace std;

    std::cout << feng::blkdiag( feng::magic(1), feng::magic(3), feng::magic(5) ) << std::endl;

    return 0;
}

