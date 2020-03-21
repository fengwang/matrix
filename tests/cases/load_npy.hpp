#include <cassert>
TEST_CASE( "Loading npy files", "[load_npy]" )
{
    {
        feng::matrix<std::uint8_t> m;
        m.load_npy( "./images/u8.npy" );
        REQUIRE( m[0][0] == 4 ); REQUIRE( m[0][1] == 1 ); REQUIRE( m[0][2] == 8 );
        REQUIRE( m[1][0] == 9 ); REQUIRE( m[1][1] == 1 ); REQUIRE( m[1][2] == 5 );
    }
    {
        feng::matrix<std::int8_t> m;
        m.load_npy( "./images/8.npy" );
        REQUIRE( m[0][0] == 4 ); REQUIRE( m[0][1] == 1 ); REQUIRE( m[0][2] == 8 );
        REQUIRE( m[1][0] == 9 ); REQUIRE( m[1][1] == 1 ); REQUIRE( m[1][2] == 5 );
    }
    // [[4.815519 , 1.0601262, 8.989337 ],
    //  [9.510697 , 1.8137231, 5.7381544]]
    {
        feng::matrix<float> m;
        m.load_npy( "./images/32.npy" );
        REQUIRE( std::abs(4.815519-m[0][0]) < 1.0e-5 ); REQUIRE( std::abs(1.0601262-m[0][1]) < 1.0e-5 ); REQUIRE( std::abs(8.989337-m[0][2]) < 1.0e-5 );
        REQUIRE( std::abs(9.510697-m[1][0]) < 1.0e-5 ); REQUIRE( std::abs(1.8137231-m[1][1]) < 1.0e-5 ); REQUIRE( std::abs(5.7381544-m[1][2]) < 1.0e-5 );
    }
    {
        feng::matrix<double> m;
        m.load_npy( "./images/64.npy" );
        REQUIRE( std::abs(4.815519-m[0][0]) < 1.0e-5 ); REQUIRE( std::abs(1.0601262-m[0][1]) < 1.0e-5 ); REQUIRE( std::abs(8.989337-m[0][2]) < 1.0e-5 );
        REQUIRE( std::abs(9.510697-m[1][0]) < 1.0e-5 ); REQUIRE( std::abs(1.8137231-m[1][1]) < 1.0e-5 ); REQUIRE( std::abs(5.7381544-m[1][2]) < 1.0e-5 );
    }
}

