// S7-R1 (PR-10, PR-2): one partial-pivoting LU object; legacy lu_decomposition and lu_solver rest on it (F13).
// The helpers in namespace s7 are shared with tests/cases/s7_r2.hpp (included after this file).
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "./s2_death.hpp"

namespace s7
{
    template< typename T > struct real_of { using type = T; };
    template< typename T > struct real_of< std::complex< T > > { using type = T; };
    template< typename T > using real_t = typename real_of< T >::type;

    template< typename T > real_t< T > eps() { return std::numeric_limits< real_t< T > >::epsilon(); }

    template< typename T > using mat = feng::matrix< T >;

    // ∞-norm: largest absolute row sum
    template< typename T >
    real_t< T > norm_inf( mat< T > const& m )
    {
        real_t< T > best{ 0 };
        for ( std::size_t r = 0; r != m.row(); ++r )
        {
            real_t< T > s{ 0 };
            for ( std::size_t c = 0; c != m.col(); ++c ) s += std::abs( m[r][c] );
            best = s > best ? s : best;
        }
        return best;
    }

    template< typename T >
    mat< T > from_real( std::size_t r, std::size_t c, std::vector< double > const& v )
    {
        mat< T > m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i ) m[i / c][i % c] = static_cast< T >( static_cast< real_t< T > >( v[i] ) );
        return m;
    }

    template< typename T >
    mat< T > hilbert( std::size_t n )
    {
        mat< T > m{ n, n };
        for ( std::size_t i = 0; i != n; ++i )
            for ( std::size_t j = 0; j != n; ++j )
                m[i][j] = T( real_t< T >( 1 ) / real_t< T >( i + j + 1 ) );
        return m;
    }

    template< typename T >
    mat< T > random_square( std::size_t n, std::uint_least64_t seed )
    {
        std::mt19937_64 g{ seed };
        return feng::random< T >( n, n, g );
    }

    // the R09 inputs: identity, diagonal, row swap, 4×4 block exchange, random n = 1..12, Hilbert 8 and the
    // near-singular 2×2
    template< typename T >
    std::vector< std::pair< std::string, mat< T > > > r09_inputs()
    {
        std::vector< std::pair< std::string, mat< T > > > v;
        v.emplace_back( "identity 5", feng::eye< T >( 5, 5 ) );
        v.emplace_back( "diagonal", from_real< T >( 3, 3, { 2, 0, 0, 0, -3, 0, 0, 0, 0.5 } ) );
        v.emplace_back( "row swap", from_real< T >( 3, 3, { 0, 1, 0, 1, 0, 0, 0, 0, 1 } ) );
        v.emplace_back( "block exchange", from_real< T >( 4, 4, { 0, 0, 1, 2, 0, 0, 3, 4, 5, 6, 0, 0, 7, 8, 0, 0 } ) );
        for ( std::size_t n = 1; n <= 12; ++n )
            v.emplace_back( "random " + std::to_string( n ), random_square< T >( n, 0x5701ULL + n ) );
        if constexpr ( sizeof( real_t< T > ) >= sizeof( double ) )
        {
            v.emplace_back( "hilbert 8", hilbert< T >( 8 ) );
            v.emplace_back( "near singular 2x2", from_real< T >( 2, 2, { 1, 1, 1, 1 + 1e-10 } ) );
        }
        else
        {
            // float: Hilbert 8 and 1 + 1e-10 are singular at ε = 1.2e-7 (float_singular_inputs); these keep the
            // near-singular shape at float precision
            v.emplace_back( "hilbert 4", hilbert< T >( 4 ) );
            v.emplace_back( "near singular 2x2", from_real< T >( 2, 2, { 1, 1, 1, 1 + 1e-3 } ) );
        }
        return v;
    }

    // the double near-singular inputs read singular in float (D-027 tolerance n·ε·max|U|), with the rank shown
    template< typename T >
    std::vector< std::pair< mat< T >, std::size_t > > float_singular_inputs()
    {
        return { { hilbert< T >( 8 ), 6 }, { from_real< T >( 2, 2, { 1, 1, 1, 1 + 1e-10 } ), 1 } };
    }

    template< typename T >
    mat< T > permute_rows( mat< T > const& a, std::vector< std::size_t > const& piv )
    {
        mat< T > pa{ a.row(), a.col() };
        for ( std::size_t i = 0; i != a.row(); ++i )
            for ( std::size_t j = 0; j != a.col(); ++j ) pa[i][j] = a[piv[i]][j];
        return pa;
    }

    template< typename T >
    void check_lu_reconstructs()
    {
        for ( auto const& [name, a] : r09_inputs< T >() )
        {
            INFO( name );
            std::size_t const n = a.row();
            auto const f = feng::lu_factor( a );
            REQUIRE( f.status() == feng::linalg_status::ok );
            REQUIRE( f.rank() == n );
            auto const& piv = f.pivots();
            REQUIRE( piv.size() == n );
            mat< T > const l = f.l(), u = f.u(), p = f.p();
            REQUIRE( l.row() == n ); REQUIRE( l.col() == n );
            REQUIRE( u.row() == n ); REQUIRE( u.col() == n );
            for ( std::size_t i = 0; i != n; ++i )
            {
                REQUIRE( l[i][i] == T( 1 ) );
                for ( std::size_t j = i + 1; j != n; ++j ) { REQUIRE( l[i][j] == T( 0 ) ); REQUIRE( u[j][i] == T( 0 ) ); }
                for ( std::size_t j = 0; j != i; ++j ) REQUIRE( std::abs( l[i][j] ) <= real_t< T >( 1 ) ); // partial pivoting
            }
            mat< T > const pa = permute_rows( a, piv );
            REQUIRE( norm_inf< T >( p * a - pa ) == real_t< T >( 0 ) );
            real_t< T > const res = norm_inf< T >( pa - l * u );
            REQUIRE( res <= real_t< T >( 4 ) * real_t< T >( n ) * eps< T >() * norm_inf( a ) );
        }
        if constexpr ( sizeof( real_t< T > ) < sizeof( double ) )
            for ( auto const& [a, rank] : float_singular_inputs< T >() )
            {
                std::size_t const n = a.row();
                auto const f = feng::lu_factor( a );
                REQUIRE( f.status() == feng::linalg_status::singular );
                REQUIRE( f.rank() == rank );
                REQUIRE( norm_inf< T >( permute_rows( a, f.pivots() ) - f.l() * f.u() ) <= real_t< T >( 4 ) * real_t< T >( n ) * eps< T >() * norm_inf( a ) );
            }
    }

    // ‖AX − B‖ / (‖A‖‖X‖ + ‖B‖) / (n·ε)
    template< typename T >
    real_t< T > solve_ratio( mat< T > const& a, mat< T > const& x, mat< T > const& b )
    {
        real_t< T > const n = static_cast< real_t< T > >( a.row() );
        return norm_inf< T >( a * x - b ) / ( norm_inf( a ) * norm_inf( x ) + norm_inf( b ) ) / ( n * eps< T >() );
    }
}

