// S1-R4 API family `linalg`: det, inverse/inv, lu_factor (S7-R1), svd_factor and status pinverse (S7-R3), solve, try_inverse, lu_decomposition, lu_solver, svd, pinv/pinverse,
// cholesky_factor, row_echelon and status expm (S7-R4, S7-R5), cholesky_decomposition, rref, gauss_jordan_elimination, householder, forward/backward_substitution, expm, eigen_*,
// conjugate gradient solvers, the S9-R5 to_expected / load_expected adapters. Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
#include "../../matrix.hpp"

#include <complex>
#include <valarray>
#include <vector>

namespace
{
    template < typename M >
    auto sink( M const& m ) -> typename M::value_type
    {
        return m.size() ? m[0][0] : typename M::value_type{};
    }

    template < typename T >
    T exercise_inverse()
    {
        // determinant and inverse: real and complex
        feng::matrix<T> const a = feng::eye<T>( 3, 3 );
        T acc{};
        acc += a.det();
        acc += feng::det( a );
        acc += sink( a.inverse() );
        acc += sink( feng::inverse( a ) );
        acc += sink( feng::inv( a ) );
        acc += sink( feng::expm( a ) );

        // S7-R1, S7-R2: the LU object, status results and the status overload of inverse
        feng::matrix<T> const b{ 3, 2, T{ 1 } };
        auto const f = feng::lu_factor( a );
        acc += static_cast<T>( f.rank() ) + static_cast<T>( f.pivots()[0] ) + f.det();
        acc += f.status() == feng::linalg_status::ok ? T{ 1 } : T{ 0 };
        acc += sink( f.l() ) + sink( f.u() ) + sink( f.p() );
        if ( auto const r = f.solve( b ); r )
            acc += sink( r.value );
        if ( auto const r = f.inverse(); r.ok() )
            acc += sink( r.value );
        if ( auto const r = feng::solve( a, b ); r.status == feng::linalg_status::ok )
            acc += sink( r.value );
        if ( auto const r = feng::try_inverse( a ); r )
            acc += sink( r.value );
        feng::matrix<T> out;
        if ( feng::inverse( a, out ) == feng::linalg_status::ok )
            acc += sink( out );

        // S7-R4, S7-R5: the Cholesky object, row_echelon and the status overload of expm
        auto const c = feng::cholesky_factor( a );
        acc += c.status() == feng::linalg_status::ok && c.ok() ? sink( c.l() ) : T{ 0 };
        auto const e = feng::row_echelon( b );
        acc += sink( e.r ) + static_cast<T>( static_cast<double>( e.rank + e.pivot_columns.size() ) );
        acc += e.status == feng::linalg_status::ok ? T{ 1 } : T{ 0 };
        if ( auto const r = feng::rref( b ); r )
            acc += sink( *r );
        if ( feng::expm( a, out ) == feng::linalg_status::ok )
            acc += sink( out );
        acc += static_cast<T>( static_cast<double>( feng::cholesky_decomposition( a, out ) ) );
#if defined( __cpp_lib_expected )
        // S9-R5: std::expected adapters over the status results and the loaders
        if ( auto const x = feng::to_expected( feng::try_inverse( a ) ); x )
            acc += sink( *x );
        if ( auto const x = feng::to_expected( f.solve( b ) ); x )
            acc += sink( *x );
        if ( auto const x = feng::to_expected( f.inverse() ); x )
            acc += sink( *x );
        if ( auto const x = feng::to_expected( feng::solve( a, b ) ); x )
            acc += sink( *x );
        if ( auto const x = feng::to_expected( feng::lu_factor( a ) ); x )
            acc += x->det();
        if ( auto const x = feng::to_expected( feng::svd_factor( a ) ); x )
            acc += sink( x->u() );
        if ( auto const x = feng::to_expected( feng::cholesky_factor( a ) ); x )
            acc += sink( x->l() );
        if ( auto const x = feng::to_expected( feng::row_echelon( b ) ); x )
            acc += sink( x->r );
        for ( auto const fmt : { feng::io_format::txt, feng::io_format::binary, feng::io_format::npy } )
            if ( auto const x = feng::load_expected<T>( "missing.npy", fmt ); x )
                acc += sink( *x );
#endif
        return acc;
    }

    template < typename T >
    T exercise_svd()
    {
        // S7-R3: the SVD object, the status pinverse and the legacy SVD and pseudoinverse spellings
        feng::matrix<T> const a = feng::eye<T>( 3, 2 );
        feng::matrix<T> u, w, v, x;
        T acc{};
        auto const f = feng::svd_factor( a );
        auto const g = feng::svd_factor( a, 8 );
        acc += sink( f.u() ) + sink( f.v() ) + T( f.s()[0] ) + T( static_cast<double>( f.sweeps() + f.rank() + g.rank( 0.5 ) ) );
        acc += f.status() == feng::linalg_status::ok ? T{ 1 } : T{ 0 };
        if ( auto const r = f.pinverse(); r )
            acc += sink( r.value );
        if ( auto const r = f.pinverse( 1.0e-3 ); r.ok() )
            acc += sink( r.value );
#if defined( __cpp_lib_expected )
        // S9-R5: std::expected adapters over the SVD object and its pinverse
        if ( auto const x = feng::to_expected( f ); x )
            acc += sink( x->v() );
        if ( auto const x = feng::to_expected( f.pinverse() ); x )
            acc += sink( *x );
#endif
        if ( feng::pinverse( a, x ) == feng::linalg_status::ok )
            acc += sink( x );
        if ( feng::pinverse( a, x, 1.0e-3 ) == feng::linalg_status::ok )
            acc += sink( x );
        acc += static_cast<T>( static_cast<double>( feng::singular_value_decomposition( a, u, w, v ) ) );
        acc += static_cast<T>( static_cast<double>( feng::singular_value_decomposition( a, u, w, v, 16 ) ) );
        if ( auto const s = feng::singular_value_decomposition( a ); s )
            acc += sink( std::get<0>( *s ) );
        if ( auto const s = feng::svd( a ); s )
            acc += sink( std::get<1>( *s ) );
        acc += sink( feng::svd_inverse( a ) );
        acc += sink( feng::pinverse( a ) ) + sink( feng::pinverse( a, 1.0e-3 ) );
        acc += sink( feng::pinv( a ) ) + sink( feng::pinv( a, 1.0e-3 ) );
        return acc;
    }

