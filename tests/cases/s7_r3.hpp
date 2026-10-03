// S7-R3 (PR-10, PR-2): one-sided Jacobi SVD for real and complex inputs of every shape, and one pseudoinverse
// with a relative cutoff behind pinverse, pinv and svd_inverse (F13, D-026, D-028). Uses the helpers of
// tests/cases/s7_r1.hpp (namespace s7). Set S7_RATIOS=1 to print the worst measured error/tolerance ratios.
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace s7r3
{
    using s7::mat;
    using s7::real_t;
    using s7::eps;

    template< typename T > constexpr bool is_cplx = !std::is_same_v< T, real_t< T > >;

    // worst error/tolerance ratio per check, printed by the last case when S7_RATIOS is set
    inline double& worst( int i )
    {
        static double w[8] = {};
        return w[i];
    }
    inline char const* const worst_name[8] = { "reconstruction", "U^H U = I", "V^H V = I", "singular values",
                                               "AXA = A", "XAX = X", "(AX)^H = AX", "(XA)^H = XA" };
    inline void track( int i, double err, double tol )
    {
        double const r = tol > 0 ? err / tol : ( err > 0 ? 1e300 : 0.0 );
        worst( i ) = std::max( worst( i ), r );
    }

    template< typename T >
    real_t< T > frob( mat< T > const& m )
    {
        real_t< T > s{ 0 };
        for ( auto const& x : m ) s += std::norm( x );
        return std::sqrt( s );
    }

    template< typename T >
    T cj( T const& x )
    {
        if constexpr ( is_cplx< T > ) return std::conj( x );
        else return x;
    }

    // conjugate transpose, written out here so the test does not lean on the library under test
    template< typename T >
    mat< T > ct( mat< T > const& m )
    {
        mat< T > r{ m.col(), m.row() };
        for ( std::size_t i = 0; i != m.row(); ++i )
            for ( std::size_t j = 0; j != m.col(); ++j ) r[j][i] = cj( m[i][j] );
        return r;
    }

    template< typename T >
    mat< T > eye( std::size_t n )
    {
        mat< T > r{ n, n };
        std::fill( r.begin(), r.end(), T( 0 ) );
        for ( std::size_t i = 0; i != n; ++i ) r[i][i] = T( 1 );
        return r;
    }

    // a unitary n×n matrix: a product of 3·n² seeded Givens rotations (with random phases for complex T)
    template< typename T >
    mat< T > givens_unitary( std::size_t n, std::uint_least64_t seed )
    {
        using R = real_t< T >;
        mat< T > q = eye< T >( n );
        if ( n < 2 ) return q;
        std::mt19937_64 g{ seed };
        std::size_t const count = 3 * n * n;
        auto const draws = feng::random< double >( count, 4, g );
        R const two_pi = R( 6.283185307179586476925286766559 );
        for ( std::size_t k = 0; k != count; ++k )
        {
            std::size_t const i = static_cast< std::size_t >( draws[k][0] * n ) % n;
            std::size_t j = static_cast< std::size_t >( draws[k][1] * ( n - 1 ) ) % ( n - 1 );
            if ( j >= i ) ++j;
            R const th = two_pi * R( draws[k][2] );
            R const c = std::cos( th ), s = std::sin( th );
            T ph = T( 1 );
            if constexpr ( is_cplx< T > ) ph = std::polar( R( 1 ), two_pi * R( draws[k][3] ) );
            for ( std::size_t r = 0; r != n; ++r ) // q ← q·G on columns i, j
            {
                T const x = q[r][i], y = q[r][j];
                q[r][i] = c * x - s * ph * y;
                q[r][j] = s * cj( ph ) * x + c * y;
            }
        }
        return q;
    }

    // Q1·Σ·Q2ᴴ with the chosen singular values on the diagonal of the m×n Σ
    template< typename T >
    mat< T > with_singular_values( std::size_t m, std::size_t n, std::vector< double > const& sigma, std::uint_least64_t seed )
    {
        mat< T > d{ m, n };
        std::fill( d.begin(), d.end(), T( 0 ) );
        for ( std::size_t i = 0; i != sigma.size(); ++i ) d[i][i] = T( real_t< T >( sigma[i] ) );
        return givens_unitary< T >( m, seed ) * d * ct( givens_unitary< T >( n, seed + 1 ) );
    }

    template< typename T >
    mat< T > seeded_random( std::size_t r, std::size_t c, std::uint_least64_t seed )
    {
        std::mt19937_64 g{ seed };
        return feng::random< T >( r, c, g );
    }

    template< typename T >
    struct fixture
    {
        std::string name;
        mat< T > a;
        std::vector< double > sigma; // the known singular values, descending; empty when not known
        std::size_t rank;
    };

    template< typename T >
    std::vector< fixture< T > > fixtures()
    {
        std::vector< fixture< T > > v;
        std::vector< double > const sq{ 4, 2.5, 1, 0.5, 0.125 };
        std::vector< double > const tw{ 3, 2, 1, 0.25 };
        v.push_back( { "square 5x5", with_singular_values< T >( 5, 5, sq, 0x5731ULL ), sq, 5 } );
        v.push_back( { "tall 7x4", with_singular_values< T >( 7, 4, tw, 0x5732ULL ), tw, 4 } );
        v.push_back( { "wide 4x7", with_singular_values< T >( 4, 7, tw, 0x5733ULL ), tw, 4 } );
        v.push_back( { "rank deficient 6x3 * 3x5", seeded_random< T >( 6, 3, 0x5734ULL ) * seeded_random< T >( 3, 5, 0x5735ULL ), {}, 3 } );
        // diagonal with signs (and a phase for complex): singular values are the sorted |d_ii|
        mat< T > d{ 4, 4 };
        std::fill( d.begin(), d.end(), T( 0 ) );
        d[0][0] = T( -2 ); d[1][1] = T( 5 ); d[2][2] = T( 0.5 ); d[3][3] = T( 3 );
        if constexpr ( is_cplx< T > ) d[2][2] = T( 0.3, -0.4 );
        v.push_back( { "diagonal 4x4", d, { 5, 3, 2, 0.5 }, 4 } );
        // an exact zero column and an exact zero singular value
        mat< T > z{ 3, 3 };
        std::fill( z.begin(), z.end(), T( 0 ) );
        z[0][0] = T( 3 ); z[2][2] = T( 1 );
        v.push_back( { "zero column 3x3", z, { 3, 1, 0 }, 2 } );
        v.push_back( { "rank deficient wide 3x6", with_singular_values< T >( 3, 6, { 2, 1, 0 }, 0x5736ULL ), {}, 2 } );
        return v;
    }

    // the fixtures at T's precision: float and complex<float> inputs are built in double and rounded once, so a
    // rank-deficient input carries only the ε/2 rounding noise, well under the p·ε·s_1 cutoff (D-028), instead of
    // the Givens-product and matrix-product noise accumulated in float
    template< typename T >
    std::vector< fixture< T > > fixtures_at()
    {
        if constexpr ( sizeof( real_t< T > ) >= sizeof( double ) ) return fixtures< T >();
        else
        {
            using W = std::conditional_t< is_cplx< T >, std::complex< double >, double >;
            std::vector< fixture< T > > v;
            for ( auto const& fx : fixtures< W >() ) v.push_back( { fx.name, fx.a.template astype< T >(), fx.sigma, fx.rank } );
            return v;
        }
    }

    template< typename T >
    void check_factorization( fixture< T > const& fx )
    {
        using R = real_t< T >;
        INFO( fx.name );
        mat< T > const& a = fx.a;
        std::size_t const m = a.row(), n = a.col(), k = std::min( m, n ), p = std::max( m, n );
        auto const f = feng::svd_factor( a );
        REQUIRE( f.status() == feng::linalg_status::ok );
        REQUIRE( f.sweeps() >= 1 );
        REQUIRE( f.u().row() == m ); REQUIRE( f.u().col() == k );
        REQUIRE( f.v().row() == n ); REQUIRE( f.v().col() == k );
        auto const& s = f.s();
        REQUIRE( s.size() == k );
        for ( std::size_t i = 0; i != k; ++i )
        {
            REQUIRE( s[i] >= R( 0 ) );
            if ( i ) REQUIRE( s[i] <= s[i - 1] );
        }
        mat< T > sd{ k, k };
        std::fill( sd.begin(), sd.end(), T( 0 ) );
        for ( std::size_t i = 0; i != k; ++i ) sd[i][i] = T( s[i] );
        R const pe = R( p ) * eps< T >();
        R const rec = frob< T >( a - f.u() * sd * ct( f.v() ) );
        R const rec_tol = R( 8 ) * pe * frob( a );
        track( 0, rec, rec_tol );
        REQUIRE( rec <= rec_tol );
        R const ou = frob< T >( ct( f.u() ) * f.u() - eye< T >( k ) );
        R const ov = frob< T >( ct( f.v() ) * f.v() - eye< T >( k ) );
        track( 1, ou, R( 8 ) * pe );
        track( 2, ov, R( 8 ) * pe );
        REQUIRE( ou <= R( 8 ) * pe );
        REQUIRE( ov <= R( 8 ) * pe );
        if ( !fx.sigma.empty() )
            for ( std::size_t i = 0; i != k; ++i )
            {
                R const err = std::abs( s[i] - R( fx.sigma[i] ) );
                R const tol = R( 8 ) * pe * R( fx.sigma[0] );
                track( 3, err, tol );
                REQUIRE( err <= tol );
            }
        REQUIRE( f.rank() == fx.rank );
    }

    // the four Moore–Penrose identities with tolerance 32·p·ε·κ·scale, κ = s_1/s_r over the kept values
    template< typename T >
    void check_moore_penrose( mat< T > const& a, mat< T > const& x, std::vector< real_t< T > > const& s, std::size_t r )
    {
        using R = real_t< T >;
        std::size_t const p = std::max( a.row(), a.col() );
        R const kappa = r ? s[0] / s[r - 1] : R( 1 );
        R const tol = R( 32 ) * R( p ) * eps< T >() * kappa;
        mat< T > const ax = a * x, xa = x * a;
        R const e1 = frob< T >( ax * a - a ), t1 = tol * frob( a );
        R const e2 = frob< T >( xa * x - x ), t2 = tol * frob( x );
        R const e3 = frob< T >( ct( ax ) - ax );
        R const e4 = frob< T >( ct( xa ) - xa );
        track( 4, e1, t1 ); track( 5, e2, t2 ); track( 6, e3, tol ); track( 7, e4, tol );
        REQUIRE( e1 <= t1 );
        REQUIRE( e2 <= t2 );
        REQUIRE( e3 <= tol );
        REQUIRE( e4 <= tol );
    }

    template< typename T >
    bool same( mat< T > const& x, mat< T > const& y )
    {
        return x.row() == y.row() && x.col() == y.col() && std::equal( x.begin(), x.end(), y.begin() );
    }

    template< typename T >
    void check_pinverse()
    {
        for ( auto const& fx : fixtures_at< T >() )
        {
            INFO( fx.name );
            mat< T > const& a = fx.a;
            mat< T > x;
            REQUIRE( feng::pinverse( a, x ) == feng::linalg_status::ok );
            REQUIRE( x.row() == a.col() ); REQUIRE( x.col() == a.row() );
            auto const f = feng::svd_factor( a );
            std::size_t const r = f.rank();
            REQUIRE( r == fx.rank );
            check_moore_penrose( a, x, f.s(), r );
            // one implementation behind every spelling
            REQUIRE( same( feng::pinverse( a ), x ) );
            REQUIRE( same( feng::pinv( a ), x ) );
            REQUIRE( same( feng::svd_inverse( a ), x ) );
            REQUIRE( same( f.pinverse().value, x ) );
        }
    }

    template< typename T >
    void check_cutoff()
    {
        using R = real_t< T >;
        // diagonal: 1e-20 ≤ 3ε·1 is zeroed, the rest inverted
        mat< T > d{ 3, 3 };
        std::fill( d.begin(), d.end(), T( 0 ) );
        d[0][0] = T( 1 ); d[1][1] = T( 0.5 ); d[2][2] = T( R( 1e-20 ) );
        mat< T > x;
        REQUIRE( feng::pinverse( d, x ) == feng::linalg_status::ok );
        mat< T > want{ 3, 3 };
        std::fill( want.begin(), want.end(), T( 0 ) );
        want[0][0] = T( 1 ); want[1][1] = T( 2 );
        REQUIRE( frob< T >( x - want ) <= R( 4 ) * eps< T >() );
        REQUIRE( feng::svd_factor( d ).rank() == 2 );
        // rtol = 0 keeps it: the cutoff is the relative rule, nothing else
        REQUIRE( feng::pinverse( d, x, R( 0 ) ) == feng::linalg_status::ok );
        REQUIRE( std::abs( x[2][2] - T( R( 1e20 ) ) ) <= R( 4 ) * eps< T >() * R( 1e20 ) );
        // a rotated 5×3 with singular values 1, 0.5, 1e-20
        auto const a = with_singular_values< T >( 5, 3, { 1, 0.5, 1e-20 }, 0x5737ULL );
        auto const f = feng::svd_factor( a );
        REQUIRE( f.ok() );
        REQUIRE( f.rank() == 2 );
        REQUIRE( feng::pinverse( a, x ) == feng::linalg_status::ok );
        check_moore_penrose( a, x, f.s(), 2 );
        REQUIRE( std::abs( frob( x ) - std::sqrt( R( 5 ) ) ) <= R( 32 ) * R( 5 ) * eps< T >() * R( 2 ) * std::sqrt( R( 5 ) ) );
    }

    template< typename T >
    void check_legacy_shapes()
    {
        using R = real_t< T >;
        for ( auto [m, n] : { std::pair< std::size_t, std::size_t >{ 7, 4 }, { 4, 7 }, { 5, 5 } } )
        {
            INFO( m << "x" << n );
            std::size_t const k = std::min( m, n );
            auto const a = seeded_random< T >( m, n, 0x5738ULL + m );
            mat< T > u, w, v;
            REQUIRE( feng::singular_value_decomposition( a, u, w, v ) == 0 );
            REQUIRE( u.row() == m ); REQUIRE( u.col() == k );
            REQUIRE( w.row() == k ); REQUIRE( w.col() == k );
            REQUIRE( v.row() == n ); REQUIRE( v.col() == k );
            for ( std::size_t i = 0; i != k; ++i )
                for ( std::size_t j = 0; j != k; ++j )
                    if ( i != j ) REQUIRE( w[i][j] == T( 0 ) );
            REQUIRE( frob< T >( a - u * w * ct( v ) ) <= R( 8 ) * R( std::max( m, n ) ) * eps< T >() * frob( a ) );
            auto const t = feng::singular_value_decomposition( a );
            REQUIRE( t.has_value() );
            REQUIRE( same( std::get< 0 >( *t ), u ) );
            REQUIRE( same( std::get< 1 >( *t ), w ) );
            REQUIRE( same( std::get< 2 >( *t ), v ) );
            auto const t2 = feng::svd( a );
            REQUIRE( t2.has_value() );
            REQUIRE( same( std::get< 1 >( *t2 ), w ) );
        }
    }

    template< typename T >
    void check_empty()
    {
        for ( auto [m, n] : { std::pair< std::size_t, std::size_t >{ 0, 3 }, { 3, 0 }, { 0, 0 } } )
        {
            INFO( m << "x" << n );
            mat< T > const a{ m, n };
            auto const f = feng::svd_factor( a );
            REQUIRE( f.status() == feng::linalg_status::ok );
            REQUIRE( f.s().empty() );
            REQUIRE( f.u().row() == m ); REQUIRE( f.u().col() == 0 );
            REQUIRE( f.v().row() == n ); REQUIRE( f.v().col() == 0 );
            REQUIRE( f.rank() == 0 );
            mat< T > x;
            REQUIRE( feng::pinverse( a, x ) == feng::linalg_status::ok );
            REQUIRE( x.row() == n ); REQUIRE( x.col() == m );
            mat< T > u, w, v;
            REQUIRE( feng::singular_value_decomposition( a, u, w, v ) == 0 );
            REQUIRE( u.row() == m ); REQUIRE( w.row() == 0 ); REQUIRE( v.row() == n );
        }
    }

    template< typename T >
    void check_nonconvergence()
    {
        auto const a = seeded_random< T >( 8, 8, 0x5739ULL );
        auto const f = feng::svd_factor( a, 1 );
        REQUIRE( f.status() == feng::linalg_status::not_converged );
        REQUIRE( f.sweeps() == 1 );
        auto const r = f.pinverse();
        REQUIRE( r.status == feng::linalg_status::not_converged );
        REQUIRE( r.value.size() == 0 );
        // legacy: 1 and the outputs untouched
        mat< T > u{ 2, 2, T( 7 ) }, w{ 1, 3, T( 8 ) }, v{ 3, 1, T( 9 ) };
        mat< T > const u0 = u, w0 = w, v0 = v;
        REQUIRE( feng::singular_value_decomposition( a, u, w, v, 1 ) == 1 );
        REQUIRE( same( u, u0 ) ); REQUIRE( same( w, w0 ) ); REQUIRE( same( v, v0 ) );
        // the default sweep limit converges
        auto const g = feng::svd_factor( a );
        REQUIRE( g.ok() );
        REQUIRE( g.sweeps() > 1 );
        // a nonfinite input: status, never NaN in an output
        mat< T > bad = a;
        bad[3][4] = T( std::numeric_limits< real_t< T > >::quiet_NaN() );
        REQUIRE( feng::svd_factor( bad ).status() == feng::linalg_status::nonfinite );
        mat< T > x{ 2, 2, T( 1 ) };
        mat< T > const x0 = x;
        REQUIRE( feng::pinverse( bad, x ) == feng::linalg_status::nonfinite );
        REQUIRE( same( x, x0 ) );
        REQUIRE( feng::pinv( bad ).size() == 0 );
        REQUIRE( feng::svd_inverse( bad ).size() == 0 );
        REQUIRE( !feng::svd( bad ).has_value() );
        REQUIRE( feng::singular_value_decomposition( bad, u, w, v ) == 1 );
        REQUIRE( same( u, u0 ) );
    }
}