TEST_CASE( "S7-R1 pivoted LU reconstructs P·A on the R09 inputs", "[S7][S7-R1]" )
{
    s7::check_lu_reconstructs< double >();
    s7::check_lu_reconstructs< std::complex< double > >();
    s7::check_lu_reconstructs< float >();
    s7::check_lu_reconstructs< std::complex< float > >();
}

namespace s7
{
    template< typename T >
    void check_pivot_rules()
    {
        // the first entry of largest |·| is the pivot: rows 1 and 2 tie at 3 in column 0
        auto const a = from_real< T >( 3, 3, { 1, 2, 3, 3, 1, 1, -3, 4, 2 } );
        auto const f = feng::lu_factor( a );
        REQUIRE( f.pivots()[0] == 1 );
        // a zero column skips elimination and the factorization reports singular, rank 2
        auto const z = from_real< T >( 3, 3, { 0, 1, 2, 0, 3, 4, 0, 5, 7 } );
        auto const fz = feng::lu_factor( z );
        REQUIRE( fz.status() == feng::linalg_status::singular );
        REQUIRE( fz.rank() == 2 );
        REQUIRE( norm_inf< T >( permute_rows( z, fz.pivots() ) - fz.l() * fz.u() ) <= real_t< T >( 12 ) * eps< T >() * norm_inf( z ) );
        // [[1, 2, 3], [4, 5, 6], [7, 8, 9]] is singular
        auto const s = from_real< T >( 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } );
        REQUIRE( feng::lu_factor( s ).status() == feng::linalg_status::singular );
        REQUIRE( feng::lu_factor( s ).rank() == 2 );
        // nonfinite input
        auto nf = feng::eye< T >( 3, 3 );
        nf[1][2] = T( std::numeric_limits< real_t< T > >::quiet_NaN() );
        REQUIRE( feng::lu_factor( nf ).status() == feng::linalg_status::nonfinite );
        nf[1][2] = T( std::numeric_limits< real_t< T > >::infinity() );
        REQUIRE( feng::lu_factor( nf ).status() == feng::linalg_status::nonfinite );
        // 0×0 is a valid, ok factorization
        auto const e = feng::lu_factor( mat< T >{} );
        REQUIRE( e.status() == feng::linalg_status::ok );
        REQUIRE( e.rank() == 0 );
    }

    template< typename T >
    void check_solve()
    {
        real_t< T > worst{ 0 };
        for ( auto const& [name, a] : r09_inputs< T >() )
        {
            INFO( name );
            std::size_t const n = a.row();
            for ( std::size_t k : { std::size_t{ 1 }, std::size_t{ 3 } } )
            {
                std::mt19937_64 g{ 0x57B0ULL + n * 7 + k };
                mat< T > const b = feng::random< T >( n, k, g );
                auto const r = feng::lu_factor( a ).solve( b );
                REQUIRE( r.ok() );
                REQUIRE( static_cast< bool >( r ) );
                REQUIRE( r.status == feng::linalg_status::ok );
                REQUIRE( r.value.row() == n ); REQUIRE( r.value.col() == k );
                real_t< T > const ratio = solve_ratio( a, r.value, b );
                worst = ratio > worst ? ratio : worst;
                REQUIRE( ratio <= real_t< T >( 16 ) );
                auto const r2 = feng::solve( a, b );
                REQUIRE( r2.ok() );
                REQUIRE( norm_inf< T >( r2.value - r.value ) == real_t< T >( 0 ) );
            }
        }
        INFO( "worst solve residual ratio " << worst );
        // singular: status singular and an empty value
        auto const s = from_real< T >( 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } );
        auto const b = from_real< T >( 3, 1, { 1, 1, 1 } );
        auto const r = feng::solve( s, b );
        REQUIRE( !r.ok() );
        REQUIRE( !static_cast< bool >( r ) );
        REQUIRE( r.status == feng::linalg_status::singular );
        REQUIRE( r.value.size() == 0 );
    }

    template< typename T >
    void check_legacy()
    {
        for ( auto const& [name, a] : r09_inputs< T >() )
        {
            INFO( name );
            std::size_t const n = a.row();
            mat< T > l, u;
            REQUIRE( feng::lu_decomposition( a, l, u ) == 0 );
            // MATLAB's two-output form: L = Pᵀ·L₀ is a permuted unit lower triangle, A = L·U
            auto const f = feng::lu_factor( a );
            REQUIRE( norm_inf< T >( l - f.p().transpose() * f.l() ) == real_t< T >( 0 ) );
            REQUIRE( norm_inf< T >( u - f.u() ) == real_t< T >( 0 ) );
            REQUIRE( norm_inf< T >( a - l * u ) <= real_t< T >( 4 ) * real_t< T >( n ) * eps< T >() * norm_inf( a ) );
            auto const lu = feng::lu_decomposition( a );
            REQUIRE( lu.has_value() );
            REQUIRE( norm_inf< T >( std::get< 0 >( *lu ) - l ) == real_t< T >( 0 ) );
            REQUIRE( norm_inf< T >( std::get< 1 >( *lu ) - u ) == real_t< T >( 0 ) );

            std::mt19937_64 g{ 0x57C0ULL + n };
            mat< T > const b = feng::random< T >( n, 1, g );
            mat< T > x;
            REQUIRE( feng::lu_solver( a, x, b ) == 0 );
            REQUIRE( solve_ratio( a, x, b ) <= real_t< T >( 16 ) );
            auto const ox = feng::lu_solver( a, b );
            REQUIRE( ox.has_value() );
            REQUIRE( norm_inf< T >( *ox - x ) == real_t< T >( 0 ) );
        }
        // not ok: 1 / nullopt, outputs unchanged
        for ( auto const& s : { from_real< T >( 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } ),
                                from_real< T >( 2, 2, { 1, std::numeric_limits< double >::quiet_NaN(), 0, 1 } ) } )
        {
            mat< T > l{ 1, 1, T( 7 ) }, u{ 1, 2, T( 9 ) };
            REQUIRE( feng::lu_decomposition( s, l, u ) == 1 );
            REQUIRE( l.row() == 1 ); REQUIRE( l.col() == 1 ); REQUIRE( l[0][0] == T( 7 ) );
            REQUIRE( u.row() == 1 ); REQUIRE( u.col() == 2 ); REQUIRE( u[0][1] == T( 9 ) );
            REQUIRE( !feng::lu_decomposition( s ).has_value() );
            mat< T > const b{ s.row(), 1, T( 1 ) };
            mat< T > x{ 2, 2, T( 5 ) };
            REQUIRE( feng::lu_solver( s, x, b ) == 1 );
            REQUIRE( x.row() == 2 ); REQUIRE( x.col() == 2 ); REQUIRE( x[1][1] == T( 5 ) );
            REQUIRE( !feng::lu_solver( s, b ).has_value() );
        }
    }
}

