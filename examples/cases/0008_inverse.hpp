void _0000_inverse()
{
    auto const& m = feng::rand<double>( 128, 128 );
    auto const& n = m.inverse();
    auto const& identity = m * n;
    identity.save_as_bmp( "./images/0000_inverse.bmp" );
}

