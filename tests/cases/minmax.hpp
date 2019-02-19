#include <cassert>
TEST_CASE( "Matrix minmax", "[minmax]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::zeros<double>( sz, tz );
            {
                auto [mn, mx] = mat.minmax();
                REQUIRE( std::abs(mn) < 1.0e-7 );
                REQUIRE( std::abs(mx) < 1.0e-7 );
            }
            {
                auto [mn, mx] = mat.minmax([](auto x, auto y){ return x < y; });
                REQUIRE( std::abs(mn) < 1.0e-7 );
                REQUIRE( std::abs(mx) < 1.0e-7 );
            }
            {
                auto mn = mat.min();
                REQUIRE( std::abs(mn) < 1.0e-7 );
            }
            {
                auto mn = mat.min([](auto x, auto y){ return x < y; } );
                REQUIRE( std::abs(mn) < 1.0e-7 );
            }
            {
                auto mx = mat.max();
                REQUIRE( std::abs(mx) < 1.0e-7 );
            }
            {
                auto mx = mat.max([](auto x, auto y){ return x < y; } );
                REQUIRE( std::abs(mx) < 1.0e-7 );
            }
        }
    }
}

