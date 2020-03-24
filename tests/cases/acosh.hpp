TEST_CASE( "Matrix acosh", "[acosh]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz ) + 1.0;
            auto const& mat_acosh = feng::acosh( mat );
            for ( unsigned long r = 0; r != sz; ++r )
                for ( unsigned long c = 0; c != tz; ++c )
                {
                    REQUIRE( std::abs(std::acosh(mat[r][c]) - mat_acosh[r][c] ) < 1.0e-7 );
                }
        }
    }
}