TEST_CASE( "S7-R3 svd factors reconstruct with orthonormal U and V and the known singular values", "[S7][S7-R3]" )
{
    for ( auto const& fx : s7r3::fixtures< double >() ) s7r3::check_factorization( fx );
    for ( auto const& fx : s7r3::fixtures< std::complex< double > >() ) s7r3::check_factorization( fx );
}

TEST_CASE( "S7-R3 svd of tall, wide and rank-deficient inputs", "[S7][S7-R3]" )
{
    // square, tall, wide, diagonal, zero-column and rank-deficient (tall product and wide) inputs in every element
    // type; ε is that of the element's real type, ᴴ the conjugate transpose
    auto const run = []< typename T >( char const* name, T* )
    {
        double saved[8];
        for ( int i = 0; i != 8; ++i ) { saved[i] = s7r3::worst( i ); s7r3::worst( i ) = 0; }
        for ( auto const& fx : s7r3::fixtures_at< T >() ) s7r3::check_factorization( fx );
        s7r3::check_pinverse< T >();
        for ( int i = 0; i != 8; ++i )
        {
            if ( std::getenv( "S7_RATIOS" ) ) std::printf( "S7-R3 %s worst %s: %.3g of tolerance\n", name, s7r3::worst_name[i], s7r3::worst( i ) );
            s7r3::worst( i ) = std::max( saved[i], s7r3::worst( i ) );
        }
    };
    run( "float", static_cast< float* >( nullptr ) );
    run( "double", static_cast< double* >( nullptr ) );
    run( "complex<float>", static_cast< std::complex< float >* >( nullptr ) );
    run( "complex<double>", static_cast< std::complex< double >* >( nullptr ) );
}

