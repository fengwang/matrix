void _0000_plot()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/Lenna.txt" );
    (void)m.plot( "./images/0000_plot_default.bmp" );
    (void)m.plot( "./images/0000_plot_jet.bmp", "jet" );
}

void _0001_plot()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/star.txt" );
    (void)m.plot( "./images/0001_plot_star_hotblue.bmp", "hotblue" );
    (void)m.plot( "./images/0001_plot_star_bluehot.bmp", "bluehot" );
}
