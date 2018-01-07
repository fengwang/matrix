void _0000_prefix()
{
    auto const& m = feng::random<double>( 127, 127 );
    auto const& pp = +m;
    auto const& pm = -m;
    auto const& shoule_be_zero = pp + pm;
    shoule_be_zero.save_as_bmp("images/0000_prefix.bmp");
}

