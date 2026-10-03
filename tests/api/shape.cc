// S1-R4 API family `shape`: reshape, resize, transpose, ctranspose, fliplr, flipud, flipdim, tril, triu, clear, swap,
// shrink_to_size. Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
#include "../../matrix.hpp"

#include <complex>

namespace
{
    template < typename M >
    auto sink( M const& m ) -> typename M::value_type
    {
        return m.size() ? m[0][0] : typename M::value_type{};
    }

    template < typename T >
    T exercise_shape()
    {
        feng::matrix<T> m{ 4, 6, T{ 1 } };
        T acc{};

        acc += sink( m.reshape( 6, 4 ) );
        acc += sink( m.resize( 3, 8 ) );
        acc += sink( m.resize( 5, 5 ) );
        acc += sink( m.shrink_to_size( 3, 3 ) );
        acc += sink( m.transpose() );
        acc += sink( feng::transpose( m ) );
        acc += sink( feng::fliplr( m ) );
        acc += sink( feng::flipud( m ) );
        acc += sink( feng::flipdim( m, 1 ) );
        acc += sink( feng::flipdim( m, 2 ) );
        acc += sink( feng::tril( m ) );
        acc += sink( feng::triu( m ) );

        feng::matrix<T> other{ 2, 2, T{ 2 } };
        m.swap( other );
        acc += sink( m ) + sink( other );
        m.clear();
        acc += static_cast<T>( static_cast<int>( m.size() ) );
        return acc;
    }

    std::complex<double> exercise_ctranspose()
    {
        feng::matrix<std::complex<double>> const m{ 2, 3, std::complex<double>{ 1.0, 2.0 } };
        return sink( feng::ctranspose( m ) );
    }
} // namespace

double api_shape_double() { return exercise_shape<double>(); }
float api_shape_float() { return exercise_shape<float>(); }
std::complex<double> api_shape_complex() { return exercise_shape<std::complex<double>>() + exercise_ctranspose(); }
int api_shape_int() { return exercise_shape<int>(); }
