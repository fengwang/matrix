#include <cassert>
TEST_CASE( "Matrix pooling", "[pooling]" )
{
    unsigned long N = 10;
    std::vector<unsigned long> dims{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}};
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            for (auto dim : dims )
            {
                feng::matrix<double> const& mat = feng::rand<double>( dim, dim );
                {
                    auto const& mx = feng::pooling( mat, dim, "mean" );
                    REQUIRE( std::abs( mx[0][0] - feng::mean(mat) ) < 1.0e-7 );
                }
                {
                    auto const& mx = feng::pooling( mat, dim, "min" );
                    REQUIRE( std::abs( mx[0][0] - feng::min(mat) ) < 1.0e-7 );
                }
                {
                    auto const& mx = feng::pooling( mat, dim, "max" );
                    REQUIRE( std::abs( mx[0][0] - feng::max(mat) ) < 1.0e-7 );
                }
            }
        }
    }
}

