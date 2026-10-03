void _0000_save_with_colormap()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/Lenna.txt" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_default.bmp" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_parula.bmp", "parula" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_hotblue.bmp", "hotblue" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_bluehot.bmp", "bluehot" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_jet.bmp", "jet" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_obscure.bmp", "obscure" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_gray.bmp", "gray" );

    (void)m.save_as_bmp( "./images/0000_save_with_colormap_hsv.bmp", "hsv" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_spring.bmp", "spring" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_summer.bmp", "summer" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_autumn.bmp", "autumn" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_winter.bmp", "winter" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_pink.bmp", "pink" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_hot.bmp", "hot" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_cool.bmp", "cool" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_bone.bmp", "bone" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_copper.bmp", "copper" );
    (void)m.save_as_bmp( "./images/0000_save_with_colormap_lines.bmp", "lines" );

    (void)m.save_as_png( "./images/0000_save_with_colormap_default.png" );
    (void)m.save_as_png( "./images/0000_save_with_colormap_parula.png", "parula" );
}

void _0001_save_with_colormap()
{
    feng::matrix<double> m;
    (void)m.load_txt( "./images/star.txt" );
    (void)m.save_as_bmp( "./images/0001_star_hotblue.bmp", "hotblue" );
    (void)m.save_as_bmp( "./images/0001_star_bluehot.bmp", "bluehot" );
    (void)m.save_as_bmp( "./images/0001_star_hotgreen.bmp", "hotgreen" );
    (void)m.save_as_bmp( "./images/0001_star_greenhot.bmp", "greenhot" );
    (void)m.save_as_bmp( "./images/0001_star_tealhot.bmp", "tealhot" );
}
