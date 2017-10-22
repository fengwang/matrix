#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>

int main()
{
    if ( 0 )
    {
        f::matrix<double> image;
        image.load( "./image.txt" );
        image.save_as_full_bmp( "image" );
    }

    if ( 1 )
    {
        f::matrix<double> image;
        image.load( "./matlab/display_experimental/rmap.txt" );
        image.save_as_full_bmp( "./matlab/display_experimental/rmap_full" );
    }

    if ( 1 )
    {
        f::matrix<double> image;
        image.load( "./matlab/display_experimental/imap.txt" );
        image.save_as_full_bmp( "./matlab/display_experimental/imap_full" );
    }

    if ( 1 )
    {
		f::variate_generator<double> vg(0.0, 1.0);
        f::matrix<double> image{1000, 1000};

		std::generate( image.begin(), image.end(), vg );

        image.save_as_full_bmp( "./rand_image" );
    }

    return 0;
}
