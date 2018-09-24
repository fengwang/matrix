#include <cassert>
TEST_CASE( "Matrix zeros", "[zeros]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::zeros<double>( sz, tz );
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    REQUIRE( std::abs(mat[r][c] ) < 1.0e-7 );
                }
        }
    }
}

