#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/laplace.hpp>

#include <map>

int main()
{
    constexpr int addtional_term = 0;
    constexpr double lambda = 1.0e-1;


    f::matrix<double> nt;
    nt.load("./matrix/demo.txt");
    f::matrix<double> df = f::laplace(nt);
    f::matrix<double> nl{ nt.row(), nt.col() };
    std::fill( nl.begin(), nl.end(), 0.0 );

    auto const& make_diff = [&]( unsigned long row_idx, unsigned long col_idx, double w ) { *(df.begin()+row_idx) -= (*(nt.begin()+col_idx)) * w; (*(nl.begin()+row_idx)) += (*(nt.begin()+col_idx))*w; };

    std::uint64_t const dim = nt.row();

    std::cout << "demo matrix loaded with size " << nt.row() << " by " << nt.col() << "\n";

    std::map<std::uint64_t, double> coords;

    auto append_record = [&]( std::uint64_t const d_r, std::uint64_t const d_c, std::uint64_t const a_m, std::uint64_t const a_n, double const w )
    {
        //std::uint64_t const row_index = d_r * dim + d_c;
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

        std::tie( D_r, D_c, A_m, A_n, w ) = f::make_gradient_x2_config<double>( nt.row(), nt.col() );

        std::cout << "L2 coordinates of dx2 loaded with size " << D_r.size() << "\n";

        /*
        for ( unsigned long r = 0; r != 10; ++r )
        {
            std::cout << r <<  ": " << D_r[r] << " " << D_c[r] << " " << A_m[r] << " " << A_n[r] << " - " << w[r] << "\n";
        }
        */

        std::cout << "Max of D_r is " << *std::max_element( D_r.begin(), D_r.end() ) << "\n";

        for ( auto idx = 0UL; idx != D_r.size(); ++idx )
            append_record( D_c[idx], D_r[idx], A_m[idx], A_n[idx], w[idx] );

        std::cout << "Mapped recorded of size " << coords.size() << "\n";

        if ( nt.row() == nt.col() )
        {
            for ( auto idx = 0UL; idx != D_r.size(); ++idx )
                append_record( D_r[idx], D_c[idx], A_n[idx], A_m[idx], w[idx] );
        }
        else
        {
            std::tie( D_c, D_r, A_m, A_n, w ) = f::make_gradient_x2_config<double>( nt.col(), nt.row() );

            for ( auto idx = 0UL; idx != D_r.size(); ++idx )
                append_record( D_r[idx], D_c[idx], A_n[idx], A_m[idx], w[idx] );
        }
        std::cout << "Mapped recorded of size " << coords.size() << "\n";
    }

    std::ofstream ofs_rws{ "./matrix/demo_inverse_laplace_coords.txt" };
    std::ofstream ofs_ws{ "./matrix/demo_inverse_laplace_weights.txt" };

    std::ofstream ofs_mtx{ "./matrix/demo_coordinates.mtx" };

    ofs_mtx << "%%MatrixMarket matrix coordinate real general\n";
    ofs_mtx << nt.size() << "\t" << nt.size() << "\t" << coords.size() << "\n";
    ofs_mtx.precision( 10 );

    auto itor = coords.begin();
    for ( auto idx = 0UL; idx != coords.size(); ++idx )
    {
        auto const& rec = *itor;
        unsigned long const row_idx = rec.first >> 32;
        unsigned long const col_idx = (rec.first << 32) >> 32;
        double const weight = rec.second;

        ofs_rws << (rec.first >> 32) << " " << ( (rec.first << 32) >> 32)  << "\n";
        ofs_ws << rec.second << "\n";

        ofs_mtx << (rec.first >> 32)+1 << "\t" << ( (rec.first << 32) >> 32)+1  << "\t" << rec.second << "\n";

        make_diff( row_idx, col_idx, weight );

        ++itor;
    }

    if ( addtional_term )
    {
        for ( auto idx = 0U; idx != nt.size(); ++idx )
        {
            //ofs_mtx << idx + 1 + coords.size() << "\t" << idx+1 << "\t" << lambda << "\n";
            ofs_mtx << idx + 1 + nt.size() << "\t" << idx+1 << "\t" << lambda << "\n";
        }
    }

    ofs_rws.close();
    ofs_ws.close();
    ofs_mtx.close();


    std::cout << "Max diff is " << *std::max_element( df.begin(), df.end() ) << std::endl;
    std::cout << "Min diff is " << *std::min_element( df.begin(), df.end() ) << std::endl;

    df.save_as_bmp( "./diff.bmp", "gray" );


    nl.save_as_bmp( "./new_lap_gray_log1.bmp", "gray", "log1" );

    std::ofstream ofs_demo( "./matrix/demo_1d.txt" );
    ofs_demo.precision( 15 );
    std::copy( nt.begin(), nt.end(), std::ostream_iterator<double>( ofs_demo, "\n" ) );

    if ( addtional_term )
    {
        for ( auto idx = 0U; idx != nt.size(); ++idx )
            ofs_demo << 0.0 << "\n";
    }

    ofs_demo.close();

    /*
    f::matrix<std::uint64_t> rws{coords.size(), 2};
    f::matrix<double> ws{coords.size(), 1};

    auto itor = coords.begin();

    for ( auto r = 0U; r != rws.row(); ++r )
    {
        auto const& rec = *itor;
        rws[r][0] = rec.first >> 32;
        rws[r][1] = (rec.first << 32) >> 32;
        ws[r][0] = rec.second;
        ++itor;
    }

    rws.save_as( "./matrix/demo_inverse_laplace_coords.txt" );
    ws.save_as( "./matrix/demo_inverse_laplace_weights.txt" );
    */

    return 0;
}

