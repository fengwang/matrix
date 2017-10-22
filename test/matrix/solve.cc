#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/lu_solver.hpp>
#include <f/variate_generator/variate_generator.hpp>

#include <iostream>

int main()
{
    f::matrix<double> fake_a( 6, 6 );
    f::matrix<double> fake_b( 1, 36 );
    f::variate_generator<double> vg( -2, 2 );
    std::generate( fake_a.begin(), fake_a.end(), vg );
    std::generate( fake_b.begin(), fake_b.end(), vg );

    fake_a.save_as( "a.txt" );
    fake_b.save_as( "b.txt" );

    f::matrix<double> _A( 6, 6 );
    f::matrix<double> B( 36, 1 );

    _A.load( "a.txt" );
    B.load( "b.txt" );

    f::matrix<double> A( 36, 36 );

    for ( unsigned long a_row = 0; a_row != 6; ++a_row )
    {
        for ( unsigned long offset = 0; offset != 6; ++offset )
        {
            std::copy( _A.row_begin(a_row), _A.row_end(a_row), A.row_begin( a_row*6 + offset ) + offset*6 );
        }
    }

    std::cout << "_A = \n" << _A << "\n";
    std::cout << "A = \n" << A << "\n";
    std::cout << "B = \n" << B << "\n";

    //f::matrix<double> X( 36, 1 );
    //f::lu_solver( A, X, B );
    auto const& X = A.inverse() * B.transpose();
    std::cout << "X = \n" << X << "\n";
    std::cout << "A*X - B = \n" << A * X - B << "\n"; 

    std::copy( X.begin(), X.end(), _A.begin() );
    _A.save_as( "x.txt" );

    return 0;
}




