// S7-R6 (PR-10, PR-14): compares the S7 linear algebra against Eigen 3 on seeded random inputs (double and
// complex<double>): det (PartialPivLU), solve, inverse, singular values (JacobiSVD), pinv (JacobiSVD with
// feng's cutoff max(m, n)·ε·s_1) and Cholesky (LLT). Built and run by `tools/check.sh oracle S7`
// (-std=c++20 -O2 -isystem /usr/include/eigen3). Prints the Eigen version, one line per failing comparison and
// `EIGEN PASS <n> comparisons`; exit status 0 only when every comparison passes.
#include "../../matrix.hpp"

#include <Eigen/Cholesky>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/SVD>

#include <cmath>
#include <complex>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>

namespace
{
    template< typename T > struct real_of { using type = T; };
    template< typename T > struct real_of< std::complex< T > > { using type = T; };
    template< typename T > using real_t = typename real_of< T >::type;
    template< typename T > using emat = Eigen::Matrix< T, Eigen::Dynamic, Eigen::Dynamic >;

    double const eps = std::numeric_limits< double >::epsilon();
    std::size_t passed = 0, failed = 0;

    void check( bool ok, std::string const& what, double err, double tol )
    {
        if ( std::getenv( "S7_RATIOS" ) ) std::cout << what << " ratio " << err / tol << "\n";
        if ( ok ) { ++passed; return; }
        ++failed;
        std::cout << "EIGEN FAIL " << what << ": error " << err << " > tol " << tol << "\n";
    }

    template< typename T >
    T conj_( T const& x )
    {
        if constexpr ( std::is_same_v< T, double > ) return x;
        else return std::conj( x );
    }

    template< typename T >
    T draw( std::mt19937_64& g )
    {
        std::uniform_real_distribution< double > u( -1.0, 1.0 );
        if constexpr ( std::is_same_v< T, double > ) return u( g );
        else { double const re = u( g ); return T( re, u( g ) ); }
    }

    template< typename T >
    feng::matrix< T > random( std::size_t m, std::size_t n, std::mt19937_64& g )
    {
        feng::matrix< T > a{ m, n };
        for ( std::size_t r = 0; r != m; ++r )
            for ( std::size_t c = 0; c != n; ++c ) a[r][c] = draw< T >( g );
        return a;
    }

    template< typename T >
    emat< T > to_eigen( feng::matrix< T > const& a )
    {
        emat< T > e( a.row(), a.col() );
        for ( std::size_t r = 0; r != a.row(); ++r )
            for ( std::size_t c = 0; c != a.col(); ++c ) e( r, c ) = a[r][c];
        return e;
    }

    // ||f - e||_F / ||e||_F; +inf on a shape mismatch
    template< typename T >
    double rel( feng::matrix< T > const& f, emat< T > const& e )
    {
        if ( f.row() != std::size_t( e.rows() ) || f.col() != std::size_t( e.cols() ) ) return HUGE_VAL;
        double d = 0;
        for ( std::size_t r = 0; r != f.row(); ++r )
            for ( std::size_t c = 0; c != f.col(); ++c ) d += std::norm( f[r][c] - e( r, c ) );
        return std::sqrt( d ) / e.norm();
    }

    template< typename T >
    emat< T > eigen_pinv( emat< T > const& a )
    {
        Eigen::JacobiSVD< emat< T >, Eigen::ComputeThinU | Eigen::ComputeThinV > svd( a );
        auto const& s = svd.singularValues();
        double const cut = double( std::max( a.rows(), a.cols() ) ) * eps * ( s.size() ? s( 0 ) : 0.0 );
        Eigen::Matrix< T, Eigen::Dynamic, 1 > inv_s( s.size() );
        for ( Eigen::Index i = 0; i != s.size(); ++i ) inv_s( i ) = s( i ) > cut ? T( 1.0 / s( i ) ) : T( 0 );
        return svd.matrixV() * inv_s.asDiagonal() * svd.matrixU().adjoint();
    }

    // κ over the singular values above the pinv cutoff
    template< typename T >
    double kappa( emat< T > const& a )
    {
        Eigen::JacobiSVD< emat< T > > svd( a );
        auto const& s = svd.singularValues();
        double const cut = double( std::max( a.rows(), a.cols() ) ) * eps * s( 0 );
        double lo = s( 0 );
        for ( Eigen::Index i = 0; i != s.size(); ++i ) if ( s( i ) > cut ) lo = s( i );
        return s( 0 ) / lo;
    }

