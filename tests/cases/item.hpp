#include <cassert>
TEST_CASE( "Matrix item", "[item]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( 1, 1 );
            auto const& item = mat.item();
            REQUIRE( std::abs(item-mat[0][0]) < 1.0e-7 );
        }
    }
}

