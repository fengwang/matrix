#include <cassert>
TEST_CASE( "Matrix remquo", "[remquo]" )
{
    {
        feng::matrix<std::tuple<int,int>> dummy{ 100, 100 };
        std::fill( dummy.begin(), dummy.end(), std::make_tuple( 1, 1 ) );
    }
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz );
            feng::matrix<double> const& nat = feng::rand<double>( sz, tz );
            auto const& mat_remquo = feng::remquo( mat, nat );
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    auto [ans_, quo_] = mat_remquo[r][c];
                    int quo = 0;
                    auto ans = std::remquo(mat[r][c], nat[r][c], &quo);
                    REQUIRE( std::abs(ans_ - ans) < 1.0e-7 );
                    REQUIRE( std::abs(quo_ - quo) < 1.0e-7 );
                }
        }
    }
}

