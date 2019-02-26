#include <cassert>
TEST_CASE( "Matrix fma", "[fma]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz );
            feng::matrix<double> const& nat = feng::rand<double>( sz, tz );
            feng::matrix<double> const& lat = feng::rand<double>( sz, tz );
            auto const& mat_fma = feng::fma( mat, nat, lat );
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    REQUIRE( std::abs(mat_fma[r][c] - std::fma(mat[r][c], nat[r][c], lat[r][c])) < 1.0e-7 );
                }
        }
    }
}

