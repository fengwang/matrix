// Independent probe — Session 1 (branch_and_compare)
//
// PROVENANCE: expected values are the verbatim output of the fresh-context independent
// test-writer subagent (docs/session_1 .work/independent/derivation.md; runId
// wf_msxq6npb-4-94d3a0fc21f7), derived from the documented contracts alone (no header,
// no worker tests, no diff read). File structure/encoding: orchestrator (subagent
// file-writes exceed the 16K output budget in this environment — see
// docs/session_1/failure_arbiter.md record 1).
//
// Build: g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1 -o .work/probe_indep .work/independent/probe_s1_independent.cc

# include "../../matrix.hpp"
# include <cmath>
# include <cstdio>

static int failures = 0;

static bool eq( double a, double b )
{
    return std::fabs( a - b ) < 1.0e-12;
}

static void check( char const* id, bool ok, char const* detail )
{
    if ( ok ) printf( "PASS %s\n", id );
    else { printf( "FAIL %s: %s\n", id, detail ); ++failures; }
}

int main()
{
    bool e01_ok = true, e02_ok = true;

    // i1: 5x5 of 1.0 -> shrink_to_size(5,3) => "5x3 all 1.0"
    {
        feng::matrix<double> m{ 5, 5, 1.0 };
        m.shrink_to_size( 5, 3 );
        bool ok = ( m.row() == 5 ) && ( m.col() == 3 );
        for ( unsigned long r = 0; ok && r < 5; ++r )
            for ( unsigned long c = 0; ok && c < 3; ++c )
                ok = ok && eq( m[r][c], 1.0 );
        check( "i1", ok, "expected 5x3 all 1.0" );
        e01_ok = e01_ok && ok;
    }

    // i2: 3x10 of 1..30 -> shrink_to_size(5,2) => [[1,2],[11,12],[21,22],[0,0],[0,0]]
    {
        feng::matrix<double> m{ 3, 10, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                         11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
                                         21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0 } };
        m.shrink_to_size( 5, 2 );
        double const expected[5][2] = { { 1.0, 2.0 }, { 11.0, 12.0 }, { 21.0, 22.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
        bool ok = ( m.row() == 5 ) && ( m.col() == 2 );
        for ( unsigned long r = 0; ok && r < 5; ++r )
            for ( unsigned long c = 0; ok && c < 2; ++c )
                ok = ok && eq( m[r][c], expected[r][c] );
        check( "i2", ok, "expected [[1,2],[11,12],[21,22],[0,0],[0,0]]" );
        e01_ok = e01_ok && ok;
    }

    // i3: 1x1 of 7.0 -> shrink_to_size(4,4) => 7.0 at [0][0], all other 15 elements 0
    {
        feng::matrix<double> m{ 1, 1, 7.0 };
        m.shrink_to_size( 4, 4 );
        bool ok = ( m.row() == 4 ) && ( m.col() == 4 ) && eq( m[0][0], 7.0 );
        for ( unsigned long r = 0; ok && r < 4; ++r )
            for ( unsigned long c = 0; ok && c < 4; ++c )
                if ( ! ( ( r == 0 ) && ( c == 0 ) ) )
                    ok = ok && eq( m[r][c], 0.0 );
        check( "i3", ok, "expected 4x4 with 7.0 at [0][0], all other 15 elements 0" );
        e01_ok = e01_ok && ok;
    }

    // i4: 3x5 of 1..15 -> flipdim(.,2) => [[5,4,3,2,1],[10,9,8,7,6],[15,14,13,12,11]]
    {
        feng::matrix<double> m{ 3, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 2 );
        double const expected[3][5] = { { 5.0, 4.0, 3.0, 2.0, 1.0 }, { 10.0, 9.0, 8.0, 7.0, 6.0 }, { 15.0, 14.0, 13.0, 12.0, 11.0 } };
        bool ok = ( f.row() == 3 ) && ( f.col() == 5 );
        for ( unsigned long r = 0; ok && r < 3; ++r )
            for ( unsigned long c = 0; ok && c < 5; ++c )
                ok = ok && eq( f[r][c], expected[r][c] );
        check( "i4", ok, "expected [[5,4,3,2,1],[10,9,8,7,6],[15,14,13,12,11]]" );
        e02_ok = e02_ok && ok;
    }

    // i5: 4x4 of 1..16 -> flipdim(.,2) => [[4,3,2,1],[8,7,6,5],[12,11,10,9],[16,15,14,13]]
    {
        feng::matrix<double> m{ 4, 4, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 2 );
        double const expected[4][4] = { { 4.0, 3.0, 2.0, 1.0 }, { 8.0, 7.0, 6.0, 5.0 }, { 12.0, 11.0, 10.0, 9.0 }, { 16.0, 15.0, 14.0, 13.0 } };
        bool ok = ( f.row() == 4 ) && ( f.col() == 4 );
        for ( unsigned long r = 0; ok && r < 4; ++r )
            for ( unsigned long c = 0; ok && c < 4; ++c )
                ok = ok && eq( f[r][c], expected[r][c] );
        check( "i5", ok, "expected [[4,3,2,1],[8,7,6,5],[12,11,10,9],[16,15,14,13]]" );
        e02_ok = e02_ok && ok;
    }

    // i6: 3x5 of 1..15 -> flipdim(.,1) => [[11,12,13,14,15],[6,7,8,9,10],[1,2,3,4,5]]
    {
        feng::matrix<double> m{ 3, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 } };
        feng::matrix<double> const f = feng::flipdim( m, 1 );
        double const expected[3][5] = { { 11.0, 12.0, 13.0, 14.0, 15.0 }, { 6.0, 7.0, 8.0, 9.0, 10.0 }, { 1.0, 2.0, 3.0, 4.0, 5.0 } };
        bool ok = ( f.row() == 3 ) && ( f.col() == 5 );
        for ( unsigned long r = 0; ok && r < 3; ++r )
            for ( unsigned long c = 0; ok && c < 5; ++c )
                ok = ok && eq( f[r][c], expected[r][c] );
        check( "i6", ok, "expected [[11,12,13,14,15],[6,7,8,9,10],[1,2,3,4,5]]" );
        e02_ok = e02_ok && ok;
    }

    if ( e01_ok ) printf( "PASS independent-E01\n" );
    if ( e02_ok ) printf( "PASS independent-E02\n" );
    return ( failures == 0 ) ? 0 : 1;
}
