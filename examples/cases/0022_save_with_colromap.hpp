void _0000_save_with_colormap()
{
    feng::matrix<double> m;
    m.load_txt( "./images/Lenna.txt" );
    m.save_as_bmp( "./images/0000_save_with_colormap_default.bmp" );
    m.save_as_bmp( "./images/0000_save_with_colormap_parula.bmp", "parula" );
    m.save_as_bmp( "./images/0000_save_with_colormap_hotblue.bmp", "hotblue" );
    m.save_as_bmp( "./images/0000_save_with_colormap_jet.bmp", "jet" );
    m.save_as_bmp( "./images/0000_save_with_colormap_obscure.bmp", "obscure" );
    m.save_as_bmp( "./images/0000_save_with_colormap_gray.bmp", "gray" );
}

