void _0000_slicing()
{
    feng::matrix<double> m{ 64, 256 };
    std::fill( m.upper_diag_begin(1), m.upper_diag_end(1), 1.0 );
    std::fill( m.diag_begin(), m.diag_end(), 1.0 );
    std::fill( m.lower_diag_begin(1), m.lower_diag_end(1), 1.0 );
    m.save_as_bmp( "./images/0000_slicing.bmp" );

    feng::matrix<double> n{ m, 0, 32, 0, 64 };
    n.save_as_bmp( "./images/0001_slicing.bmp" );

    feng::matrix<double> p{ m, {16, 48}, {0, 64} };
    p.save_as_bmp( "./images/0002_slicing.bmp" );
}

