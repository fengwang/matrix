// S7-R4 (PR-10, PR-2): Cholesky with a status and RREF with independent pivot rows and columns (F13, F15).
// Uses the helpers of tests/cases/s7_r1.hpp (namespace s7). The exact RREF references below were derived with
// python3 fractions.Fraction (Gaussian rationals for the complex fixtures) and pasted as literals.
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

namespace s7r4
{
    using s7::mat;
    using s7::real_t;
    using cd = std::complex< double >;

    template< typename T >
    mat< T > from_list( std::size_t r, std::size_t c, std::vector< T > const& v )
    {
        mat< T > m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i ) m[i / c][i % c] = v[i];
        return m;
    }

    template< typename T >
    T conj_( T const& x )
    {
        if constexpr ( std::is_same_v< T, real_t< T > > ) return x;
        else return std::conj( x );
    }

    template< typename T >
    real_t< T > frob( mat< T > const& m )
    {
        real_t< T > s{ 0 };
        for ( auto const& x : m ) s += std::norm( x );
        return std::sqrt( s );
    }

    template< typename T >
    bool any_nan( mat< T > const& m )
    {
        for ( auto const& x : m )
            if ( std::isnan( std::real( x ) ) || std::isnan( std::imag( x ) ) ) return true;
        return false;
    }

    struct rref_case
    {
        std::size_t m, n;
        std::vector< double > a, r;
        std::vector< std::size_t > pivots;
    };

    inline std::vector< rref_case > real_cases()
    {
        return {
            // square, full rank
            { 3, 3, { 2, 1, 1, 1, 3, 2, 1, 0, 0 }, { 1, 0, 0, 0, 1, 0, 0, 0, 1 }, { 0, 1, 2 } },
            // square, rank 2
            { 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 1, 0, -1, 0, 1, 2, 0, 0, 0 }, { 0, 1 } },
            // square 4×4, column 1 = 2·column 0
            { 4, 4, { 2, 4, 1, 3, 1, 2, 0, 1, 3, 6, 1, 4, 0, 0, 2, 5 }, { 1, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 }, { 0, 2, 3 } },
            // tall, column 1 = 2·column 0
            { 4, 3, { 1, 2, 1, 2, 4, 0, 3, 6, 1, 1, 2, 2 }, { 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 }, { 0, 2 } },
            // tall, full column rank
            { 3, 2, { 1, 2, 3, 4, 5, 7 }, { 1, 0, 0, 1, 0, 0 }, { 0, 1 } },
            // wide with a zero column 0 and column 2 = 2·column 1, rank 2
            { 3, 5, { 0, 1, 2, 1, 3, 0, 2, 4, 1, 1, 0, 3, 6, 2, 4 }, { 0, 1, 2, 0, -2, 0, 0, 0, 1, 5, 0, 0, 0, 0, 0 }, { 1, 3 } },
            // wide, column 2 = 1.6·column 0 − 0.2·column 1
            { 3, 4, { 2, 1, 3, 1, 1, 3, 1, 2, 3, 4, 4, 5 }, { 1, 0, 1.6, 0, 0, 1, -0.2, 0, 0, 0, 0, 1 }, { 0, 1, 3 } },
            // wide, full row rank
            { 2, 3, { 1, 2, 3, 4, 5, 6 }, { 1, 0, -1, 0, 1, 2 }, { 0, 1 } },
        };
    }

    template< typename T >
    void check_rref( mat< T > const& a, mat< T > const& ref, std::vector< std::size_t > const& pivots )
    {
        auto const [m, n] = a.shape();
        real_t< T > const p = static_cast< real_t< T > >( m > n ? m : n );
        real_t< T > const tol = 64 * p * s7::eps< T >() * ( real_t< T >( 1 ) + s7::norm_inf( a ) );
        auto const res = feng::row_echelon( a );
        REQUIRE( res.status == feng::linalg_status::ok );
        REQUIRE( res.rank == pivots.size() );
        REQUIRE( res.pivot_columns == pivots );
        REQUIRE( res.r.row() == m );
        REQUIRE( res.r.col() == n );
        for ( std::size_t i = 0; i != m; ++i )
            for ( std::size_t j = 0; j != n; ++j )
                CHECK( std::abs( res.r[i][j] - ref[i][j] ) <= tol );
        // pivot entries exactly 1, every other entry of a pivot column exactly 0, rows past the rank exactly 0
        for ( std::size_t k = 0; k != pivots.size(); ++k )
            for ( std::size_t i = 0; i != m; ++i )
                CHECK( res.r[i][pivots[k]] == ( i == k ? T( 1 ) : T( 0 ) ) );
        for ( std::size_t i = pivots.size(); i != m; ++i )
            for ( std::size_t j = 0; j != n; ++j )
                CHECK( res.r[i][j] == T( 0 ) );
        // the legacy spellings forward to row_echelon for every shape
        auto const r1 = feng::rref( a );
        auto const r2 = feng::gauss_jordan_elimination( a );
        REQUIRE( r1.has_value() );
        REQUIRE( r2.has_value() );
        CHECK( *r1 == res.r );
        CHECK( *r2 == res.r );
    }

    template< typename T >
    void check_rref_real_cases()
    {
        for ( auto const& c : real_cases() )
            check_rref< T >( s7::from_real< T >( c.m, c.n, c.a ), s7::from_real< T >( c.m, c.n, c.r ), c.pivots );
    }
} // namespace s7r4

