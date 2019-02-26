#include <cassert>
TEST_CASE( "Matrix fmin", "[fmin]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz );
            feng::matrix<double> const& nat = feng::rand<double>( sz, tz );
            auto const& mat_fmin = feng::fmin( mat, nat );
            auto const& mat_fmin_m = feng::fmin( mat[0][0], nat );
            auto const& mat_fmin_n = feng::fmin( mat, nat[0][0] );
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    REQUIRE( std::abs(mat_fmin[r][c] - std::fmin(mat[r][c], nat[r][c])) < 1.0e-7 );
                    REQUIRE( std::abs(mat_fmin_m[r][c] - std::fmin(mat[0][0], nat[r][c])) < 1.0e-7 );
                    REQUIRE( std::abs(mat_fmin_n[r][c] - std::fmin(mat[r][c], nat[0][0])) < 1.0e-7 );
                }
        }
    }
}

