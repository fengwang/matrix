#include <matrix.hpp>
#include <iostream>
#include <algorithm>
#include <iterator>

int main()
{
    feng::matrix<int> m( 12, 9 );
    int i = 0;

    std::for_each( m.begin(), m.end(), [&i](int& v){ v = i++; } );

    std::cout << "M=\n" << m << "\n";

    std::cout << "\n3th upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_begin(3), m.upper_cross_diag_end(3), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nconst 7th upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_cbegin(7), m.upper_cross_diag_cend(7), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\n5th reverse upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_rbegin(5), m.upper_cross_diag_rend(5), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nconst 6th reverse upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_crbegin(6), m.upper_cross_diag_crend(6), std::ostream_iterator<int>(std::cout, " " ));

    return 0;
}