namespace s7r4
{
    // the complex fixtures as complex<double> literals, rounded to T (every entry is exact in float)
    template< typename T >
    mat< T > cplx( std::size_t r, std::size_t c, std::vector< cd > const& v )
    {
        return from_list< cd >( r, c, v ).template astype< T >();
    }

    template< typename T >
    void check_rref_complex_cases()
    {
        // complex 3×3 of rank 2: row 2 = row 0 + row 1
        check_rref< T >( cplx< T >( 3, 3, { { 1, 1 }, { 2, 0 }, { 0, 1 }, { 0, 2 }, { 2, -2 }, { 1, 0 }, { 1, 3 }, { 4, -2 }, { 1, 1 } } ),
                         cplx< T >( 3, 3, { 1, 0, { -0.25, 0.25 }, 0, 1, { 0.25, 0.5 }, 0, 0, 0 } ), { 0, 1 } );
        // complex wide with a zero column and a dependent column
        check_rref< T >( cplx< T >( 2, 4, { 0, { 1, 1 }, { 2, 0 }, { 3, -1 }, 0, { 2, 2 }, { 4, 0 }, { 1, 0 } } ),
                         cplx< T >( 2, 4, { 0, 1, { 1, -1 }, 0, 0, 0, 0, 1 } ), { 1, 3 } );
    }

    template< typename T >
    void check_rref_degenerate()
    {
        // the zero matrix: rank 0, unchanged; a 0×0 input
        mat< T > const z{ 2, 3, T( 0 ) };
        auto const rz = feng::row_echelon( z );
        CHECK( rz.rank == 0 );
        CHECK( rz.pivot_columns.empty() );
        CHECK( rz.r == z );
        CHECK( feng::row_echelon( mat< T >{} ).rank == 0 );

        // nonfinite input: nullopt from the legacy spellings, status nonfinite from row_echelon
        mat< T > bad = s7::from_real< T >( 2, 2, { 1, 2, 3, 4 } );
        bad[1][0] = T( std::numeric_limits< real_t< T > >::quiet_NaN() );
        CHECK( feng::row_echelon( bad ).status == feng::linalg_status::nonfinite );
        CHECK_FALSE( feng::rref( bad ).has_value() );
        CHECK_FALSE( feng::gauss_jordan_elimination( bad ).has_value() );
    }
} // namespace s7r4

