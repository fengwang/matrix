void _0000_conv()
{
    feng::matrix<double> m;
    m.load_txt( "./images/Lenna.txt" );
    m.save_as_bmp( "./images/0000_conv.bmp", "gray" );

    feng::matrix<double> filter{3, 3, {0.0, 1.0, 0.0,
                                       1.0,-4.0, 1.0,
                                       0.0, 1.0, 0.0}};
    auto const& edge = feng::conv( m, filter );
    edge.save_as_bmp( "./images/0001_conv.bmp", "gray" );
    edge.save_as_txt( "./images/0001_conv.txt" );
}

