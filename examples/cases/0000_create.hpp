void _0000_create()
{
    feng::matrix<double> m{ 64, 128 };

    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );

    m.save_as_bmp( "./images/0000_create.bmp" );
}
