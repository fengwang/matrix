void _0000_arange()
{
    auto m = feng::arange<double>( 256*256 );
    m.reshape( 256, 256 );
    m.save_as_bmp( "./images/0000_arange.bmp" );
}

