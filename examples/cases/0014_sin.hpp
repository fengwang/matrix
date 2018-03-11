void _0000_sin()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0000_sin.bmp" );

    m = feng::sin(m);
    m.save_as_bmp( "./images/0001_sin.bmp" );
}
