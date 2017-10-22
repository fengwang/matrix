#include <f/matrix/matrix.hpp>
#include <algorithm>
#include <vector>

int main()
{
    f::matrix<double> mat;
    //mat.load( "r1.txt" );
    //mat.load( "r.txt" );
    //mat.load( "r1.txt" );
    mat.load( "rx1.txt" );

    std::cout << "\nthe loaded matrix is of size (" << mat.row() << "," << mat.col() << ")\n";

    //mat.load( "r2.txt" );
    
    //double const max_threshold = *std::min_element( mat.begin(), mat.end() ) * 10000000.0;
    double const max_threshold = *std::min_element( mat.begin(), mat.end() ) * 1000.0;
    double const min_threshold = *std::min_element( mat.begin(), mat.end() ) * 2.0;
    //double const max_threshold = std::min( *std::min_element( mat.begin(), mat.end() ) * 1000.0, 1.0e-4);
    //double const min_threshold = std::min( *std::min_element( mat.begin(), mat.end() ) * 2.0, 1.0e-4);
    //
    std::cout << "\nthreshold are " << max_threshold << " and " << min_threshold << "\n";

    std::vector<double> vec;
    std::copy_if( mat.begin(), mat.end(), std::back_inserter(vec), [max_threshold, min_threshold](double x){ return (x < max_threshold)&&(x > min_threshold); } );

    double max_e = *std::max_element( vec.begin(), vec.end() );
    double min_e = *std::min_element( vec.begin(), vec.end() );

    std::cout << "valid elements in the vector is " << vec.size() << "\n";
    std::cout << "max and min element in the filted vector is " << max_e << " and " << min_e << "\n";

    //unsigned long const bins = std::max( static_cast<unsigned long>(200), static_cast<unsigned long>(std::ceil( std::sqrt( vec.size() ) )) );
    unsigned long const bins = 200;
    double const step = ( max_e - min_e ) / static_cast<double>(bins);
    f::matrix<double> emat( bins, 2 );

    std::fill( emat.begin(), emat.end(), 0 );

    for ( unsigned long index = 0; index != bins; ++index )
        emat[index][0] = min_e + step*index;

    for ( unsigned long index = 0; index != vec.size(); ++index )
    {
        unsigned long const jndex = static_cast<unsigned long>(std::floor( (vec[index] - min_e) / step ));
        emat[jndex][1]++;
    }

    std::cout << "\n\nthe generated bin are\n" << emat << "\n";

    return 0;
}

