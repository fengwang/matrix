void _0000_pooling()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/Lenna.txt" );
    (void)m.save_as_bmp( "./images/0000_pooling.bmp", "gray" );

    auto const& pooling_2 = feng::pooling( m, 2 );
    (void)pooling_2.save_as_bmp( "./images/0000_pooling_2.bmp", "gray" );
    auto const& pooling_4 = feng::pooling( m, 4 );
    (void)pooling_4.save_as_bmp( "./images/0000_pooling_4.bmp", "gray" );
    auto const& pooling_2_4 = feng::pooling( m, 2, 4 );
    (void)pooling_2_4.save_as_bmp( "./images/0000_pooling_2_4.bmp", "gray" );
    auto const& pooling_4_2 = feng::pooling( m, 4, 2 );
    (void)pooling_4_2.save_as_bmp( "./images/0000_pooling_4_2.bmp", "gray" );

    auto const& pooling_min = feng::pooling( m, 2, "min" );
    (void)pooling_min.save_as_bmp( "./images/0000_pooling_2_min.bmp", "gray" );

    auto const& pooling_max = feng::pooling( m, 2, "max" );
    (void)pooling_max.save_as_bmp( "./images/0000_pooling_2_max.bmp", "gray" );
}

