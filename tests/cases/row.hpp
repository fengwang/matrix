TEST_CASE( "Matrix row", "[row]" )
{
    {
        feng::matrix<int> mat{ 12, 34 };
        REQUIRE( mat.row() == 12 );
    }

    {
        feng::matrix<int> mat;
        REQUIRE( mat.row() == 0 );
    }
}
