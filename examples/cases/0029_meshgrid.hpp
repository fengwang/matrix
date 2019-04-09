void _0000_meshgrid()
{
    auto const& [X, Y] = feng::meshgrid( 384, 512 );
    X.save_as_bmp( "./images/0000_meshgrid_x.bmp", "grey" );
    Y.save_as_bmp( "./images/0000_meshgrid_y.bmp", "grey" );

    {
        auto const& [X, Y] = feng::meshgrid( 3, 5 );
        std::cout << X << std::endl;
        std::cout << Y << std::endl;
    }
}

