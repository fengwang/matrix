// E01/E02 probes — Session 1 (C1: shrink_to_size wrong column count; C2: flipdim dim==2 col-vs-row swap)
//
// Build (contract deterministic check):
//   g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1 -o .work/probe_s1 .work/probes/E01_E02.cc
// Run:
//   .work/probe_s1            # all cases (post-fix: prints PASS E01 / PASS E02, exit 0)
//   .work/probe_s1 <case>     # one case: e01 e02 e01a e01b e01c e02a e02b e02c (pre-flight reproduction runs)
//
// -DNDEBUG is deliberate (project contract §4): better_assert is silent, real OOB behavior is observable.
// Pre-fix expectation (probe-first, P5):
//   e01a: ASan heap-buffer-overflow (5x5 -> 5x3)
//   e01b: ASan report and/or content corruption (3x10 -> 5x2)
//   e02a: ASan heap-buffer-overflow (3x5 flipdim dim 2)
//   e02b: content corruption, no ASan report (4x4 flipdim dim 2, review's silent case)
//   e02c: PASS even pre-fix (dim==1 branch is correct — regression pin)

# include "../../matrix.hpp"
# include <cmath>
# include <cstdio>
# include <string>

static int failures = 0;

static bool value_eq( double a, double b )
{
    return std::fabs( a - b ) < 1.0e-12;
}

static void fail( char const* id, char const* what )
{
    printf( "FAIL %s: %s\n", id, what );
    ++failures;
}

static void pass( char const* id )
{
    printf( "PASS %s\n", id );
}

// e01a — review C1 ASan reproduction: 5x5 of 1.0 -> 5x3. Post-fix: 5x3, all 1.0.
static void case_e01a()
{
    feng::matrix<double> m{ 5, 5, 1.0 };
    m.shrink_to_size( 5, 3 );
    bool ok = ( m.row() == 5 ) && ( m.col() == 3 );
    for ( unsigned long r = 0; ok && r < m.row(); ++r )
        for ( unsigned long c = 0; ok && c < m.col(); ++c )
            ok = ok && value_eq( m[r][c], 1.0 );
    if ( ok ) pass( "e01a" ); else fail( "e01a", "5x5->5x3: expected 5x3 all 1.0" );
}

// e01b — review C1 silent-corruption reproduction: 3x10 (values 1..30) -> 5x2.
// Documented contract (matrix.hpp comment ~3515-3517): keep min(row,new_row) rows,
// min(col,new_col) cols, zero-pad the growth region.
static void case_e01b()
{
    feng::matrix<double> m{ 3, 10, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                     11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
                                     21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0 } };
    m.shrink_to_size( 5, 2 );
    double const expected[5][2] = { { 1.0, 2.0 }, { 11.0, 12.0 }, { 21.0, 22.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
    bool ok = ( m.row() == 5 ) && ( m.col() == 2 );
    for ( unsigned long r = 0; ok && r < 5; ++r )
        for ( unsigned long c = 0; ok && c < 2; ++c )
            ok = ok && value_eq( m[r][c], expected[r][c] );
    if ( ok ) pass( "e01b" );
    else
    {
        fail( "e01b", "3x10->5x2: expected [[1,2],[11,12],[21,22],[0,0],[0,0]]" );
        printf( "  got row0: %g %g | row2: %g %g | row4: %g %g\n", m[0][0], m[0][1], m[2][0], m[2][1], m[4][0], m[4][1] );
    }
}

// e01c — contract acceptance: {1,1,7}.shrink_to_size(4,4) zero-pads (grow-only extreme).
static void case_e01c()
{
    feng::matrix<double> m{ 1, 1, 7.0 };
    m.shrink_to_size( 4, 4 );
    bool ok = ( m.row() == 4 ) && ( m.col() == 4 ) && value_eq( m[0][0], 7.0 );
    for ( unsigned long r = 0; ok && r < 4; ++r )
        for ( unsigned long c = 0; ok && c < 4; ++c )
            ok = ok && ( ( r == 0 ) && ( c == 0 ) ? value_eq( m[r][c], 7.0 ) : value_eq( m[r][c], 0.0 ) );
    if ( ok ) pass( "e01c" ); else fail( "e01c", "1x1->4x4: expected 7.0 at [0][0], zeros elsewhere" );
}

// e02a — review C2 ASan reproduction: 3x5 (values 1..15), flipdim(.,2) = left-right flip.
static void case_e02a()
{
    feng::matrix<double> m{ 3, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 } };
    feng::matrix<double> const f = feng::flipdim( m, 2 );
    double const expected[3][5] = { { 5.0, 4.0, 3.0, 2.0, 1.0 }, { 10.0, 9.0, 8.0, 7.0, 6.0 }, { 15.0, 14.0, 13.0, 12.0, 11.0 } };
    bool ok = ( f.row() == 3 ) && ( f.col() == 5 );
    for ( unsigned long r = 0; ok && r < 3; ++r )
        for ( unsigned long c = 0; ok && c < 5; ++c )
            ok = ok && value_eq( f[r][c], expected[r][c] );
    if ( ok ) pass( "e02a" ); else fail( "e02a", "3x5 flipdim(.,2): expected per-row reversed 1..15" );
}

// e02b — review C2 silent-corruption reproduction: 4x4 (values 1..16), flipdim(.,2) = left-right flip.
static void case_e02b()
{
    feng::matrix<double> m{ 4, 4, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0 } };
    feng::matrix<double> const f = feng::flipdim( m, 2 );
    bool ok = ( f.row() == 4 ) && ( f.col() == 4 );
    for ( unsigned long r = 0; ok && r < 4; ++r )
        for ( unsigned long c = 0; ok && c < 4; ++c )
            ok = ok && value_eq( f[r][c], m[r][3 - c] );
    if ( ok ) pass( "e02b" ); else fail( "e02b", "4x4 flipdim(.,2): expected f[r][c] == m[r][3-c]" );
}

// e02c — dim==1 regression pin (branch must stay untouched): 3x5 flipdim(.,1) = up-down flip.
static void case_e02c()
{
    feng::matrix<double> m{ 3, 5, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 } };
    feng::matrix<double> const f = feng::flipdim( m, 1 );
    bool ok = ( f.row() == 3 ) && ( f.col() == 5 );
    for ( unsigned long r = 0; ok && r < 3; ++r )
        for ( unsigned long c = 0; ok && c < 5; ++c )
            ok = ok && value_eq( f[r][c], m[2 - r][c] );
    if ( ok ) pass( "e02c" ); else fail( "e02c", "3x5 flipdim(.,1): expected rows in reverse order" );
}

static bool want( char const* const which, char const* const name )
{
    return std::string( which ) == "all" || std::string( which ) == name;
}

int main( int argc, char const* const* argv )
{
    char const* which = ( argc > 1 ) ? argv[1] : "all";

    if ( want( which, "e01" ) || want( which, "e01a" ) ) case_e01a();
    if ( want( which, "e01" ) || want( which, "e01b" ) ) case_e01b();
    if ( want( which, "e01" ) || want( which, "e01c" ) ) case_e01c();
    if ( want( which, "e02" ) || want( which, "e02a" ) ) case_e02a();
    if ( want( which, "e02" ) || want( which, "e02b" ) ) case_e02b();
    if ( want( which, "e02" ) || want( which, "e02c" ) ) case_e02c();

    if ( failures != 0 )
        return 1;

    if ( std::string( which ) == "all" || std::string( which ) == "e01" )
        printf( "PASS E01\n" );
    if ( std::string( which ) == "all" || std::string( which ) == "e02" )
        printf( "PASS E02\n" );
    return 0;
}
