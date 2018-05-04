void _0000_make_view()
{
    feng::matrix<double> m;
    m.load_txt( "./images/Lenna.txt" );
    m.save_as_bmp( "./images/0000_make_view.bmp" );

    auto const[r,c] = m.shape();
    auto const& v = feng::make_view( m, {r>>2, (r>>2)*3}, {c>>2, (c>>2)*3} );

    v.save_as_bmp( "./images/0001_make_view.bmp" );

    auto new_matrix{v};
    new_matrix.save_as_bmp( "./images/0002_make_view.bmp" );
}

