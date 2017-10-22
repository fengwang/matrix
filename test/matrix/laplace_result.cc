#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/laplace.hpp>

int main()
{

    f::matrix<double> nt;
    //nt.load("./matrix/demo_result.txt");
    nt.load("./matrix/demo.laplace.1d.txt");
    //nt.load("./matrix/demo4_result_cmt_se.txt");

    std::cout << "Max element in the reconstructed matrix is " << *std::max_element( nt.begin(), nt.end() ) << std::endl;
    std::cout << "Min element in the reconstructed matrix is " << *std::min_element( nt.begin(), nt.end() ) << std::endl;
    std::cout << "Average of the reconstructed matrix is " << std::accumulate( nt.begin(), nt.end(), 0.0 )/nt.size() << std::endl;

    assert( nt.size() == 2048*2048 );

    nt.reshape( 2048, 2048 );
    nt.save_as_bmp( "demo4_inverse", "gray" );
    nt.save_as( "demo4_inverse.txt" );
    nt.save_as_binary( "demo4_inverse.bin" );


    auto const& mt = f::laplace( nt );
    mt.save_as_bmp( "demo4_reconstructed_laplace", "gray" );



    return 0;
}

