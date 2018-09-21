void _0000_save_with_colormap()
{
    feng::matrix<double> m;
    m.load_txt( "./images/Lenna.txt" );
    m.save_as_bmp( "./images/0000_save_with_colormap_default.bmp" );
    m.save_as_bmp( "./images/0000_save_with_colormap_parula.bmp", "parula" );
    m.save_as_bmp( "./images/0000_save_with_colormap_hotblue.bmp", "hotblue" );
    m.save_as_bmp( "./images/0000_save_with_colormap_bluehot.bmp", "bluehot" );
    m.save_as_bmp( "./images/0000_save_with_colormap_jet.bmp", "jet" );
    m.save_as_bmp( "./images/0000_save_with_colormap_obscure.bmp", "obscure" );
    m.save_as_bmp( "./images/0000_save_with_colormap_gray.bmp", "gray" );

    m.save_as_bmp( "./images/0000_save_with_colormap_hsv.bmp", "hsv" );
    m.save_as_bmp( "./images/0000_save_with_colormap_spring.bmp", "spring" );
    m.save_as_bmp( "./images/0000_save_with_colormap_summer.bmp", "summer" );
    m.save_as_bmp( "./images/0000_save_with_colormap_autumn.bmp", "autumn" );
    m.save_as_bmp( "./images/0000_save_with_colormap_winter.bmp", "winter" );
    m.save_as_bmp( "./images/0000_save_with_colormap_pink.bmp", "pink" );
    m.save_as_bmp( "./images/0000_save_with_colormap_hot.bmp", "hot" );
    m.save_as_bmp( "./images/0000_save_with_colormap_cool.bmp", "cool" );
    m.save_as_bmp( "./images/0000_save_with_colormap_bone.bmp", "bone" );
    m.save_as_bmp( "./images/0000_save_with_colormap_copper.bmp", "copper" );
    m.save_as_bmp( "./images/0000_save_with_colormap_lines.bmp", "lines" );
}

void _0001_save_with_colormap()
{
    feng::matrix<double> m;
    m.load_txt( "./images/star.txt" );
    m.save_as_bmp( "./images/0001_star_hotblue.bmp", "hotblue" );
    m.save_as_bmp( "./images/0001_star_bluehot.bmp", "bluehot" );
}
