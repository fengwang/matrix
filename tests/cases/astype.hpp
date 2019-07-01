#include <cassert>
TEST_CASE( "Matrix astype", "[astype]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz ) * 10.0 - 5.0;
            auto const& mat_astype = mat.astype<int>();
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    REQUIRE( std::abs(std::trunc(mat[r][c]) - mat_astype[r][c] ) < 1.0e-7 );
                }
        }
    }
}