TEST_CASE( "S7-R4 rref matches exact references on every shape", "[S7][S7-R4]" )
{
    // square, tall, wide and rank-deficient fixtures in float, double, complex<float> and complex<double>
    s7r4::check_rref_real_cases< float >();
    s7r4::check_rref_real_cases< double >();
    s7r4::check_rref_real_cases< std::complex< float > >();
    s7r4::check_rref_real_cases< std::complex< double > >();
    s7r4::check_rref_complex_cases< std::complex< float > >();
    s7r4::check_rref_complex_cases< std::complex< double > >();
    s7r4::check_rref_degenerate< float >();
    s7r4::check_rref_degenerate< double >();
    s7r4::check_rref_degenerate< std::complex< float > >();
    s7r4::check_rref_degenerate< std::complex< double > >();
}

namespace s7r4
{
    template< typename T >
    void check_cholesky_spd( mat< T > const& a )
    {
        std::size_t const n = a.row();
        auto const f = feng::cholesky_factor( a );
        REQUIRE( f.status() == feng::linalg_status::ok );
        REQUIRE( f.ok() );
        mat< T > const& l = f.l();
        REQUIRE( l.row() == n );
        REQUIRE( l.col() == n );
        mat< T > llh{ n, n };
        for ( std::size_t i = 0; i != n; ++i )
        {
            CHECK( std::imag( l[i][i] ) == 0 );
            CHECK( std::real( l[i][i] ) > 0 );
            for ( std::size_t j = i + 1; j != n; ++j ) CHECK( l[i][j] == T( 0 ) );
            for ( std::size_t j = 0; j != n; ++j )
            {
                T s{ 0 };
                for ( std::size_t k = 0; k != n; ++k ) s += l[i][k] * conj_( l[j][k] );
                llh[i][j] = s;
            }
        }
        mat< T > d{ n, n };
        for ( std::size_t i = 0; i != n; ++i )
            for ( std::size_t j = 0; j != n; ++j ) d[i][j] = llh[i][j] - a[i][j];
        CHECK( frob( d ) <= 8 * real_t< T >( n ) * s7::eps< T >() * frob( a ) );

        mat< T > out;
        CHECK( feng::cholesky_decomposition( a, out ) == 0 );
        CHECK( out == l );
    }

    template< typename T >
    void check_cholesky_fails( mat< T > const& a )
    {
        auto const f = feng::cholesky_factor( a );
        CHECK( f.status() == feng::linalg_status::not_positive_definite );
        CHECK_FALSE( f.ok() );
        CHECK_FALSE( any_nan( f.l() ) );
        mat< T > out = s7::from_real< T >( 1, 2, { 7, 8 } );
        mat< T > const keep = out;
        CHECK( feng::cholesky_decomposition( a, out ) == 1 );
        CHECK( out == keep );
    }

    template< typename T >
    void check_cholesky_real_cases()
    {
        check_cholesky_spd< T >( s7::from_real< T >( 3, 3, { 4, 12, -16, 12, 37, -43, -16, -43, 98 } ) );
        check_cholesky_spd< T >( s7::from_real< T >( 1, 1, { 2 } ) );
        // random SPD: Bᵀ·B + n·I
        mat< T > const b = s7::random_square< T >( 6, 77 );
        mat< T > spd{ 6, 6 };
        for ( std::size_t i = 0; i != 6; ++i )
            for ( std::size_t j = 0; j != 6; ++j )
            {
                T s = i == j ? T( 6 ) : T( 0 );
                for ( std::size_t k = 0; k != 6; ++k ) s += conj_( b[k][i] ) * b[k][j];
                spd[i][j] = s;
            }
        check_cholesky_spd< T >( spd );

        check_cholesky_fails< T >( s7::from_real< T >( 2, 2, { 2, 1, 0, 2 } ) );                      // not symmetric
        check_cholesky_fails< T >( s7::from_real< T >( 2, 2, { 1, 2, 2, 1 } ) );                      // indefinite
        check_cholesky_fails< T >( s7::from_real< T >( 2, 2, { -1, 0, 0, -1 } ) );                    // negative definite
        check_cholesky_fails< T >( s7::from_real< T >( 2, 2, { 1, 1, 1, 1 } ) );                      // singular
        check_cholesky_fails< T >( s7::from_real< T >( 3, 3, { 1, 2, 3, 2, 4, 6, 3, 6, 9 } ) );       // singular, rank 1
        mat< T > nan_a = s7::from_real< T >( 2, 2, { 4, 1, 1, 3 } );
        nan_a[1][1] = T( std::numeric_limits< real_t< T > >::quiet_NaN() );
        check_cholesky_fails< T >( nan_a );                                                           // nonfinite
        mat< T > inf_a = s7::from_real< T >( 2, 2, { 4, 1, 1, 3 } );
        inf_a[0][0] = T( std::numeric_limits< real_t< T > >::infinity() );
        check_cholesky_fails< T >( inf_a );
    }
} // namespace s7r4

