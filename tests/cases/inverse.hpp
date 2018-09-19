#include <cassert>
TEST_CASE( "Matrix inverse", "[inverse]" )
{
    unsigned long N = 100;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        feng::matrix<double> const& mat = feng::rand<double>( sz, sz, 0 );
        feng::matrix<double> const& inv = mat.inverse();
        feng::matrix<double> const& identity = mat * inv;
        for ( unsigned long r = 0; r != sz; ++r )
            for ( unsigned long c = 0; c != sz; ++c )
            {
                if ( r == c )
                {
                    //better_assert( (std::abs(identity[r][c] - 1.0 ) < 1.0e-5), " but at r == c == ", r, " the value is ", identity[r][c], " with sz ", sz );
                    REQUIRE( (std::abs(identity[r][c] - 1.0 ) < 1.0e-3) );
                }
                else
                {
                    //better_assert( (std::abs(identity[r][c] ) < 1.0e-5), " but at r == ", r, " and c == ", c, " the value is ", identity[r][c], " with sz ", sz );
                    REQUIRE( std::abs(identity[r][c] ) < 1.0e-3 );
                }
            }
    }
}

