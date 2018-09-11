void _0000_plus_equal()
{
    feng::matrix<double> image;
    image.load_txt( "images/Lenna.txt" );
    image.save_as_bmp("images/0000_plus_equal.bmp", "gray");

    double const mn = *std::min_element( image.begin(), image.end() );
    double const mx = *std::max_element( image.begin(), image.end() );
    image = (image - mn)/(mx - mn);

    auto const& noise = feng::rand<double>( image.row(), image.col(), 1 ); //setting random seed to 1
    image += 0.1*noise;
    image.save_as_bmp("images/0001_plus_equal.bmp", "gray");
}

