void _0000_save_load()
{
    //auto const& m = feng::rand<double>( 128, 128 );
    feng::matrix<double> m;
    m.load_txt( "./images/Lenna.txt" );
    m.save_as_txt( "./images/0000_save_load.txt" );
    m.save_as_binary( "./images/0000_save_load.bin" );
    m.save_as_bmp( "./images/0000_save_load.bmp" );
    feng::matrix<double> n;
    n.load_txt( "./images/0000_save_load.txt" );
    n.save_as_bmp( "./images/0001_save_load.bmp" );
    n.load_binary( "./images/0000_save_load.bin" );
    n.save_as_pgm( "./images/0002_save_load.pgm" );
}

void _0001_save_load()
{
    {
        feng::matrix<double> m;
        m.load_txt( "./images/Lenna.txt" );
        m.save_as_bmp( "./images/Lenna.bmp", "gray" );
    }

    auto const& mat_3 = feng::load_bmp( "./images/Lenna.bmp" );
    if ( mat_3 )
    {
        (*mat_3)[0].save_as_bmp( "./images/0001_save_load_julia_red.bmp", "gray" );
        (*mat_3)[1].save_as_bmp( "./images/0001_save_load_julia_green.bmp", "gray" );
        (*mat_3)[2].save_as_bmp( "./images/0001_save_load_julia_blue.bmp", "gray" );
    }
    else
    {
        std::cerr << "Failed to load Lenna." << std::endl;
    }
}

