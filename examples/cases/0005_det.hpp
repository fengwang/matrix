void _0000_det()
{
    feng::matrix<double> m{ 128, 128 };

    std::generate( m.diag_begin(), m.diag_end(), [](){ double x = 0.9; return [x]() mutable { x+= 0.156; return x; }(); } );

    double det1 = m.det();
    double det2 = std::accumulate( m.diag_begin(), m.diag_end(), 1.0, []( double x, double y ){ return x*y; } );

    std::cout.precision( 15 );
    std::cout << det1 << "\t:\t" << det2 << std::endl;
}

