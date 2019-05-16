void _0000_clip()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0000_clip.bmp" );
    m = feng::sin(m);
    m.save_as_bmp( "./images/0001_clip.bmp" );

    auto const& cm0 = feng::clip( 0.1, 0.9 )( m );
    cm0.save_as_bmp( "./images/0002_clip.bmp" );

    auto const& cm1 = feng::clip( 0.4, 0.6 )( m );
    cm1.save_as_bmp( "./images/0003_clip.bmp" );
}
