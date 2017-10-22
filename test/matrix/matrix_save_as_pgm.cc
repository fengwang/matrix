#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>
#include <f/algorithm/for_each.hpp>
#include <f/lexical_cast/lexical_cast.hpp>
#include <f/variate_generator/variate_generator.hpp>
#include <f/matrix/numeric/gradient_x2.hpp>

#include <string>

int main()
{

    auto const& process = [](std::string const& path )
    {
        std::string const folder = std::string{"./matrix/"};
        std::string const txt = std::string{".txt"};
        std::string const new_txt = std::string{"_new.txt"};
        std::string const xx_txt = std::string{"_xx.txt"};
        std::string const yy_txt = std::string{"_yy.txt"};

        f::matrix<double> mt;
        mt.load( folder+path+txt );
        double const mx = *std::max_element( mt.begin(), mt.end() );
        double const mn = *std::min_element( mt.begin(), mt.end() );
        mt -= mn;
        mt /= mx - mn;

        for ( unsigned long r = 0; r != mt.row() / 2; ++r )
            std::swap_ranges( mt.row_begin(r), mt.row_end(r), mt.row_begin( mt.row()-r-1 ) );

        mt.save_as( folder+path+new_txt );
        mt.save_as_pgm( folder+path );

        auto const& fxx = f::gradient_x2( mt );
        fxx.save_as( folder+path+xx_txt );

        auto const& fyy = f::gradient_x2( mt.transpose() ).transpose();
        fyy.save_as( folder+path+yy_txt );
    };

    process( "design" );
    process( "fractal" );
    process( "mass" );
    process( "snow" );
    process( "summer" );
    process( "tide" );

    if ( 1 )
    {
        f::matrix<double> mt;
        mt.load("./matrix/demo4.txt");
        mt.save_as_pgm( "./matrix/demo4_orig.pgm");
        mt.save_as_bmp( "./matrix/demo4_orig.bmp", "jet");
        for ( unsigned long r = 0; r != mt.row() / 2; ++r )
            std::swap_ranges( mt.row_begin(r), mt.row_end(r), mt.row_begin( mt.row()-r-1 ) );

        mt.save_as( "./matrix/_demo4.txt");
        mt.save_as_pgm( "./matrix/demo4.pgm");
        mt.save_as_bmp( "./matrix/demo4.bmp");
        //mt.save_as_bmp( "./matrix/demo4.bmp", "jet");

        auto const& fxx = f::gradient_x2( mt );
        fxx.save_as( "./matrix/demo4_xx.txt" );
        fxx.save_as_pgm( "./matrix/demo4_xx.pgm" );

        auto const& fyy = f::gradient_x2( mt.transpose() ).transpose();
        fyy.save_as( "./matrix/demo4_yy.txt" );
        fyy.save_as_pgm( "./matrix/demo4_fyy.pgm" );
    }

    if ( 1 )
    {
        f::matrix<double> mt;
        mt.load("./matrix/demo2.txt");
        mt.save_as_pgm( "./matrix/demo2_orig.pgm");
        mt.save_as_bmp( "./matrix/demo2_orig.bmp", "jet");
        for ( unsigned long r = 0; r != mt.row() / 2; ++r )
            std::swap_ranges( mt.row_begin(r), mt.row_end(r), mt.row_begin( mt.row()-r-1 ) );

        mt.save_as( "./matrix/_demo2.txt");
        mt.save_as_pgm( "./matrix/demo2.pgm");
        mt.save_as_bmp( "./matrix/demo2.bmp");
        //mt.save_as_bmp( "./matrix/demo2.bmp", "jet");

        auto const& fxx = f::gradient_x2( mt );
        fxx.save_as( "./matrix/demo2_xx.txt" );
        fxx.save_as_pgm( "./matrix/demo2_xx.pgm" );

        auto const& fyy = f::gradient_x2( mt.transpose() ).transpose();
        fyy.save_as( "./matrix/demo2_yy.txt" );
        fyy.save_as_pgm( "./matrix/demo2_fyy.pgm" );
    }

    if ( 1 )
    {
        f::matrix<double> mt;
        mt.load("./matrix/demo.txt");
        mt.save_as_pgm( "./matrix/demo_orig.pgm");
        mt.save_as_bmp( "./matrix/demo_orig.bmp", "jet");
        for ( unsigned long r = 0; r != mt.row() / 2; ++r )
            std::swap_ranges( mt.row_begin(r), mt.row_end(r), mt.row_begin( mt.row()-r-1 ) );

        mt.save_as( "./matrix/_demo.txt");
        mt.save_as_pgm( "./matrix/demo.pgm");
        mt.save_as_bmp( "./matrix/demo.bmp");
        //mt.save_as_bmp( "./matrix/demo.bmp", "jet");

        auto const& fxx = f::gradient_x2( mt );
        fxx.save_as( "./matrix/demo_xx.txt" );
        fxx.save_as_pgm( "./matrix/demo_xx.pgm" );

        auto const& fyy = f::gradient_x2( mt.transpose() ).transpose();
        fyy.save_as( "./matrix/demo_yy.txt" );
        fyy.save_as_pgm( "./matrix/demo_fyy.pgm" );
    }

    if ( 1 )
    {
        f::matrix<double> mt;
        mt.load("./matrix/phantom.txt");
        mt.save_as_pgm( "./matrix/phantom_orig.pgm");
        mt.save_as_bmp( "./matrix/phantom_orig.bmp", "jet");
        for ( unsigned long r = 0; r != mt.row() / 2; ++r )
            std::swap_ranges( mt.row_begin(r), mt.row_end(r), mt.row_begin( mt.row()-r-1 ) );

        mt.save_as( "./matrix/_phantom.txt");
        mt.save_as_pgm( "./matrix/phantom.pgm");
        mt.save_as_bmp( "./matrix/phantom.bmp");
        //mt.save_as_bmp( "./matrix/phantom.bmp", "jet");

        auto const& fxx = f::gradient_x2( mt );
        fxx.save_as( "./matrix/phantom_xx.txt" );
        fxx.save_as_pgm( "./matrix/phantom_xx.pgm" );

        auto const& fyy = f::gradient_x2( mt.transpose() ).transpose();
        fyy.save_as( "./matrix/phantom_yy.txt" );
        fyy.save_as_pgm( "./matrix/phantom_fyy.pgm" );
    }

    if ( 0 )
    {
        f::matrix<double> mt;
        mt.load("./matrix/demo2.txt");
        mt.save_as_pgm( "demo2_default.pgm");

    }

    if ( 1 )
    {
        auto const& func1 = []( double x, double y )
        {
            //double const alpha = 62.8318530717958647692;
            //double const alpha = 15.7079632679489661923;
            double const alpha = 31.4159265358979323846;
            double const beta = 2.0;
            //return x * x + y * y - beta * std::cos( alpha * x ) - beta * std::cos( alpha * y ) + x*y*(x+x*x+y+y*y+beta);
            return x * x + y * y - beta * std::cos( alpha * x ) - beta * std::cos( alpha * y );// - beta * std::cos(alpha * x * y);
        };

        auto const& func2 = []( double x, double y )
        {
            double const rr = x*x + y*y;
            double const r = std::sqrt(rr);
            return (1.0 + std::cos( 12.0 * r )) / (0.5 * rr + 2.0);
        };

        auto const& func3 = []( double x, double y )
        {
            double const pi = 3.14159265358979323846;
            double const fx = std::sin(pi*x);
            double const fx2 = fx*fx;
            double const fx6 = fx2*fx2*fx2;
            double const fy = std::sin(pi*y);
            double const fy2 = fy*fy;
            double const fy6 = fy2*fy2*fy2;
            return fx6 + fy6;
        };

        //auto const& func = func2;
        //auto const& func = [&]( double x, double y ) { return func1(x,y) * func2(x,y) * func3(x,y); };
        //auto const& func = func3;
        auto const& func = func1;

        unsigned long const dim = 2048;
        double const x_start = 0.0;
        double const x_end = 1.0;
        double const y_start = 0.0;
        double const y_end = 1.0;
        double const x_stride = (x_end-x_start)/(dim-1);
        double const y_stride = (y_end-y_start)/(dim-1);

        f::matrix<double> m{ dim, dim };
        for ( unsigned long r = 0; r != dim; ++r )
            for ( unsigned long c = 0; c != dim; ++c )
            {
                double const x = x_start + x_stride*r;
                double const y = y_start + y_stride*c;
                m[r][c] = func( x, y );
            }

        std::cout << "Max of generated matrix is " << *std::max_element( m.begin(), m.end() ) << std::endl;
        std::cout << "Min of generated matrix is " << *std::min_element( m.begin(), m.end() ) << std::endl;

        double const mx = *std::max_element( m.begin(), m.end() );
        double const mn = *std::min_element( m.begin(), m.end() );

        m /= mx - mn;
        m += mn;

        m.save_as( "./matrix/f1.txt" );
        m.save_as_pgm( "./matrix/f1.pgm" );
        m.save_as_bmp( "f1.bmp" );
        m.save_as_bmp( "f1_jet.bmp", "jet" );

        auto const& fxx = f::gradient_x2( m );
        fxx.save_as( "./matrix/fxx.txt" );
        fxx.save_as_pgm( "./matrix/fxx.pgm" );
        fxx.save_as_bmp( "fxx.bmp", "jet" );

        auto const& fyy = f::gradient_x2( m.transpose() ).transpose();
        fyy.save_as( "./matrix/fyy.txt" );
        fyy.save_as_pgm( "./matrix/fyy.pgm" );
        fyy.save_as_bmp( "fyy.bmp", "jet" );
    }

    return 0;
}

