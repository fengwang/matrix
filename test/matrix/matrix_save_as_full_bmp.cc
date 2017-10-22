#include <f/matrix/matrix.hpp>

int main()
{
    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./image.txt" );
        image.save_as_full_bmp( "image" );
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./matlab/display_experimental/rmap.txt" );
        image.save_as_full_bmp( "./matlab/display_experimental/rmap_full" );
    }

    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./matlab/display_experimental/imap.txt" );
        image.save_as_full_bmp( "./matlab/display_experimental/imap_full" );
    }

    if ( 1 )
    {
        auto image = f::rand<double>( 1000, 1000 );

        image.save_as_full_bmp( "./rand_image" );
    }

    return 0;
}
