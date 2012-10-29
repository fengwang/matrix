#include <matrix.hpp>
#include <iostream>

int main()
{
    feng::matrix<int> m( 3, 3 );

    m.import( 1, 2, 3, 
              4, 5, 6, 
              7, 8, 9 );

    std::cout << m << std::endl;

    m.import(   m.diag_rbegin(), 
                0,
                   -1,
                       -2 );

    std::cout << m << std::endl;

    m.import( m.begin(),
               1,  2, -3, 
               4, -5,  6, 
              -7,  8,  9 );

    std::cout << m << std::endl;

    m.import(   m.upper_diag_rbegin(1), 
                /**/  99, /**/
                /**/ /**/  66
                /**/ /**/ /**/);
                     

    std::cout << m << std::endl;

    return 0;
}

