void _0000_minus_equal()
{
    feng::matrix<double> image;
    (void)image.load_txt( "images/Lenna.txt" );
    (void)image.save_as_bmp("images/0000_minus_equal.bmp", "gray");

    double const min = *std::min_element( image.begin(), image.end() );
    image -= min;

    (void)image.save_as_bmp("images/0001_minus_equal.bmp", "jet");

    image -= image;

    (void)image.save_as_bmp("images/0002_minus_equal.bmp");
}

