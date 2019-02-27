#include <cassert>
TEST_CASE( "Matrix mean", "[mean]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz );
            //feng::matrix<double> const& mat = feng::ones<double>( sz, tz );
            auto const mat_mean = feng::mean( mat );
            auto const acc_mean = std::accumulate( mat.begin(), mat.end(), 0.0 ) / mat.size();
            REQUIRE( std::abs(mat_mean-acc_mean) < 1.0e-7 );
        }
    }
}

