void _0000_plot()
{
    feng::matrix<double> m;
    m.load_txt( "./images/Lenna.txt" );
    m.plot( "./images/0000_plot_default.bmp" );
    m.plot( "./images/0000_plot_jet.bmp", "jet" );
}

void _0001_plot()
{
    feng::matrix<double> m;
    m.load_txt( "./images/star.txt" );
    m.plot( "./images/0001_plot_star_hotblue.bmp", "hotblue" );
    m.plot( "./images/0001_plot_star_bluehot.bmp", "bluehot" );
}
