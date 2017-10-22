#include <iostream>
#include <algorithm>
#include <fstream>
#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>

int main()
{
    using namespace f;

    if (1)
    {
        matrix<double> m;
        m.load("./matrix/demo.txt");
        m.save_as_pgm( "m256" );
    }

    if (0)
    {
        matrix<float> m;
        m.load("/Users/feng/workspace/cunn/Astigmatism/Isim_00_a.txt");
        std::cout << m.size() << std::endl;
    }

    if (0)
    {
        variate_generator<double> vg{ 0.0, 1.0 };

        matrix<double> m{ 1000, 1000 };

        std::generate( m.begin(), m.end(), vg );

        m.save_as_bmp( "./rand.bmp" );
    }

#if 0
    unsigned long N = 1;

    variate_generator<double> v;
    matrix<double> A(N,N);

    std::generate( A.begin(), A.end(), v );

    A.save_as( "a1.txt" );

    std::ofstream ofs( "a2.txt" );

    for ( unsigned long r = 0; r != A.row(); ++r )
    {
        for ( unsigned long c = 0; c != A.col(); ++c )
        {
            ofs << A[r][c];
            if ( c != A.col() -1 )
                ofs << "\t";
        }
        if ( r != A.row() -1 )
            ofs << "\n";
    }

    ofs.close();

    matrix<double> a1, a2;
    a1.load( "a1.txt" );
    a2.load( "a2.txt" );

    std::cout << "\na1-a2=\n" << a1 - a2 << "\n";

    matrix<double> const A1( "a1.txt");
    matrix<double> const A2( "a2.txt");

    std::cout << "\nA1-A2=\n" << A1 - A2 << "\n";
#endif

#if 0
    matrix<double> img;
    img.load( "./matrix/intensity/TlNbO_457_d120t50.txt" );

    unsigned long r = img.row();
    unsigned long c = img.col();
    unsigned long m = c >> 1;

    for ( unsigned long i = 0; i != r; ++i )
    {
        double total = std::accumulate( img.row_begin(i), img.row_end(i), 0.0 );

        std::fill( img.row_begin(i), img.row_end(i), 0.0 );

        //img[i][m - 2] = total;
        //img[i][m - 1] = total;
        img[i][m] = total;
        //img[i][m + 1] = total;
        //img[i][m + 2] = total;
    }

    img.save_as( "./matrix/intensity/TlNbO_457_d120t50_se.txt" );

#endif

    return 0;
}

