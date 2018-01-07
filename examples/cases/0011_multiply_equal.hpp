void _0000_multiply_equal()
{
    auto m = feng::rand( 127, 127 );
    m *= m.inverse();

    m.save_as_bmp("images/0001_multiply_equal.bmp");
}

