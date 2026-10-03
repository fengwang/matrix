// S1-R4 API family `views`: make_view, matrix_view, every clone overload, copy, slicing.
// Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
#include "../../matrix.hpp"

#include <complex>
#include <cstddef>
#include <utility>

namespace
{
    template < typename T >
    T exercise_views()
    {
        using size_type = typename feng::matrix<T>::size_type;
        feng::matrix<T> m{ 5, 6, T{ 1 } };
        feng::matrix<T> const& cm = m;
        T acc{};

        // make_view and matrix_view, and the view's read API
        auto v = feng::make_view( cm, { 1UL, 4UL }, { 2UL, 5UL } );
        feng::matrix_view<T, std::allocator<T>> w{ cm, std::make_pair( size_type{ 0 }, size_type{ 2 } ),
                                                   std::make_pair( size_type{ 1 }, size_type{ 3 } ) };
        auto const [vr, vc] = v.shape();
        acc += static_cast<T>( static_cast<int>( vr + vc + v.row() + v.col() + v.size() + w.size() ) );
        acc += v[0][0];
        acc += v( 1, 1 );
        acc += *v.row_begin( 0 );
        acc += *v.row_cbegin( 1 );
        acc += w[1][1];

        // every clone overload
        feng::matrix<T> a;
        a.clone( cm, { 1, 3 }, { 0, 2 } );
        a.clone( cm, 1, 3, 0, 2 );
        auto const b = cm.clone( { 0, 2 }, { 1, 4 } );
        auto const c = cm.clone( 0, 2, 1, 4 );
        acc += a[0][0] + b[0][0] + c[0][0];

        // copy, whole and into a block
        feng::matrix<T> d;
        d.copy( cm );
        d.copy( a, { 0, 2 }, { 0, 2 } );
        acc += d[0][0];

        // slicing constructors
        typename feng::matrix<T>::range_type const rr{ 1, 3 }, rc{ 2, 4 };
        feng::matrix<T> const s1{ cm, { 1, 3 }, { 2, 4 } };
        feng::matrix<T> const s2{ cm, rr, rc };
        feng::matrix<T> const s3{ cm, 1, 3, 2, 4 };
        acc += s1[0][0] + s2[0][0] + s3[0][0];
        return acc;
    }
} // namespace

double api_views_double() { return exercise_views<double>(); }
float api_views_float() { return exercise_views<float>(); }
std::complex<double> api_views_complex() { return exercise_views<std::complex<double>>(); }
int api_views_int() { return exercise_views<int>(); }
