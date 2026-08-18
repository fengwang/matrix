#include "../../matrix.hpp"
#include <cstdio>
int main()
{
    feng::matrix<double> const A{ 3, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 } };
    feng::matrix<double> const K{ 2, 2, { 1.0, 1.0, 1.0, 1.0 } };
    feng::matrix<double> const C = feng::conv( A, K, std::string{ "same" } );
    std::printf( "same 3x3 = [3][3]:\n" );
    for ( unsigned long r = 0; r < C.row(); ++r )
        for ( unsigned long c = 0; c < C.col(); ++c )
            std::printf( "%8.4f ", C[r][c] );
    std::printf( "\n" );
    feng::matrix<double> const F = feng::conv( A, K, std::string{ "full" } );
    std::printf( "full = [%lu][%lu]:\n", F.row(), F.col() );
    for ( unsigned long r = 0; r < F.row(); ++r )
        for ( unsigned long c = 0; c < F.col(); ++c )
            std::printf( "%8.4f ", F[r][c] );
    std::printf( "\n" );
    return 0;
}
