TEST_CASE( "Matrix size", "[size]" )
{
    {
        feng::matrix<int> mat{ 12, 34 };
        auto const& [r,c] = mat.shape();
        REQUIRE( mat.row() == 12 );
        REQUIRE( mat.col() == 34 );
        REQUIRE( r == 12 );
        REQUIRE( c == 34 );
        REQUIRE( mat.size() == 12*34 );
    }

    {
        feng::matrix<int> mat;
        auto const& [r,c] = mat.shape();
        REQUIRE( mat.row() == 0 );
        REQUIRE( mat.col() == 0 );
        REQUIRE( r == 0 );
        REQUIRE( c == 0 );
        REQUIRE( mat.size() == 0 );
    }
}
