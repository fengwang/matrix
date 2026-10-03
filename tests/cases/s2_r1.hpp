// S2-R1 (PR-2): one always-on violation path; runs in debug and NDEBUG builds.
#include "./s2_death.hpp"

TEST_CASE( "S2 violation path aborts with one message", "[S2][S2-R1]" )
{
    SECTION( "m(5, 0) on a 2x2 matrix aborts with exactly one stderr line" )
    {
        auto const out = s2_death::run( []{ feng::matrix<double> m{ 2, 2 }; double volatile x = m( 5, 0 ); (void)x; } );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( out.err.rfind( "feng::matrix: contract violation: ", 0 ) == 0 );
        REQUIRE( s2_death::line_count( out.err ) == 1 );
        REQUIRE( out.err.back() == '\n' );
        REQUIRE_FALSE( s2_death::contains( out.err, "AddressSanitizer" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "runtime error:" ) );
    }
    SECTION( "the message names the site's text" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 2, 2 }; double volatile x = m( 5, 0 ); (void)x; }, "Row index out of boundary" );
        S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 2, 2 }; m( 0, 7 ) = 1.0; }, "Column index out of boundary" );
    }
    SECTION( "a child whose callable returns exits 0 and is not a death" )
    {
        auto const out = s2_death::run( []{} );
        REQUIRE_FALSE( out.signaled );
        REQUIRE( out.exited );
        REQUIRE( out.exit_code == 0 );
    }
}