TEST_CASE( "S7-R3 pseudoinverse meets the Moore-Penrose identities", "[S7][S7-R3]" )
{
    s7r3::check_pinverse< double >();
    s7r3::check_pinverse< std::complex< double > >();
}

TEST_CASE( "S7-R3 pseudoinverse zeroes singular values below the relative cutoff", "[S7][S7-R3]" )
{
    s7r3::check_cutoff< double >();
    s7r3::check_cutoff< std::complex< double > >();
}

TEST_CASE( "S7-R3 legacy singular_value_decomposition returns thin u, diagonal w and v", "[S7][S7-R3]" )
{
    s7r3::check_legacy_shapes< double >();
    s7r3::check_legacy_shapes< std::complex< double > >();
}

TEST_CASE( "S7-R3 svd and pinverse accept 0xn and mx0 inputs", "[S7][S7-R3]" )
{
    s7r3::check_empty< double >();
    s7r3::check_empty< std::complex< double > >();
}

TEST_CASE( "S7-R3 svd reports nonconvergence", "[S7][S7-R3]" )
{
    s7r3::check_nonconvergence< double >();
    s7r3::check_nonconvergence< std::complex< double > >();
    if ( std::getenv( "S7_RATIOS" ) )
        for ( int i = 0; i != 8; ++i ) std::printf( "S7-R3 worst %s: %.3g of tolerance\n", s7r3::worst_name[i], s7r3::worst( i ) );
}
