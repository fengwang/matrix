#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/math/standard/matrix_abs_expression.hpp>

#include <vector>
#include <iostream>
#include <algorithm>

int main()
{
    if (0)
    {
        f::matrix<double> A( 4, 5 );

        std::fill( A.begin(), A.end(), -1.0 );
        std::fill( A.anti_diag_begin(), A.anti_diag_end(), 3.4 );

        std::cout << "A=\n";
        std::cout << A << "\n";
        std::cout << "abs(A)=\n";
        std::cout << f::abs(A) << "\n";

        std::cout << "A+abs(A)=\n" << A+f::abs(A) << "\n";
    }
    {
        std::vector<std::string> vs = { "5", "10", "20", "40" };
        for ( unsigned long d = 0; d != 4; ++d )
            for ( unsigned long t = 0; t != 4; ++t )
        {
            std::string const& d1_path = std::string{"./matrix/intensity/SrTiO3_demo_d"} + vs[d] + std::string{"t"} + vs[t] + std::string{".txt"};
            std::string const& d2_path = std::string{"./matrix/intensity/SrTiO3_demo_d"} + vs[d] + std::string{"t"} + vs[t] + std::string{"_se.txt"};

            auto const&& normalize = []( f::matrix<double>& m )
            {
                std::for_each( m.begin(), m.end(), [](double& x){ if (x<0.0) x = 0.0; } );
                for ( unsigned long c = 0; c != m.col(); ++c )
                {
                    double const total = std::accumulate( m.col_begin(c), m.col_end(c), 0.0 );
                    std::for_each( m.col_begin(c), m.col_end(c), [total](double& x){ x/=total; } );
                }
                std::for_each( m.begin(), m.end(), [](double& x){ x = std::sqrt(x); } );
            };

            f::matrix<double> d1;
            //d1.load("./matrix/intensity/SrTiO3_demo_d40t20.txt");
            d1.load( d1_path );
            normalize(d1);

            f::matrix<double> d2;
            //d2.load("./matrix/intensity/SrTiO3_demo_d40t20_f.txt");
            d2.load( d2_path );
            normalize(d2);

            auto df = f::abs(d1-d2);
            double rm = std::accumulate( df.begin(), df.end(), 0.0 ) / std::accumulate( d1.begin(), d1.end(), 0.0 );
            std::cout << "R factor for d" << vs[d] << "t" << vs[t] << " is "  << rm << "\n";
        }
    }
    if (0)
    {
            std::string const& d1_path = std::string{"./matrix/intensity/SrTiO3_demo_d40t10.txt"};
            std::string const& d2_path = std::string{"./matrix/intensity/SrTiO3_demo_d40t10_se.txt"};

            auto const&& normalize = []( f::matrix<double>& m )
            {
                std::for_each( m.begin(), m.end(), [](double& x){ if (x<0.0) x = 0.0; } );
                for ( unsigned long c = 0; c != m.col(); ++c )
                {
                    double const total = std::accumulate( m.col_begin(c), m.col_end(c), 0.0 );
                    std::for_each( m.col_begin(c), m.col_end(c), [total](double& x){ x/=total; } );
                }
                std::for_each( m.begin(), m.end(), [](double& x){ x = std::sqrt(x); } );
            };

            f::matrix<double> d1;
            //d1.load("./matrix/intensity/SrTiO3_demo_d40t20.txt");
            d1.load( d1_path );
            normalize(d1);

            f::matrix<double> d2;
            //d2.load("./matrix/intensity/SrTiO3_demo_d40t20_f.txt");
            d2.load( d2_path );
            normalize(d2);

            auto df = f::abs(d1-d2);
            double rm = std::accumulate( df.begin(), df.end(), 0.0 ) / std::accumulate( d1.begin(), d1.end(), 0.0 );
            std::cout << "R factor is "  << rm << "\n";
    }

    return 0;
}