TEST_CASE( "S7-R1 pivot choice, rank and status", "[S7][S7-R1]" )
{
    s7::check_pivot_rules< double >();
    s7::check_pivot_rules< std::complex< double > >();
    s7::check_pivot_rules< float >();
    s7::check_pivot_rules< std::complex< float > >();
}

TEST_CASE( "S7-R1 solve meets the residual bound c = 16", "[S7][S7-R1]" )
{
    s7::check_solve< double >();
    s7::check_solve< std::complex< double > >();
    s7::check_solve< float >();
    s7::check_solve< std::complex< float > >();
}

TEST_CASE( "S7-R1 legacy lu_decomposition and lu_solver rest on the LU object", "[S7][S7-R1]" )
{
    s7::check_legacy< double >();
    s7::check_legacy< std::complex< double > >();
    s7::check_legacy< float >();
    s7::check_legacy< std::complex< float > >();
}

namespace s7
{
    // the callable aborts (SIGABRT) with exactly one contract-violation line on stderr naming `expected`
    template< typename Callable >
    void require_one_message_death( Callable&& fn, std::string const& expected )
    {
        s2_death::outcome const out = s2_death::run( std::forward< Callable >( fn ) );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::line_count( out.err ) == 1 );
        REQUIRE( s2_death::contains( out.err, "contract violation" ) );
        REQUIRE( s2_death::contains( out.err, expected ) );
    }
}

