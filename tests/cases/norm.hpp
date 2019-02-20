#include <cassert>
TEST_CASE( "Matrix norm", "[norm]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat_r = feng::rand<double>( sz, tz );
            feng::matrix<double> const& mat_i = feng::rand<double>( sz, tz );
            feng::matrix<std::complex<double>>  mat_c{ sz, tz };
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                    mat_c[r][c] = std::complex<double>{ mat_r[r][c], mat_i[r][c] };

            auto const& mat_p = feng::norm( mat_c );
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    REQUIRE( std::abs(mat_p[r][c] - std::norm(mat_c[r][c])) < 1.0e-7 );
                }
        }
    }
}

