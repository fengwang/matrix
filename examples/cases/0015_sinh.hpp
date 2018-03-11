void _0000_sinh()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init/500.0; }; }() );
    m.save_as_bmp( "./images/0000_sinh.bmp" );

    m = feng::sinh(m);
    m.save_as_bmp( "./images/0001_sinh.bmp" );
}
