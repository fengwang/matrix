void _0000_astype()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return std::sin(init); }; }() );
    m = m * 2.0;
    m.save_as_bmp( "./images/0000_astype.bmp" );

    auto const& mm = m.astype<int>();
    mm.save_as_bmp( "./images/0001_astype.bmp" );
}
