void _0000_divide_equal()
{
    auto m = feng::rand<double>( 197, 197 );
    auto n = m;
    n /= 2.0;
    m /= n;

    m.save_as_bmp( "images/0000_divide_equal.bmp" );
}

