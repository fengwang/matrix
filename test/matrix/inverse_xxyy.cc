#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/laplace.hpp>

#include <map>
#include <sstream>
#include <string>
#include <iomanip>

int main()
{
    std::stringbuf buffer;
    std::stringbuf buffer_xx;
    std::stringbuf buffer_yy;
    std::stringbuf buffer_row;
    std::stringbuf buffer_col;
    std::stringbuf buffer_data;
    std::ostream os(&buffer);
    std::ostream os_xx(&buffer_xx);
    std::ostream os_yy(&buffer_yy);
    std::ostream os_row(&buffer_row);
    std::ostream os_col(&buffer_col);
    std::ostream os_data(&buffer_data);

    unsigned long buffer_counter = 0;

    std::ofstream ofs_demo4( "./matrix/demo4_1d_xxyy.txt" );
    std::ofstream ofs_demo4_xx( "./matrix/demo4_1d_xx.txt" );
    std::ofstream ofs_demo4_yy( "./matrix/demo4_1d_yy.txt" );
    std::ofstream ofs_demo4_row( "./matrix/demo4_1d_xxyy_row.txt" );
    std::ofstream ofs_demo4_col( "./matrix/demo4_1d_xxyy_col.txt" );
    std::ofstream ofs_demo4_data( "./matrix/demo4_1d_xxyy_data.txt" );
    ofs_demo4.precision( 15 );

    f::matrix<double> nt;
    nt.load("./matrix/demo4.txt");
    std::cout << "demo4 matrix loaded with size " << nt.row() << " by " << nt.col() << "\n";

    {
        auto xx = f::gradient_x2(nt);

        std::cout.precision( 15 );
        std::cout << "max of Dxx is " << *std::max_element( xx.begin(), xx.end() ) << std::endl;;
        std::cout << "min of Dxx is " << *std::min_element( xx.begin(), xx.end() ) << std::endl;;


        std::copy( xx.begin(), xx.end(), std::ostream_iterator<double>( ofs_demo4, "\n" ) );
        std::copy( xx.begin(), xx.end(), std::ostream_iterator<double>( ofs_demo4_xx, "\n" ) );

        auto const& dim = nt.row();
        std::map<std::uint64_t, double> coords;

        auto append_record = [&]( std::uint64_t const d_r, std::uint64_t const d_c, std::uint64_t const a_m, std::uint64_t const a_n, double const w )
        {
            if ( std::abs(w) < 0.001 ) return; //filter zeros

            std::uint64_t const row_index = d_c * dim + d_r;
            std::uint64_t const col_index = a_m * dim + a_n;
            std::uint64_t const key = (row_index << 32) | col_index;

            if ( coords.find( key ) != coords.end() )
            {
                coords[key] += w;
            }
            else
            {
                coords[key] = w;
            }
        };

        {
            // row, col, val
            std::vector<std::uint64_t> D_r;
            std::vector<std::uint64_t> D_c;
            std::vector<std::uint64_t> A_m;
            std::vector<std::uint64_t> A_n;
            std::vector<double> w;

            // sum xx[Dr[idx]][Dc[idx]] += w[idx] G[Am[idx]][An[idx]]
            // b = A x
            std::tie( D_r, D_c, A_m, A_n, w ) = f::make_gradient_x2_config<double>( nt.row(), nt.col() );

            std::cout << "L2 coordinates of dx2 loaded with size " << D_r.size() << "\n";

            std::cout << "Max of D_r is " << *std::max_element( D_r.begin(), D_r.end() ) << "\n";

            for ( auto idx = 0UL; idx != D_r.size(); ++idx )
                append_record( D_c[idx], D_r[idx], A_m[idx], A_n[idx], w[idx] );

            std::cout << "Mapped recorded of size " << coords.size() << "\n";
        }

        auto const& make_diff = [&]( unsigned long row_idx, unsigned long col_idx, double w )
        {
            *(xx.begin()+row_idx) -= (*(nt.begin()+col_idx)) * w;
        };

        auto itor = coords.begin();
        for ( auto idx = 0UL; idx != coords.size(); ++idx )
        {
            auto const& rec = *itor;
            unsigned long const row_idx = rec.first >> 32;
            unsigned long const col_idx = (rec.first << 32) >> 32;
            double const weight = rec.second;

            os << (rec.first >> 32)+1 << "\t" << ( (rec.first << 32) >> 32)+1  << "\t" << rec.second << "\n";
            os_row << (rec.first >> 32)+1 << "\n";
            os_col << ( (rec.first << 32) >> 32)+1 << "\n";
            os_data << rec.second << "\n";
            os_xx << (rec.first >> 32)+1 << "\t" << ( (rec.first << 32) >> 32)+1  << "\t" << rec.second << "\n";
            ++buffer_counter;

            make_diff( row_idx, col_idx, weight );

            ++itor;
        }

        std::cout.precision( 15 );
        std::cout << "There are " << coords.size() << " elements in the sparse coefficient matrix.\n";
        std::cout << "Max diff is " << *std::max_element( xx.begin(), xx.end() ) << std::endl;
        std::cout << "Min diff is " << *std::min_element( xx.begin(), xx.end() ) << std::endl;

    }

    {
        auto yy = f::gradient_x2(nt.transpose()).transpose();

        std::cout.precision( 15 );
        std::cout << "max of Dyy is " << *std::max_element( yy.begin(), yy.end() ) << std::endl;;
        std::cout << "min of Dyy is " << *std::min_element( yy.begin(), yy.end() ) << std::endl;;

        std::copy( yy.begin(), yy.end(), std::ostream_iterator<double>( ofs_demo4, "\n" ) );
        std::copy( yy.begin(), yy.end(), std::ostream_iterator<double>( ofs_demo4_yy, "\n" ) );

        auto const& dim = nt.row();
        std::map<std::uint64_t, double> coords;

        auto append_record = [&]( std::uint64_t const d_r, std::uint64_t const d_c, std::uint64_t const a_m, std::uint64_t const a_n, double const w )
        {
            if ( std::abs(w) < 0.001 ) return; //filter zeros

            std::uint64_t const row_index = d_c * dim + d_r;
            std::uint64_t const col_index = a_m * dim + a_n;
            std::uint64_t const key = (row_index << 32) | col_index;

            if ( coords.find( key ) != coords.end() )
            {
                coords[key] += w;
            }
            else
            {
                coords[key] = w;
            }
        };

        {
            // row, col, val
            std::vector<std::uint64_t> D_r;
            std::vector<std::uint64_t> D_c;
            std::vector<std::uint64_t> A_m;
            std::vector<std::uint64_t> A_n;
            std::vector<double> w;

            // sum yy[Dr[idx]][Dc[idx]] += w[idx] G[Am[idx]][An[idx]]
            // b = A x
            std::tie( D_r, D_c, A_m, A_n, w ) = f::make_gradient_x2_config<double>( nt.row(), nt.col() );

            std::cout << "L2 coordinates of dx2 loaded with size " << D_r.size() << "\n";

            std::cout << "Max of D_r is " << *std::max_element( D_r.begin(), D_r.end() ) << "\n";

            for ( auto idx = 0UL; idx != D_r.size(); ++idx )
                //append_record( D_c[idx], D_r[idx], A_m[idx], A_n[idx], w[idx] );
                //!append_record( D_c[idx], D_r[idx], A_n[idx], A_m[idx], w[idx] );
                //!!append_record( D_r[idx], D_c[idx], A_m[idx], A_n[idx], w[idx] );
                append_record( D_r[idx], D_c[idx], A_n[idx], A_m[idx], w[idx] );

            std::cout << "Mapped recorded of size " << coords.size() << "\n";
        }

        auto const& make_diff = [&]( unsigned long row_idx, unsigned long col_idx, double w )
        {
            *(yy.begin()+row_idx) -= (*(nt.begin()+col_idx)) * w;
        };

        auto itor = coords.begin();
        for ( auto idx = 0UL; idx != coords.size(); ++idx )
        {
            auto const& rec = *itor;
            unsigned long const row_idx = rec.first >> 32;
            unsigned long const col_idx = (rec.first << 32) >> 32;
            double const weight = rec.second;

            os << yy.size() + (rec.first >> 32)+1 << "\t" << ( (rec.first << 32) >> 32)+1  << "\t" << rec.second << "\n";
            os_row << yy.size() + (rec.first >> 32)+1 << "\n";
            os_col << ( (rec.first << 32) >> 32)+1 << "\n";
            os_data << rec.second << "\n";
            os_yy << (rec.first >> 32)+1 << "\t" << ( (rec.first << 32) >> 32)+1  << "\t" << rec.second << "\n";
            ++buffer_counter;

            make_diff( row_idx, col_idx, weight );

            ++itor;
        }

        std::cout.precision( 15 );
        std::cout << "There are " << coords.size() << " elements in the sparse coefficient matrix.\n";
        std::cout << "Max diff is " << *std::max_element( yy.begin(), yy.end() ) << std::endl;
        std::cout << "Min diff is " << *std::min_element( yy.begin(), yy.end() ) << std::endl;

    }

    std::cout << "Generating sparse matrix _xxyy with size " << 2*nt.size() << " X " << nt.size() << " including " << buffer_counter << " Elements\n";

    {
        std::ofstream ofs_mtx{ "./matrix/demo4_coordinates_xxyy.mtx" };
        ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
        ofs_mtx << 2*nt.size() << "\t" << nt.size() << "\t" << buffer_counter << "\n";
        ofs_mtx.precision( 10 );
        ofs_mtx << buffer.str();
        ofs_mtx.close();
    }

    {
        std::ofstream ofs_mtx{ "./matrix/demo4_coordinates_xx.mtx" };
        ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
        ofs_mtx << nt.size() << "\t" << nt.size() << "\t" << buffer_counter/2 << "\n";
        ofs_mtx.precision( 10 );
        ofs_mtx << buffer_xx.str();
        ofs_mtx.close();
    }

    {
        std::ofstream ofs_mtx{ "./matrix/demo4_coordinates_yy.mtx" };
        ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
        ofs_mtx << nt.size() << "\t" << nt.size() << "\t" << buffer_counter/2 << "\n";
        ofs_mtx.precision( 10 );
        ofs_mtx << buffer_yy.str();
        ofs_mtx.close();
    }

    ofs_demo4_row << buffer_row.str();
    ofs_demo4_row.close();

    ofs_demo4_col << buffer_col.str();
    ofs_demo4_col.close();

    ofs_demo4_data << buffer_data.str();
    ofs_demo4_data.close();

    ofs_demo4.close();
    ofs_demo4_xx.close();
    ofs_demo4_yy.close();

    return 0;
}