    template< typename T >
    void svd_and_pinv( feng::matrix< T > const& a, std::string const& tag )
    {
        std::size_t const p = std::max( a.row(), a.col() );
        emat< T > const e = to_eigen( a );
        Eigen::JacobiSVD< emat< T > > esvd( e );
        auto const& es = esvd.singularValues();
        auto const f = feng::svd_factor( a );
        double err = HUGE_VAL;
        if ( f.ok() && f.s().size() == std::size_t( es.size() ) )
        {
            err = 0;
            for ( std::size_t i = 0; i != f.s().size(); ++i ) err = std::max( err, std::abs( f.s()[i] - es( i ) ) );
        }
        double tol = 32.0 * p * eps * es( 0 );
        check( err <= tol, "svdvals " + tag, err, tol );

        double const k = kappa( e );
        feng::matrix< T > x;
        err = feng::pinverse( a, x ) == feng::linalg_status::ok ? rel( x, eigen_pinv( e ) ) : HUGE_VAL;
        tol = 64.0 * p * eps * k * k;
        check( err <= tol, "pinv " + tag, err, tol );
    }

    template< typename T >
    void square( std::size_t n, std::mt19937_64& g, std::string const& tname )
    {
        std::string const tag = tname + " " + std::to_string( n ) + "x" + std::to_string( n );
        auto const a = random< T >( n, n, g );
        emat< T > const e = to_eigen( a );
        double const k = kappa( e );
        double const tol = 64.0 * n * eps * k;

        Eigen::PartialPivLU< emat< T > > lu( e );
        T const de = lu.determinant();
        T const df = feng::det( a );
        double err = std::abs( df - de ) / std::abs( de );
        check( err <= tol, "det " + tag, err, tol );

        auto const b = random< T >( n, 3, g );
        auto const xs = feng::solve( a, b );
        err = xs.ok() ? rel( xs.value, emat< T >( lu.solve( to_eigen( b ) ) ) ) : HUGE_VAL;
        check( err <= tol, "solve " + tag, err, tol );

        auto const xi = feng::try_inverse( a );
        err = xi.ok() ? rel( xi.value, emat< T >( lu.inverse() ) ) : HUGE_VAL;
        check( err <= tol, "inverse " + tag, err, tol );

        // Hermitian positive definite: A·Aᴴ + n·I, symmetrized exactly
        feng::matrix< T > h{ n, n };
        for ( std::size_t r = 0; r != n; ++r )
            for ( std::size_t c = 0; c != n; ++c )
            {
                T s = r == c ? T( double( n ) ) : T( 0 );
                for ( std::size_t j = 0; j != n; ++j ) s += a[r][j] * conj_( a[c][j] );
                h[r][c] = s;
            }
        for ( std::size_t r = 0; r != n; ++r )
        {
            h[r][r] = T( std::real( h[r][r] ) );
            for ( std::size_t c = 0; c != r; ++c ) h[r][c] = conj_( h[c][r] );
        }
        emat< T > const eh = to_eigen( h );
        Eigen::LLT< emat< T > > llt( eh );
        auto const ch = feng::cholesky_factor( h );
        double const tolc = 64.0 * n * eps * kappa( eh );
        err = ( ch.ok() && llt.info() == Eigen::Success ) ? rel( ch.l(), emat< T >( llt.matrixL() ) ) : HUGE_VAL;
        check( err <= tolc, "cholesky " + tag, err, tolc );

        svd_and_pinv( a, tag );
    }

    template< typename T >
    void run( std::string const& tname, std::uint64_t seed )
    {
        std::mt19937_64 g{ seed };
        for ( std::size_t n : { 1, 4, 8 } ) square< T >( n, g, tname );
        svd_and_pinv( random< T >( 6, 3, g ), tname + " 6x3" );
        svd_and_pinv( random< T >( 3, 7, g ), tname + " 3x7" );
        // rank 2, 5x5: a 5x2 times a 2x5 product
        auto const b = random< T >( 5, 2, g );
        auto const c = random< T >( 2, 5, g );
        svd_and_pinv( feng::matrix< T >( b * c ), tname + " 5x5 rank 2" );
    }
}

int main()
{
    std::cout << "Eigen " << EIGEN_WORLD_VERSION << "." << EIGEN_MAJOR_VERSION << "." << EIGEN_MINOR_VERSION;
#ifdef EIGEN_VERSION_STRING
    std::cout << " (" << EIGEN_VERSION_STRING << ")";
#endif
    std::cout << "\n";
    run< double >( "double", 20261001 );
    run< std::complex< double > >( "complex<double>", 20261002 );
    if ( failed != 0 )
    {
        std::cout << "EIGEN FAIL " << failed << " of " << ( passed + failed ) << " comparisons\n";
        return 1;
    }
    std::cout << "EIGEN PASS " << passed << " comparisons\n";
    return 0;
}
