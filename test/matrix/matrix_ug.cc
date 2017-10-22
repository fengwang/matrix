#include <f/matrix/matrix.hpp>

int main()
{
    f::matrix<double> um;

    um.load("./testdata/sto_accr/theory_ug.txt");

    f::matrix<double> ug( um.row(), 2 );

    std::copy( um.col_begin(1), um.col_end(1), ug.col_begin(0) );
    std::copy( um.col_begin(2), um.col_end(2), ug.col_begin(1) );

    ug.reshape( 1, ug.size() );

    ug.save_as( "./testdata/sto_accr/the_ug.txt" );

    return 0;
}

