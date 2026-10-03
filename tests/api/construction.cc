// S1-R4 API family `construction`: constructors, zeros, ones, eye, arange, linspace, magic, hilbert, toeplitz, diag,
// make_diag, blkdiag, meshgrid, *_like, rand/random/randn. Compiled alone by `tools/check.sh api`; calling each API
// instantiates its template.
#include "../../matrix.hpp"

#include <complex>
#include <deque>
#include <memory>
#include <set>
#include <utility>
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
    T exercise_constructors()
    {
        using matrix_type = feng::matrix<T>;
        T acc{};

        // constructors: default, sized, filled, initializer list, allocator, copy, move, converting
        matrix_type const a;
        matrix_type const b{ 2, 3 };
        matrix_type const c{ 2, 3, T{ 1 } };
        matrix_type const d{ 2, 2, { T{ 1 }, T{ 2 }, T{ 3 }, T{ 4 } } };
        matrix_type const e{ std::allocator<T>{}, 2, 2, T{ 1 } };
        matrix_type f{ c };
        matrix_type const g{ std::move( f ) };
        feng::matrix<double> const h{ feng::matrix<int>{ 2, 2, 1 } };
        matrix_type i;
        i = c;
        i = matrix_type{ 2, 2 };
        i = T{ 1 };
        acc += sink( a ) + sink( b ) + sink( c ) + sink( d ) + sink( e ) + sink( g ) + sink( i );
        acc += static_cast<T>( static_cast<int>( h.size() ) );

        // zeros, ones, empty, eye
        acc += sink( feng::zeros<T>( 2, 3 ) );
        acc += sink( feng::zeros<T>( 2 ) );
        acc += sink( feng::zeros<T>( std::allocator<T>{}, 2, 3 ) );
        acc += sink( feng::zeros<T>( std::allocator<T>{}, 2 ) );
        acc += sink( feng::ones<T>( 2, 3 ) );
        acc += sink( feng::ones<T>( 2 ) );
        acc += sink( feng::ones<T>( std::allocator<T>{}, 2, 3 ) );
        acc += sink( feng::ones<T>( std::allocator<T>{}, 2 ) );
        acc += sink( feng::empty<T>( 2, 3 ) );
        acc += sink( feng::empty<T>( std::allocator<T>{}, 2UL, 3UL ) );
        acc += sink( feng::eye<T>( 2, 3 ) );
        acc += sink( feng::eye<T>( 3 ) );
        acc += sink( feng::eye( c ) );

        // *_like
        acc += sink( feng::zeros_like( c ) );
        acc += sink( feng::ones_like( c ) );

        // diag, make_diag, blkdiag
        acc += sink( feng::diag( d ) );
        acc += sink( feng::diag( d, 1 ) );
        acc += sink( feng::diag( std::vector<T>( 3, T{ 1 } ) ) );
        acc += sink( feng::diag( std::deque<T>( 3, T{ 1 } ), -1 ) );
        acc += sink( feng::diag( std::valarray<T>( T{ 1 }, 3 ) ) );
        acc += sink( feng::make_diag( T{ 1 }, T{ 2 }, T{ 3 } ) );
        acc += sink( feng::blkdiag( c, d ) );
        acc += sink( feng::blkdiag( c, d, e ) );
        acc += sink( feng::blk_diag( c, d ) );
        acc += sink( feng::block_diag( c, d ) );

        // toeplitz and hilbert through a prototype matrix
        std::vector<T> const col{ T{ 1 }, T{ 2 }, T{ 3 } };
        acc += sink( feng::toeplitz( col.begin(), col.end() ) );
        acc += sink( feng::toeplitz( col.begin(), col.end(), col.begin(), col.end() ) );
        acc += sink( feng::repmat( d, 2, 2 ) );
        return acc;
    }

    template < typename T >
    T exercise_real()
    {
        // APIs restricted to ordered (real or integer) element types
        T acc{};
        std::set<T> const s{ T{ 1 }, T{ 2 } };
        std::multiset<T> const ms{ T{ 1 }, T{ 1 } };
        acc += sink( feng::diag( s ) );
        acc += sink( feng::diag( ms ) );
        acc += sink( feng::arange<T>( 0, 6, 2 ) );
        acc += sink( feng::arange<T>( 5 ) );
        acc += sink( feng::linspace<T>( T{ 0 }, T{ 10 }, 5 ) );
        acc += sink( feng::linspace<T>( T{ 0 }, T{ 10 }, 5, false ) );
        acc += sink( feng::hilbert<T>( 3 ) );
        acc += sink( feng::hilb<T>( 3 ) );
        acc += sink( feng::hilbert( 3, feng::matrix<T>{} ) );
        acc += sink( feng::hilb( 3, feng::matrix<T>{} ) );
        auto const [mx, my] = feng::meshgrid( T{ 3 }, T{ 2 } );
        acc += sink( mx ) + sink( my );
        return acc;
    }

    template < typename T >
    T exercise_random()
    {
        feng::matrix<T> const m{ 2, 2, T{ 1 } };
        T acc{};
        acc += sink( feng::rand<T>( 2, 3 ) );
        acc += sink( feng::rand<T>( 2, 3, 42U ) );
        acc += sink( feng::rand<T>( 2 ) );
        acc += sink( feng::random<T>( 2, 3 ) );
        acc += sink( feng::random<T>( 2 ) );
        acc += sink( feng::rand_like( m ) );
        acc += sink( feng::random_like( m ) );
        acc += sink( feng::randn_like( m ) );
        return acc;
    }
} // namespace

double api_construction_double()
{
    return exercise_constructors<double>() + exercise_real<double>() + exercise_random<double>() +
           static_cast<double>( feng::magic( 4 )[0][0] );
}
float api_construction_float() { return exercise_constructors<float>() + exercise_real<float>() + exercise_random<float>(); }
std::complex<double> api_construction_complex() { return exercise_constructors<std::complex<double>>(); }
int api_construction_int() { return exercise_constructors<int>() + exercise_real<int>(); }
