void _0000_gauss_jordan_elimination()
{
    auto const& m = feng::rand<double>( 64, 128);
    m.save_as_bmp( "./images/0000_gauss_jordan_elimination.bmp", "gray" );

    auto const& n = feng::gauss_jordan_elimination( m ); //<- also `feng::rref(m);`, alias name from Matlab

    if (n)
        (*n).save_as_bmp( "./images/0001_gauss_jordan_elimination.bmp", "gray" );
    else
        std::cout << "Failed to execute Gauss-Jordan Elimination for matrix m.\n";
}

