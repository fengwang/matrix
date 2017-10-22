#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>
#include <f/algorithm/for_each.hpp>
#include <f/lexical_cast/lexical_cast.hpp>
#include <f/variate_generator/variate_generator.hpp>

#include <string>

int main()
{
    if ( 0 )
    {
        int thickness[] = { 5, 10, 20, 40 };
        unsigned long total = 4;

        f::matrix<double> image;
        for ( unsigned long k = 0; k != total; ++k )
        {
            std::string tk = f::lexical_cast<std::string>( thickness[k] );
            {
                std::string file_path = std::string{ "./matrix/ug/hybrid_01_homotopy/hybrid_d40t" } + tk + std::string{"_ug.txt"};
                std::string imag_path = std::string{ "./matrix/ug/hybrid_01_homotopy/hybrid_d40t" } + tk + std::string{"_ug"};
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
        }
    }

    if ( 0 )
    {
        f::matrix<double> p11; 
        f::matrix<double> p12; 
        f::matrix<double> pd; 

        p11.load( "./SrTiO3_demo_d70t11.txt" );
        p12.load( "./SrTiO3_demo_d70t12.txt" );

        pd = f::abs( p11 - p12 );

        pd[0][0] = *std::max_element( p11.begin(), p11.end() );

        pd.save_as_bmp( "SrTiO3_demo_d70_diff.bmp" );

    }


    if ( 0 )
    {
        int beams[] = { 512, 256, 128, 64, 32, 16 };
        unsigned long total = 6;

        f::matrix<double> image;
        for ( unsigned long k = 0; k != total; ++k )
        {
            std::string tk = f::lexical_cast<std::string>( beams[k] );
            {
                std::string file_path = std::string{ "./matrix/ug/limited_beam/srtio3_" } + tk + std::string{".txt"};
                std::string imag_path = std::string{ "./matrix/ug/limited_beam/srtio3_" } + tk;
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
            {
                std::string file_path = std::string{ "./matrix/ug/limited_beam/poisson_srtio3_" } + tk + std::string{".txt"};
                std::string imag_path = std::string{ "./matrix/ug/limited_beam/poisson_srtio3_" } + tk;
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
        }
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./SrTiO3_abs_d40t40");

        double mx = *std::max_element( image.begin(), image.end() );

        auto old_image = image;
        f::variate_generator<double, f::poisson> vg( 1.0 );
        for ( auto& x : image )
        {
            x *= 1.0e3;
            if ( x > 1.0 );
            x = vg( x ) * 1.0e-3;
        }
        image.save_as_bmp( "./SrTiO3_abs_d40t40_se");
         
        old_image -= image;
        old_image = f::abs(old_image);
        old_image[0][0] = mx;

        old_image.save_as_bmp( "./SrTiO3_abs_d40t40_diff");
    }

    if ( 0 )
    {
        ///Users/feng/workspace/f/matrix/ug/si_se_abs/
        int tilt[4] = { 40, 20, 10, 5 };
        int thick[5] = { 30, 20, 10, 5 };

        f::matrix<double> image;
        for ( unsigned long t = 0 ; t != 4; ++t )
        {
            std::string tt = f::lexical_cast<std::string>( tilt[t] );
            for ( unsigned long k = 0; k != 4; ++k )
            {
                std::string tk = f::lexical_cast<std::string>( thick[k] );
                std::string file_path = std::string{ "./matrix/ug/si_abs/d" } + tt + std::string{"t"} + tk + std::string{".txt"};
                std::string imag_path = std::string{ "./matrix/ug/si_abs/d" } + tt + std::string{"t"} + tk;
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
        }
    }

    if ( 0 )
    {
        ///Users/feng/workspace/f/matrix/ug/si_se_abs/
        int tilt[4] = { 40, 20, 10, 5 };
        int thick[5] = { 30, 20, 10, 5 };

        f::matrix<double> image;
        for ( unsigned long t = 0 ; t != 4; ++t )
        {
            std::string tt = f::lexical_cast<std::string>( tilt[t] );
            for ( unsigned long k = 0; k != 4; ++k )
            {
                std::string tk = f::lexical_cast<std::string>( thick[k] );
                std::string file_path = std::string{ "./matrix/ug/si_se_chi_2/d" } + tt + std::string{"t"} + tk + std::string{".txt"};
                std::string imag_path = std::string{ "./matrix/ug/si_se_chi_2/d" } + tt + std::string{"t"} + tk;
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
        }
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./matrix/ug/si_potential_map_matrix.txt");
        image.save_as_bmp( "./matrix/ug/si_potential_map_matrix");
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./matrix/ug/si_se_potential_map_matrix.txt");
        image.save_as_bmp( "./matrix/ug/si_se_potential_map_matrix");
    }

    if ( 0 )
    {
        ///Users/feng/workspace/f/matrix/ug/si_se_abs/
        int tilt[4] = { 40, 20, 10, 5 };
        int thick[5] = { 40, 30, 20, 10, 5 };

        f::matrix<double> image;
        for ( unsigned long t = 0 ; t != 4; ++t )
        {
            std::string tt = f::lexical_cast<std::string>( tilt[t] );
            for ( unsigned long k = 0; k != 5; ++k )
            {
                std::string tk = f::lexical_cast<std::string>( thick[k] );
                std::string file_path = std::string{ "./matrix/ug/si_se_abs/d" } + tt + std::string{"t"} + tk + std::string{".txt"};
                std::string imag_path = std::string{ "./matrix/ug/si_se_abs/d" } + tt + std::string{"t"} + tk;
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
        }
    }

    if ( 0 )
    {
        ///Users/feng/workspace/f/matrix/ug/potential_field/SrTiO3_abs_ug_d20t10.txt
        int tilt[4] = { 40, 20, 10, 5 };
        int thick[5] = { 40, 30, 20, 10, 5 };

        f::matrix<double> image;
        for ( unsigned long t = 0 ; t != 4; ++t )
        {
            std::string tt = f::lexical_cast<std::string>( tilt[t] );
            for ( unsigned long k = 0; k != 5; ++k )
            {
                std::string tk = f::lexical_cast<std::string>( thick[k] );
                std::string file_path = std::string{ "./matrix/ug/potential_field/SrTiO3_abs_ug_d" } + tt + std::string{"t"} + tk + std::string{".txt"};
                std::string imag_path = std::string{ "./matrix/ug/potential_field/SrTiO3_abs_ug_d" } + tt + std::string{"t"} + tk;
                image.load( file_path );
                image.save_as_bmp( imag_path );
            }
        }
    }


    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./real.txt" );
        image.save_as_bmp( "real" );
    }
    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./image.txt" );
        image.save_as_bmp( "image" );
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./matlab/display_experimental/rmap.txt" );
        image.save_as_bmp( "./matlab/display_experimental/rmap" );
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./matlab/display_experimental/imap.txt" );
        image.save_as_bmp( "./matlab/display_experimental/imap" );
    }

    if ( 0 )
    {
		f::variate_generator<double> vg(0.0, 1.0);
        f::matrix<double> image{1000, 1000};

		std::generate( image.begin(), image.end(), vg );

        image.save_as_bmp( "./rand_image" );
    }

    if ( 0 )
    {
        f::matrix<double> a;
        f::matrix<double> b;
        a.load( "./a.txt" );
        b.load( "./b.txt" );

        auto processor = []( auto& mat )
        {
            auto mx = *std::max_element( mat.begin(), mat.end() );
            auto mn = *std::min_element( mat.begin(), mat.end() );
            mat -= mn;
            mat /= ( mx - mn );
        };
        processor( a );
        processor( b );

        f::for_each( a.begin(), a.end(), b.begin(), [](double& x, double y){ x *= y; } );

        a.save_as( "average.txt" );
        a.save_as_bmp( "average" );
    }

    if ( 1 )
    {
        f::matrix<double> mt;
        mt.load("Si_se_d40t40.txt");
        std::for_each( mt.begin(), mt.end(), [](double& x){ if (x > 1.0e-20) x+= 0.1;} );
        mt.save_as_uniform_bmp( "Si_se_d40t40_revised_uniform.bmp" );
    }

    return 0;
}
