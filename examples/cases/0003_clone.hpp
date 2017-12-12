void _0000_clone()
{
    feng::matrix<double> m{ 64, 256 };
    //std::generate( m.begin(), m.end(), [](){ double x = 0.0; return [x]()mutable{ x+=0.1; return std::sin(x);}; }() );
    std::fill( m.diag_begin(), m.diag_end(), 1.1 );
    m.save_as_bmp( "./images/0000_clone.bmp" );

    auto n = m.clone( 0, 32, 0, 64 );
    n.save_as_bmp( "./images/0001_clone.bmp" );

    n.clone( m, 32, 64, 0, 64 );
    n.save_as_bmp( "./images/0002_clone.bmp" );
}
