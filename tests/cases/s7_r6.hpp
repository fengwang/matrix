// S7-R6 (PR-10, PR-14): the committed numpy/scipy fixtures in tests/fixtures/s7/ match the S7 APIs within the
// tolerance recorded on each manifest line. The suite runs from the repo root (AGENTS.md), so the paths are
// relative to it. Error measure: ||got - ref||_F / ||ref||_F, or ||got - ref||_F when ref is 0.
// Prints `ORACLE S7 FIXTURES <n>` for tools/check.sh oracle.
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace s7r6
{
    inline std::string const dir = "tests/fixtures/s7/";

    struct line
    {
        std::string name, op;
        double tol = 0;
        std::vector< std::string > files;
    };

    template< typename T >
    double frob( feng::matrix< T > const& m )
    {
        double s = 0;
        for ( auto const& x : m ) s += std::norm( x );
        return std::sqrt( s );
    }

    // relative Frobenius error; +inf on a shape mismatch
    template< typename T >
    double rel_err( feng::matrix< T > const& got, feng::matrix< T > const& ref )
    {
        if ( got.row() != ref.row() || got.col() != ref.col() ) return HUGE_VAL;
        feng::matrix< T > d{ ref.row(), ref.col() };
        for ( std::size_t i = 0; i != ref.size(); ++i ) d.data()[i] = got.data()[i] - ref.data()[i];
        double const r = frob( ref );
        return r == 0 ? frob( d ) : frob( d ) / r;
    }

    template< typename T >
    bool load( std::string const& f, feng::matrix< T >& m )
    {
        return m.load_npy( dir + f );
    }

    // computes the op on the loaded inputs; returns the error, or -1 (with why set) when a file or the op failed
    template< typename T >
    double run( line const& c, std::string& why )
    {
        using R = s7::real_t< T >;
        std::size_t const want = c.op == "lu_solve" ? 3 : 2;
        if ( c.files.size() != want ) { why = "expected " + std::to_string( want ) + " files"; return -1; }
        // singular values are real (<f8) for either element type, so svdvals loads its reference as matrix<R>
        bool const real_ref = c.op == "svdvals";
        std::vector< feng::matrix< T > > in( want );
        feng::matrix< R > sref;
        for ( std::size_t i = 0; i != want; ++i )
        {
            bool const ok = ( real_ref && i + 1 == want ) ? load( c.files[i], sref ) : load( c.files[i], in[i] );
            if ( !ok ) { why = "cannot load " + dir + c.files[i]; return -1; }
        }
        auto const& a = in[0];
        auto const& ref = in.back();
        if ( c.op == "lu_solve" )
        {
            auto x = feng::solve( a, in[1] );
            if ( !x.ok() ) { why = "solve status not ok"; return -1; }
            return rel_err( x.value, ref );
        }
        if ( c.op == "det" )
        {
            feng::matrix< T > d{ 1, 1 };
            d[0][0] = feng::det( a );
            return rel_err( d, ref );
        }
        if ( c.op == "inv" )
        {
            auto x = feng::try_inverse( a );
            if ( !x.ok() ) { why = "try_inverse status not ok"; return -1; }
            return rel_err( x.value, ref );
        }
        if ( c.op == "svdvals" )
        {
            auto const f = feng::svd_factor( a );
            if ( !f.ok() ) { why = "svd_factor status not ok"; return -1; }
            feng::matrix< R > s{ 1, f.s().size() };
            for ( std::size_t i = 0; i != f.s().size(); ++i ) s[0][i] = f.s()[i];
            return rel_err( s, sref );
        }
        if ( c.op == "pinv" )
        {
            feng::matrix< T > x;
            if ( feng::pinverse( a, x, R( -1 ) ) != feng::linalg_status::ok ) { why = "pinverse status not ok"; return -1; }
            return rel_err( x, ref );
        }
        if ( c.op == "cholesky" )
        {
            auto const f = feng::cholesky_factor( a );
            if ( !f.ok() ) { why = "cholesky_factor status not ok"; return -1; }
            return rel_err( f.l(), ref );
        }
        if ( c.op == "rref" )
        {
            auto const r = feng::row_echelon( a );
            if ( r.status != feng::linalg_status::ok ) { why = "row_echelon status not ok"; return -1; }
            return rel_err( r.r, ref );
        }
        if ( c.op == "expm" )
        {
            feng::matrix< T > e;
            if ( feng::expm( a, e ) != feng::linalg_status::ok ) { why = "expm status not ok"; return -1; }
            return rel_err( e, ref );
        }
        why = "unknown op " + c.op;
        return -1;
    }
}

TEST_CASE( "S7-R6 committed fixtures match within their tolerances", "[S7][S7-R6][oracle]" )
{
    std::ifstream in( s7r6::dir + "manifest.txt" );
    REQUIRE( in.good() );
    std::vector< s7r6::line > cases;
    std::string text;
    while ( std::getline( in, text ) )
    {
        if ( text.empty() || text[0] == '#' ) continue;
        std::istringstream is( text );
        s7r6::line c;
        std::string f;
        is >> c.name >> c.op >> c.tol;
        INFO( "manifest line: " << text );
        REQUIRE( !is.fail() );
        while ( is >> f ) c.files.push_back( f );
        cases.push_back( c );
    }
    REQUIRE( cases.size() > 0 );

    std::size_t checked = 0;
    for ( auto const& c : cases )
    {
        // case names are <op>_<f8|c16>_<shape> (tests/fixtures/s7/make_fixtures.py)
        std::string const type = c.name.substr( c.op.size() + 1, c.name.find( '_', c.op.size() + 1 ) - c.op.size() - 1 );
        std::string why;
        double err = -1;
        if ( type == "f8" ) err = s7r6::run< double >( c, why );
        else if ( type == "c16" ) err = s7r6::run< std::complex< double > >( c, why );
        else why = "unknown element type '" + type + "'";
        INFO( "case " << c.name << " (" << c.op << "): error " << err << ", tol " << c.tol << ( why.empty() ? "" : ", " ) << why );
        CHECK( why.empty() );
        CHECK( err >= 0 );
        CHECK( err <= c.tol );
        if ( why.empty() && err >= 0 && err <= c.tol ) ++checked;
        if ( std::getenv( "S7_RATIOS" ) ) std::cout << c.name << " ratio " << err / c.tol << "\n";
    }
    CHECK( checked == cases.size() );
    std::cout << "ORACLE S7 FIXTURES " << checked << "\n";
}
