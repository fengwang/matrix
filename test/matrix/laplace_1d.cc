#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/singular_value_decomposition.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace f;

void fill_x( matrix<double>& A, unsigned long idx )
{
    assert( idx > 0 );
    assert( A.row() == A.col() );
    assert( idx + idx <= A.row() );

    double start = 1.0 * idx * idx / (A.row()+1);
    double stride = 1.0 * idx / (A.row()+1);

    unsigned long const col_index = idx-1;
    unsigned long const row_start = A.row() - idx;
    unsigned long const row_end = idx - 1;

    for ( std::int64_t r = row_start; r != static_cast<std::int64_t>(row_end); --r )
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
    matrix<double> A{ n, n };
    std::fill( A.begin(), A.end(), 0.0 );

    for ( unsigned long idx = 1; idx != (n+2)/2; ++idx )
        fill_x( A, idx );


    for ( unsigned long idx = 1; idx != A.row(); ++idx )
        std::copy( A.lower_diag_begin(idx), A.lower_diag_end(idx), A.upper_diag_begin(idx) );

    for ( unsigned long idx = 1; idx != A.row(); ++idx )
        std::copy( A.upper_anti_diag_begin(idx), A.upper_anti_diag_end(idx), A.lower_anti_diag_begin(idx) );

    if ( n&1 )
        A[n/2][n/2] = (n+1)/4.0;

    A.save_as_pgm( "bad_laplace_1d_inv.pgm" );
    A.save_as( "bad_laplace_1d_inv.txt" );
}



int main()
{
    unsigned long const n = 2048;

    fill( n );

    matrix<double> A{ n, n };
    std::fill( A.begin(), A.end(), 0.0 );
    std::fill( A.diag_begin(), A.diag_end(), -2.0 );
    std::fill( A.diag_begin(1), A.diag_end(1), 1.0 );
    std::fill( A.diag_begin(-1), A.diag_end(-1), 1.0 );
    A[0][0] = 2.0;
    A[0][1] = -5.0;
    A[0][2] = 4.0;
    A[0][3] = -1.0;
    A[n-1][n-1] = 2.0;
    A[n-1][n-2] = -5.0;
    A[n-1][n-3] = 4.0;
    A[n-1][n-4] = -1.0;

    {
        matrix<double> const& B = svd_inverse(A);
        B.save_as_pgm( "laplace_1d_inv.pgm" );
        B.save_as( "laplace_1d_inv.txt" );

        matrix<double> const& I =  A*B;

        std::cout << "Error of direct inverse is " << std::inner_product( I.begin(), I.end(), I.begin(), 0.0 ) << std::endl;
    }

    return 0;
}

