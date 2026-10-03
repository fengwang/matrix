void _0000_conv()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/Lenna.txt" );
    (void)m.save_as_bmp( "./images/0000_conv.bmp", "gray" );

    feng::matrix<double> filter{3, 3, {0.0, 1.0, 0.0,
                                       1.0,-4.0, 1.0,
                                       0.0, 1.0, 0.0}};
    auto const& edge = feng::conv( m, filter );
    (void)edge.save_as_bmp( "./images/0001_conv.bmp", "gray" );
    (void)edge.save_as_txt( "./images/0001_conv.txt" );
    (void)edge.save_as_pgm( "./images/0001_conv.pgm" );

    auto const& edge_valid = feng::conv( m, filter, "valid" );
    (void)edge_valid.save_as_bmp( "./images/0001_conv_valid.bmp", "gray" );

    auto const& edge_same = feng::conv( m, filter, "same" );
    (void)edge_same.save_as_bmp( "./images/0001_conv_same.bmp", "gray" );

    auto const& edge_full = feng::conv( m, filter, "full" );
    (void)edge_full.save_as_bmp( "./images/0001_conv_full.bmp", "gray" );

    // asymmetric kernel: conv reverses it (scipy.signal.convolve2d), so this is the horizontal Sobel derivative
    feng::matrix<double> sobel{3, 3, {-1.0, 0.0, 1.0,
                                      -2.0, 0.0, 2.0,
                                      -1.0, 0.0, 1.0}};
    auto const& edge_sobel = feng::conv( m, sobel, "same" );
    (void)edge_sobel.save_as_bmp( "./images/0001_conv_sobel.bmp", "gray" );
}

