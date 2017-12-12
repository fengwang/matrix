void _0000_apply()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0000_apply.bmp" );

    m.apply( [](auto& x) { x = std::sin(x); } );
    m.save_as_bmp( "./images/0001_apply.bmp" );
}
