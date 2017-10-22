#include <f/matrix/matrix.hpp>
#include <f/algorithm/for_each.hpp>

#include <map>
#include <sstream>
#include <string>
#include <iomanip>

int main()
{
    using namespace f;

    if (0)
    {
        matrix<double> xx;
        //xx.load("./matrix/demo_xx.txt");
        xx.load("./matrix/summer_xx.txt");

        matrix<double> yy;
        //yy.load("./matrix/demo_yy.txt");
        yy.load("./matrix/summer_yy.txt");

        matrix<double> iv;
        iv.load("./matrix/laplace_1d_inverse.txt");
        //iv.load("./matrix/laplace_1d_pinverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * (xx.transpose());
        auto const& xxiv = xxiv_t.transpose();
        auto const& yyiv = iv * yy;

        xxiv.save_as("./matrix/summer_xxiv.txt");
        xxiv.save_as_pgm("./matrix/summer_xxiv.pgm");
        yyiv.save_as("./matrix/summer_yyiv.txt");
        yyiv.save_as_pgm("./matrix/summer_yyiv.pgm");

    }

    if (1)
    {
        matrix<double> xx;
        //xx.load("./matrix/demo_xx.txt");
        xx.load("./matrix/tide_xx.txt");

        matrix<double> yy;
        //yy.load("./matrix/demo_yy.txt");
        yy.load("./matrix/tide_yy.txt");

        matrix<double> iv;
        iv.load("./matrix/laplace_1d_inverse.txt");
        //iv.load("./matrix/laplace_1d_pinverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * (xx.transpose());
        auto const& xxiv = xxiv_t.transpose();
        auto const& yyiv = iv * yy;

        xxiv.save_as("./matrix/tide_xxiv.txt");
        xxiv.save_as_pgm("./matrix/tide_xxiv.pgm");
        yyiv.save_as("./matrix/tide_yyiv.txt");
        yyiv.save_as_pgm("./matrix/tide_yyiv.pgm");

    }

    if (0)
    {
        matrix<double> xx;
        //xx.load("./matrix/demo_xx.txt");
        xx.load("./matrix/snow_xx.txt");

        matrix<double> yy;
        //yy.load("./matrix/demo_yy.txt");
        yy.load("./matrix/snow_yy.txt");

        matrix<double> iv;
        iv.load("./matrix/laplace_1d_inverse.txt");
        //iv.load("./matrix/laplace_1d_pinverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * (xx.transpose());
        auto const& xxiv = xxiv_t.transpose();
        auto const& yyiv = iv * yy;

        xxiv.save_as("./matrix/snow_xxiv.txt");
        xxiv.save_as_pgm("./matrix/snow_xxiv.pgm");
        yyiv.save_as("./matrix/snow_yyiv.txt");
        yyiv.save_as_pgm("./matrix/snow_yyiv.pgm");

    }

    if (0)
    {
        matrix<double> xx;
        //xx.load("./matrix/demo_xx.txt");
        xx.load("./matrix/phantom_xx.txt");

        matrix<double> yy;
        //yy.load("./matrix/demo_yy.txt");
        yy.load("./matrix/phantom_yy.txt");

        matrix<double> iv;
        iv.load("./matrix/laplace_1d_inverse.txt");
        //iv.load("./matrix/laplace_1d_pinverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * xx.transpose();
        auto const& xxiv = xxiv_t.transpose();
        auto const& yyiv = iv * yy;

        xxiv.save_as("./matrix/phantom_xxiv.txt");
        xxiv.save_as_pgm("./matrix/phantom_xxiv.pgm");
        yyiv.save_as("./matrix/phantom_yyiv.txt");
        yyiv.save_as_pgm("./matrix/phantom_yyiv.pgm");

    }

    if (0)
    {
        matrix<double> xx;
        //xx.load("./matrix/demo_xx.txt");
        xx.load("./matrix/stone_xx.txt");

        matrix<double> yy;
        //yy.load("./matrix/demo_yy.txt");
        yy.load("./matrix/stone_yy.txt");

        matrix<double> iv;
        iv.load("./matrix/laplace_1d_inverse.txt");
        //iv.load("./matrix/laplace_1d_pinverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * xx.transpose();
        auto const& xxiv = xxiv_t.transpose();
        auto const& yyiv = iv * yy;

        xxiv.save_as("stone_xxiv.txt");
        xxiv.save_as_pgm("stone_xxiv.pgm");
        yyiv.save_as("stone_yyiv.txt");
        yyiv.save_as_pgm("stone_yyiv.pgm");

    }

    if (1)
    {
        matrix<double> xx;
        //xx.load("./matrix/demo_xx.txt");
        xx.load("./matrix/fxx.txt");

        matrix<double> yy;
        //yy.load("./matrix/demo_yy.txt");
        yy.load("./matrix/fyy.txt");

        matrix<double> iv;
        iv.load("./matrix/laplace_1d_inverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * xx.transpose();
        auto const& xxiv = xxiv_t.transpose();
        auto const& yyiv = iv * yy;

        xxiv.save_as("./matrix/f1_xxiv.txt");
        xxiv.save_as_pgm("./matrix/f1_xxiv.pgm");
        yyiv.save_as("./matrix/f1_yyiv.txt");
        yyiv.save_as_pgm("./matrix/f1_yyiv.pgm");

    }

    return 0;
}
