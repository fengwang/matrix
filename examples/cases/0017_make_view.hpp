void _0000_make_view()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/Lenna.txt" );
    (void)m.save_as_bmp( "./images/0000_make_view.bmp" );

    auto const[r,c] = m.shape();
    auto const& v = feng::make_view( m, {r>>2, (r>>2)*3}, {c>>2, (c>>2)*3} );

    (void)v.save_as_bmp( "./images/0001_make_view.bmp" );

    auto new_matrix{v};
    (void)new_matrix.save_as_bmp( "./images/0002_make_view.bmp" );

    feng::matrix<double> n{ v.row(), v.col() }; // row() and col() of a matrix view
    for ( auto r = 0UL; r != n.row(); ++r )
        for ( auto c = 0UL; c != n.col(); ++c )
            n[r][c] = v[r][c]; // accessing matrix elements using operator [], read-only
    (void)n.save_as_bmp( "./images/0003_make_view.bmp", "gray" );
}

