#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/laplace.hpp>

int main()
{

    f::matrix<double> nt;

    if (0)
    {
        nt.load("./matrix/demo4.txt");
        nt.save_as_bmp( "demo4.bmp" );
    }

    if (0)
    {
        auto const& Gx = f::gradient_x2(nt);
        auto const& Gy = f::gradient_x2(nt.transpose());
        auto cmt = Gx - Gy;
        cmt.save_as_bmp( "L_se_demo4_gray_default_cmt.bmp", "gray", "default" );
        cmt.reshape( cmt.size(), 1 );
        cmt.save_as( "./matrix/demo4.laplace.1d_cmt.txt" );
    }

    if (0)
    {
        auto mt = laplace( nt );
        mt.save_as( "./matrix/demo4.laplace.txt" );

        std::cout << "Max element of Laplace is " << *std::max_element( mt.begin(), mt.end() ) << "\n";
        std::cout << "Min element of Laplace is " << *std::min_element( mt.begin(), mt.end() ) << "\n";
        std::cout << "Average of Laplace is " << std::accumulate( mt.begin(), mt.end(), 0.0 ) / mt.size() << "\n";

        mt.save_as_bmp( "L_se_demo4_default_log1.bmp", "default", "log1" );
        mt.save_as_bmp( "L_se_demo4_gray_log.bmp", "gray", "log" );
        mt.save_as_bmp( "L_se_demo4_gray_log1.bmp", "gray", "log1" );
        mt.save_as_bmp( "L_se_demo4_hotblue_logpi.bmp", "hotblue", "logpi" );
        mt.save_as_bmp( "L_se_demo4_jet_log2.bmp", "jet", "log2" );
        mt.save_as_bmp( "L_se_demo4_obscure_log3.bmp", "obscure", "log3" );
        mt.save_as_bmp( "L_se_demo4_deault_log4.bmp", "default", "log4" );
        mt.save_as_bmp( "L_se_demo4_jet_log4.bmp", "jet", "log4" );
        mt.save_as_bmp( "L_se_demo4_gray_log4.bmp", "gray", "log4" );
        mt.save_as_bmp( "L_se_demo4_gray_default.bmp", "gray", "default" );


        auto const& df = nt - mt;
        df.save_as_bmp( "L_se_diff_gray_default.bmp", "gray", "default" );
        nt.save_as_bmp( "demo4.bmp", "gray", "default" );


        mt.reshape( mt.size(), 1 );
        mt.save_as( "./matrix/demo4.laplace.1d.txt" );

        nt.reshape( nt.size(), 1 );
        nt.save_as( "./matrix/demo4.1d.txt" );
    }


    if (0)
    {
        nt.load( "./matrix/demo.txt" );
        nt.save_as_pgm( "./matrix/demo.pgm" );
        auto const& Gx = f::gradient_x2(nt);
        Gx.save_as( "./matrix/demo_xx.txt" );
        Gx.save_as_pgm( "./matrix/demo_xx.pgm" );
        auto const& Gy = f::gradient_x2(nt.transpose()).transpose();
        Gy.save_as( "./matrix/demo_yy.txt" );
        Gy.save_as_pgm( "./matrix/demo_yy.pgm" );
    }

    if (1)
    {
        nt.load( "./matrix/stone.txt" );
        nt.save_as_pgm( "./matrix/stone.pgm" );
        auto const& Gx = f::gradient_x2(nt);
        Gx.save_as( "./matrix/stone_xx.txt" );
        Gx.save_as_pgm( "./matrix/stone_xx.pgm" );
        auto const& Gy = f::gradient_x2(nt.transpose()).transpose();
        Gy.save_as( "./matrix/stone_yy.txt" );
        Gy.save_as_pgm( "./matrix/stone_yy.pgm" );
    }

    return 0;
}

