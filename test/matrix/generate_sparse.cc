#include <f/matrix/matrix.hpp>
#include <f/algorithm/for_each.hpp>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <iomanip>
#include <array>

int main()
{
    using namespace f;

    if (1)
    {
        matrix<double> xxiv;
        xxiv.load("./matrix/stone_xxiv.txt");
        matrix<double> yyiv;
        yyiv.load("./matrix/stone_yyiv.txt");
        auto diff = yyiv - xxiv;
        diff.reshape( diff.size(), 1 );
        diff.save_as( "./matrix/stone_yy_xx_diff_1d.txt" );

        unsigned long const N = xxiv.row();
        unsigned long const elements = N*N * 4;

        std::ofstream ofs_mtx( "./matrix/alpha_beta_gamma_epsilon.mtx" );
        ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
        ofs_mtx.precision( 15 );
        ofs_mtx << N*N << " " << 4*N << " " << elements << "\n";

        auto const& write_record = [&]( unsigned long r, unsigned long c )
        {
            unsigned long const row_index = r * N + c + 1;
            unsigned long const col_index_1 = c+1;
            unsigned long const col_index_2 = N+c+1;
            unsigned long const col_index_3 = N+N+c+1;
            unsigned long const col_index_4 = N+N+N+c+1;
            double const val_1 = xxiv[r][c];
            double const val_2 = 1.0;
            double const val_3 = -yyiv[r][c];
            double const val_4 = -1.0;

            ofs_mtx << row_index << " " << col_index_1 << " " << val_1 << "\n";
            ofs_mtx << row_index << " " << col_index_2 << " " << val_2 << "\n";
            ofs_mtx << row_index << " " << col_index_3 << " " << val_3 << "\n";
            ofs_mtx << row_index << " " << col_index_4 << " " << val_4 << "\n";
        };

        for ( unsigned long r = 0; r != N; ++r )
            for ( unsigned long c = 0; c != N; ++c )
                write_record( r, c );

        ofs_mtx.close();
    }


    if (0)
    {
        std::vector<int> row;
        std::vector<int> col;
        std::vector<int> val;

        std::array<int,2> row_rec;
        std::array<int,2> col_rec;
        std::array<int,2> val_rec;

        int const N = 2048;
        int const elements = N * N * 2;
        row.reserve( elements );
        col.reserve( elements );
        val.reserve( elements );

        auto const& impl = [&]( int r, int c )
        {
            int const A_row = r * N + c;

            row_rec = { A_row, A_row };
            col_rec = { r, N+c };
            val_rec = { 1, -1 };

            for ( unsigned long idx = 0; idx != 2; ++idx )
            {
                row.push_back( row_rec[idx]+1 );
                col.push_back( col_rec[idx]+1 );
                val.push_back( val_rec[idx] );
            }
        };

        for ( int r = 0; r != N; ++r )
            for ( int c = 0; c != N; ++c )
                impl( r, c );

        std::ofstream ofs_row( "row.txt" );
        std::copy( row.begin(), row.end(), std::ostream_iterator<int>( ofs_row, "\n" ) );
        ofs_row.close();

        std::ofstream ofs_col( "col.txt" );
        std::copy( col.begin(), col.end(), std::ostream_iterator<int>( ofs_col, "\n" ) );
        ofs_col.close();

        std::ofstream ofs_val( "val.txt" );
        std::copy( val.begin(), val.end(), std::ostream_iterator<int>( ofs_val, "\n" ) );
        ofs_val.close();

        std::ofstream ofs_mtx( "N2_2N.mtx" );
        ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
        ofs_mtx.precision( 15 );
        ofs_mtx << N*N << " " << 2*N << " " << elements << "\n";
        for ( unsigned long idx = 0; idx != elements; ++idx )
            ofs_mtx << row[idx] << " " << col[idx] << " " << val[idx] << "\n";
        ofs_mtx.close();
    }

    if (0)
    {
        std::vector<int> row;
        std::vector<int> col;
        std::vector<double> val;

        std::array<int,4> row_rec;
        std::array<int,4> col_rec;
        std::array<int,4> val_rec;

        int const N = 2048;
        int const elements = N * N * 4;
        row.reserve( elements );
        col.reserve( elements );
        val.reserve( elements );

        auto const& impl = [&]( int r, int c )
        {
            int const A_row = r * N + c;

            row_rec = { A_row, A_row, A_row, A_row };
            col_rec = { r+r, r+r+1, N+N+c+c, N+N+c+c+1 };
            val_rec = { r+1, 1, -c-1, -1 };

            for ( unsigned long idx = 0; idx != 4; ++idx )
            {
                row.push_back( row_rec[idx]+1 );
                col.push_back( col_rec[idx]+1 );
                val.push_back( 1.0 * val_rec[idx] / 2048.0 );
            }
        };

        for ( int r = 0; r != N; ++r )
            for ( int c = 0; c != N; ++c )
                impl( r, c );



        std::ofstream ofs_row( "row.txt" );
        std::copy( row.begin(), row.end(), std::ostream_iterator<int>( ofs_row, "\n" ) );
        ofs_row.close();

        std::ofstream ofs_col( "col.txt" );
        std::copy( col.begin(), col.end(), std::ostream_iterator<int>( ofs_col, "\n" ) );
        ofs_col.close();

        std::ofstream ofs_val( "val.txt" );
        std::copy( val.begin(), val.end(), std::ostream_iterator<double>( ofs_val, "\n" ) );
        ofs_val.close();

        std::ofstream ofs_mtx( "N2_2N.mtx" );
        ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
        ofs_mtx.precision( 15 );
        ofs_mtx << N*N << " " << 4*N << " " << elements << "\n";
        for ( unsigned long idx = 0; idx != elements; ++idx )
            ofs_mtx << row[idx] << " " << col[idx] << " " << val[idx] << "\n";
        ofs_mtx.close();

    }

    return 0;
}