    template < typename T >
    T exercise_real()
    {
        feng::matrix<T> const a = feng::eye<T>( 3, 3 );
        feng::matrix<T> const b{ 3, 1, T{ 1 } };
        feng::matrix<T> x, l, u, w, v, q, d;
        T acc{};

        // LU
        acc += static_cast<T>( feng::lu_decomposition( a, l, u ) );
        if ( auto const lu = feng::lu_decomposition( a ); lu )
            acc += sink( std::get<0>( *lu ) ) + sink( std::get<1>( *lu ) );
        acc += static_cast<T>( feng::lu_solver( a, x, b ) );
        if ( auto const s = feng::lu_solver( a, b ); s )
            acc += sink( *s );

        // substitution, Cholesky, Householder, row reduction
        acc += static_cast<T>( feng::forward_substitution( a, x, b ) );
        acc += static_cast<T>( feng::backward_substitution( a, x, b ) );
        acc += static_cast<T>( feng::cholesky_decomposition( a, l ) );
        feng::householder( a, q, d );
        acc += sink( l ) + sink( q ) + sink( d );
        if ( auto const r = feng::rref( a ); r )
            acc += sink( *r );
        if ( auto const g = feng::gauss_jordan_elimination( a ); g )
            acc += sink( *g );

        // SVD and pseudo-inverse
        acc += static_cast<T>( feng::singular_value_decomposition( a, u, w, v ) );
        if ( auto const s = feng::singular_value_decomposition( a ); s )
            acc += sink( std::get<0>( *s ) );
        if ( auto const s = feng::svd( a ); s )
            acc += sink( std::get<1>( *s ) );
        acc += sink( feng::svd_inverse( a ) );
        acc += sink( feng::pinverse( a ) );
        acc += sink( feng::pinv( a ) );

        // conjugate gradient solvers
        acc += static_cast<T>( feng::conjugate_gradient_squared( a, x, b ) );
        acc += static_cast<T>( feng::cgs( a, x, b ) );
        acc += static_cast<T>( feng::biconjugate_gradient_stabilized_method( a, x, b ) );
        acc += static_cast<T>( feng::bicgstab( a, x, b ) );

        // eigen solvers
        std::vector<T> lv;
        std::valarray<T> la;
        feng::matrix<T> lm;
        acc += static_cast<T>( feng::eigen_jacobi( a, v, lv ) );
        acc += static_cast<T>( feng::eigen_jacobi( a, v, la ) );
        acc += static_cast<T>( feng::eigen_jacobi( a, v, lm ) );
        acc += static_cast<T>( feng::cyclic_eigen_jacobi( a, v, lv ) );
        acc += static_cast<T>( feng::cyclic_eigen_jacobi( a, v, la ) );
        acc += static_cast<T>( feng::cyclic_eigen_jacobi( a, v, lm ) );
        feng::eigen_real_symmetric( a, v, lv );
        feng::eigen_real_symmetric( a, v, la );
        feng::eigen_real_symmetric( a, v, lm );
        acc += feng::eigen_power_iteration( a );
        acc += feng::eigen_power_iteration( a, x.begin() );
        return acc;
    }

    template < typename T >
    T exercise_hermitian()
    {
        using C = std::complex<T>;
        feng::matrix<C> const a = feng::eye<C>( 3, 3 );
        feng::matrix<C> v, x{ 3, 1 };
        std::vector<T> lv;
        std::valarray<T> la;
        feng::matrix<T> lm;
        feng::eigen_hermitian( a, v, lv );
        feng::eigen_hermitian( a, v, la );
        feng::eigen_hermitian( a, v, lm );
        T acc = std::real( sink( v ) );
        acc += feng::eigen_power_iteration( a );
        acc += feng::eigen_power_iteration( a, x.begin() );
        return acc;
    }
} // namespace

double api_linalg_double() { return exercise_inverse<double>() + exercise_svd<double>() + exercise_real<double>() + exercise_hermitian<double>(); }
float api_linalg_float() { return exercise_inverse<float>() + exercise_svd<float>() + exercise_real<float>(); }
std::complex<double> api_linalg_complex() { return exercise_inverse<std::complex<double>>() + exercise_svd<std::complex<double>>(); }
std::complex<float> api_linalg_complex_float() { return exercise_svd<std::complex<float>>(); }