namespace s7r4
{
    template< typename T >
    void check_cholesky_complex_cases()
    {
        // Hermitian positive definite
        check_cholesky_spd< T >( cplx< T >( 3, 3, { 4, { 1, 2 }, { 0, -1 }, { 1, -2 }, 10, { 2, 1 }, { 0, 1 }, { 2, -1 }, 6 } ) );
        // complex symmetric but not Hermitian, and a diagonal with an imaginary part
        check_cholesky_fails< T >( cplx< T >( 2, 2, { 4, { 1, 2 }, { 1, 2 }, 5 } ) );
        check_cholesky_fails< T >( cplx< T >( 2, 2, { { 4, 1 }, 0, 0, 5 } ) );
    }
} // namespace s7r4

TEST_CASE( "S7-R4 cholesky factors SPD and HPD inputs", "[S7][S7-R4]" )
{
    s7r4::check_cholesky_real_cases< float >();
    s7r4::check_cholesky_real_cases< double >();
    s7r4::check_cholesky_real_cases< std::complex< float > >();
    s7r4::check_cholesky_real_cases< std::complex< double > >();
    s7r4::check_cholesky_complex_cases< std::complex< float > >();
    s7r4::check_cholesky_complex_cases< std::complex< double > >();
    // 0×0 is ok and empty
    auto const f0 = feng::cholesky_factor( feng::matrix< double >{} );
    CHECK( f0.ok() );
    CHECK( f0.l().size() == 0 );
    auto const f0f = feng::cholesky_factor( feng::matrix< float >{} );
    CHECK( f0f.ok() );
    CHECK( f0f.l().size() == 0 );
}

namespace s7r4
{
    template< typename T >
    void check_cholesky_failure_classes()
    {
        // the four failure classes of S7-R4, each with no NaN in l() and the legacy destination unchanged
        check_cholesky_fails< T >( s7::from_real< T >( 3, 3, { 2, 1, 0, 0, 2, 1, 0, 0, 2 } ) );
        check_cholesky_fails< T >( s7::from_real< T >( 3, 3, { 1, 0, 0, 0, -1, 0, 0, 0, 1 } ) );
        check_cholesky_fails< T >( s7::from_real< T >( 3, 3, { 1, 1, 0, 1, 1, 0, 0, 0, 1 } ) );
        mat< T > a = feng::eye< T >( 3, 3 );
        a[2][1] = a[1][2] = T( std::numeric_limits< real_t< T > >::infinity() );
        check_cholesky_fails< T >( a );
    }
} // namespace s7r4

TEST_CASE( "S7-R4 cholesky reports a non-SPD input", "[S7][S7-R4]" )
{
    s7r4::check_cholesky_failure_classes< float >();
    s7r4::check_cholesky_failure_classes< double >();
    s7r4::check_cholesky_failure_classes< std::complex< float > >();
    s7r4::check_cholesky_failure_classes< std::complex< double > >();
}
