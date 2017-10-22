#include <f/matrix/matrix.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>

void test_lap( unsigned long n )
{
    f::matrix<double> A{n , n};
    std::fill( A.begin(), A.end(), 0.0 );
    std::fill( A.diag_begin(1), A.diag_end(1), -1.0 );
    std::fill( A.diag_begin(0), A.diag_end(0), 2.0 );
    std::fill( A.diag_begin(-1), A.diag_end(-1), -1.0 );

    std::cout << "A"<< n << "=\n" << A.inverse() << std::endl << std::endl;

}

void fill_x( f::matrix<double>& A, unsigned long idx )
{
    assert( idx > 0 );
    assert( A.row() == A.col() );
    assert( idx + idx <= A.row() );

    double start = 1.0 * idx * idx / (A.row()+1);
    double stride = 1.0 * idx / (A.row()+1);

    unsigned long const col_index = idx-1;
    unsigned long const row_start = A.row() - idx;
    unsigned long const row_end = idx - 1;

    for ( std::int64_t r = row_start; r != row_end; --r )
    {
        A[r][col_index] = start;

        //A[col_index][r] = start;
        //A[r][A.col()-1-col_index] = start;
        //A[A.col()-1-col_index][r] = start;

        start += stride;
    }
    A[row_end][row_end] = start;


}

void fill( unsigned long n )
{
    f::matrix<double> A{ n, n };
    std::fill( A.begin(), A.end(), 0.0 );

    for ( unsigned long idx = 1; idx != (n+2)/2; ++idx )
        fill_x( A, idx );


    for ( unsigned long idx = 1; idx != A.row(); ++idx )
        std::copy( A.lower_diag_begin(idx), A.lower_diag_end(idx), A.upper_diag_begin(idx) );

    for ( unsigned long idx = 1; idx != A.row(); ++idx )
        std::copy( A.upper_anti_diag_begin(idx), A.upper_anti_diag_end(idx), A.lower_anti_diag_begin(idx) );

    if ( n&1 )
        A[n/2][n/2] = (n+1)/4.0;

    std::cout << "A"<< n << "=\n" << A << std::endl << std::endl;
}



int main()
{
    using namespace f;

    unsigned long n = 7;



    auto A = f::rand<double>( n, n );

    auto B = A.inverse() * A;

    //std::for_each( B.begin(), B.end(), [](double& v){ if (std::abs(v) < 1.0-10){v = 0.0;} } );

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            if ( std::abs(B[r][c]) < 1.0e-10 )
                B[r][c] = 0.0;
        }

    std::cout << "\nA^{-1}*A = \n" << B  << "\n";

    std::cout << "\noutput again\n";
    std::copy( B.begin(), B.end(), std::ostream_iterator<double>(std::cout, "\t" ));


    for ( unsigned long index = 2; index != 20; ++index )
    {
        test_lap(index);
        fill( index );
    }

    return 0;
}

