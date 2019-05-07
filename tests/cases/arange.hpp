#include <cassert>
TEST_CASE( "Matrix arange", "[arange]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::arange<double>( sz*tz );
            for ( unsigned long c = 0; c != tz*sz; ++c )
            {
                REQUIRE( std::abs(mat[0][c] - c ) < 1.0e-7 );
            }
        }
    }
}

