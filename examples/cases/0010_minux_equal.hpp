void _0000_minus_equal()
{
    feng::matrix<double> image;
    image.load_txt( "images/Lenna.txt" );
    image.save_as_bmp("images/0000_minus_equal.bmp");

    double const min = *std::min_element( image.begin(), image.end() );
    image -= min;

    image.save_as_bmp("images/0001_minus_equal.bmp");

    image -= image;

    image.save_as_bmp("images/0002_minus_equal.bmp");
}

