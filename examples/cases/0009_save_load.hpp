void _0000_save_load()
{
    //auto const& m = feng::rand<double>( 128, 128 );
    feng::matrix<double> m;
    (void)m.load_txt( "./images/Lenna.txt" );
    (void)m.save_as_txt( "./images/0000_save_load.txt" );
    (void)m.save_as_binary( "./images/0000_save_load.bin" );
    (void)m.save_as_bmp( "./images/0000_save_load.bmp" );
    feng::matrix<double> n;
    (void)n.load_txt( "./images/0000_save_load.txt" );
    (void)n.save_as_bmp( "./images/0001_save_load.bmp" );
    (void)n.load_binary( "./images/0000_save_load.bin" );
    (void)n.save_as_pgm( "./images/0002_save_load.pgm" );
}

void _0001_save_load()
{
    {
        feng::matrix<double> m;
        (void)m.load_txt( "./images/Lenna.txt" );
        (void)m.save_as_bmp( "./images/Lenna.bmp", "gray" );
    }

    auto const& mat_3 = feng::load_bmp( "./images/Lenna.bmp" );
    if ( mat_3 )
    {
        (void)(*mat_3)[0].save_as_bmp( "./images/0001_save_load_julia_red.bmp", "gray" );
        (void)(*mat_3)[1].save_as_bmp( "./images/0001_save_load_julia_green.bmp", "gray" );
        (void)(*mat_3)[2].save_as_bmp( "./images/0001_save_load_julia_blue.bmp", "gray" );
    }
    else
    {
        std::cerr << "Failed to load Lenna." << std::endl;
    }
}

