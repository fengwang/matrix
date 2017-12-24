void _0000_save_load()
{
    auto const& m = feng::rand<double>( 128, 128 );
    m.save_as_txt( "./images/0000_save_load.txt" );
    m.save_as_binary( "./images/0000_save_load.bin" );
    m.save_as_bmp( "./images/0000_save_load.bmp" );
    feng::matrix<double> n;
    n.load_txt( "./images/0000_save_load.txt" );
    n.save_as_bmp( "./images/0001_save_load.bmp" );
    n.load_binary( "./images/0000_save_load.bin" );
    n.save_as_pgm( "./images/0002_save_load.pgm" );
}

