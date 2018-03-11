void _0000_eye()
{
    auto const& m = feng::eye<double>( 128, 128 );
    m.save_as_bmp( "./images/0000_eye.bmp" );
}
