void _0000_minus_equal()
{
    feng::matrix<double> image;
    image.load_txt( "images/Lenna.txt" );
    image.save_as_bmp("images/0000_minus_equal.bmp", "gray");

    double const min = *std::min_element( image.begin(), image.end() );
    image -= min;

    image.save_as_bmp("images/0001_minus_equal.bmp", "jet");

    image -= image;

    image.save_as_bmp("images/0002_minus_equal.bmp");
}

