#include <matrix.hpp>
#include <iostream>
#include <algorithm>
#include <iterator>

int main()
{
    feng::matrix<int> m( 12, 13 );
    int i = 1;

    std::for_each( m.begin(), m.end(), [&i](int& v){ v = i++; } );

    std::cout << "M=\n" << m << "\n";

    std::cout << "\n3th lower_cross diag of M is \n";
    std::copy( m.lower_cross_diag_begin(3), m.lower_cross_diag_end(3), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nconst 7th lower_cross diag of M is \n";
    std::copy( m.lower_cross_diag_cbegin(7), m.lower_cross_diag_cend(7), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\n5th reverse lower_cross diag of M is \n";
    std::copy( m.lower_cross_diag_rbegin(5), m.lower_cross_diag_rend(5), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nconst 6th reverse lower_cross diag of M is \n";
    std::copy( m.lower_cross_diag_crbegin(6), m.lower_cross_diag_crend(6), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nreverse order of cross diag is\n";
    std::copy( m.cross_diag_crbegin(), m.cross_diag_crend(), std::ostream_iterator<int>(std::cout, " " ));


    std::cout << "\nFilling 0 to the 4th lane\n";
        std::fill( m.lower_cross_diag_begin(4), m.lower_cross_diag_end(4), 0 );

    std::cout << "M=\n" << m << "\n";

    std::cout << "filling -1 to the cross diag\n";
    std::fill( m.cross_diag_begin(), m.cross_diag_end(), -1 );
    std::cout << "M=\n" << m << "\n";

    std::cout << "\n3th upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_begin(3), m.upper_cross_diag_end(3), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nconst 7th upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_cbegin(7), m.upper_cross_diag_cend(7), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\n5th reverse upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_rbegin(5), m.upper_cross_diag_rend(5), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nconst 6th reverse upper_cross diag of M is \n";
    std::copy( m.upper_cross_diag_crbegin(6), m.upper_cross_diag_crend(6), std::ostream_iterator<int>(std::cout, " " ));

    std::cout << "\nreverse order of cross diag is\n";
    std::copy( m.cross_diag_crbegin(), m.cross_diag_crend(), std::ostream_iterator<int>(std::cout, " " ));


    std::cout << "\nFilling 0 to the 4th lane\n";
        std::fill( m.upper_cross_diag_begin(4), m.upper_cross_diag_end(4), 0 );

    std::cout << "M=\n" << m << "\n";

    std::cout << "filling -1 to the cross diag\n";
    std::fill( m.cross_diag_begin(), m.cross_diag_end(), -1 );
    std::cout << "M=\n" << m << "\n";

    return 0;
}