TEST_CASE( "S7-R1 contract violations abort", "[S7][S7-R1]" )
{
    feng::matrix< double > const a{ 2, 3, 1.0 };
    feng::matrix< double > const sq = feng::eye< double >( 3, 3 );
    feng::matrix< double > const b{ 2, 1, 1.0 };
    s7::require_one_message_death( [&] { auto f = feng::lu_factor( a ); (void)f; }, "feng::lu_factor: expecting a square matrix" );
    s7::require_one_message_death( [&] { auto d = a.det(); (void)d; }, "matrix::det: expecting a square matrix" );
    s7::require_one_message_death( [&] { auto d = feng::det( a ); (void)d; }, "expecting a square matrix" );
    s7::require_one_message_death( [&] { auto x = a.inverse(); (void)x; }, "matrix::inverse: expecting a square matrix" );
    s7::require_one_message_death( [&] { auto x = feng::inverse( a ); (void)x; }, "expecting a square matrix" );
    s7::require_one_message_death( [&] { feng::matrix< double > out; auto st = feng::inverse( a, out ); (void)st; }, "feng::inverse: expecting a square matrix" );
    s7::require_one_message_death( [&] { auto r = feng::lu_factor( sq ).solve( b ); (void)r; }, "feng::lu_factorization::solve: B must have 3 rows" );
    s7::require_one_message_death( [&] { auto r = feng::solve( sq, b ); (void)r; }, "feng::solve: expecting a square A and B with as many rows" );
    std::complex< float > const one{ 1.0f, 0.0f };
    feng::matrix< std::complex< float > > const ca{ 3, 2, one };
    s7::require_one_message_death( [&] { auto f = feng::lu_factor( ca ); (void)f; }, "feng::lu_factor: expecting a square matrix" );
    s7::require_one_message_death( [&] { auto d = ca.det(); (void)d; }, "matrix::det: expecting a square matrix" );
    s7::require_one_message_death( [&] { auto x = ca.inverse(); (void)x; }, "matrix::inverse: expecting a square matrix" );
}
