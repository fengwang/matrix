// S1-R4 API family `arithmetic`: + - * / with scalars, matrices, valarray and vector; compound and prefix
// operators; `^` power. Compiled alone by `tools/check.sh api`; calling each operator instantiates its template.
#include "../../matrix.hpp"

#include <complex>
#include <valarray>
#include <vector>

namespace
{
    template < typename T >
    T sink( feng::matrix<T> const& m )
    {
        return m.size() ? m[0][0] : T{};
    }

    template < typename T >
    T exercise_arithmetic()
    {
        feng::matrix<T> a{ 3, 3, T{ 1 } };
        feng::matrix<T> b{ 3, 3, T{ 2 } };
        T const s{ 3 };
        T acc{};

        // matrix with matrix
        acc += sink( a + b );
        acc += sink( a - b );
        acc += sink( a * b );

        // matrix with scalar, both sides
        acc += sink( a + s );
        acc += sink( s + a );
        acc += sink( a - s );
        acc += sink( s - a );
        acc += sink( a * s );
        acc += sink( s * a );
        acc += sink( a / s );

        // valarray, vector and pointer products
        std::valarray<T> const va( T{ 1 }, 3 );
        std::vector<T> const ve( 3, T{ 1 } );
        acc += sink( a * va );
        acc += sink( va * a );
        acc += sink( a * ve );
        acc += sink( ve * a );
        acc += sink( a * ve.data() );
        acc += sink( ve.data() * a );

        // compound operators
        feng::matrix<T> c{ a };
        c += b;
        c += s;
        c -= b;
        c -= s;
        c *= b;
        c *= s;
        c /= s;
        acc += sink( c );

        // prefix operators
        acc += sink( -a );
        acc += sink( +a );

        // power
        acc += sink( a ^ 0 );
        acc += sink( a ^ 5 );
        return acc;
    }

    template < typename T >
    T exercise_division()
    {
        // matrix / matrix and scalar / matrix go through the inverse, so floating-point and complex types only.
        feng::matrix<T> a = feng::eye<T>( 3, 3 );
        feng::matrix<T> b = feng::eye<T>( 3, 3 );
        T const s{ 2 };
        T acc{};
        acc += sink( a / b );
        acc += sink( s / a );
        feng::matrix<T> c{ a };
        c /= b;
        acc += sink( c );
        return acc;
    }
} // namespace

double api_arithmetic_double() { return exercise_arithmetic<double>() + exercise_division<double>(); }
float api_arithmetic_float() { return exercise_arithmetic<float>() + exercise_division<float>(); }
std::complex<double> api_arithmetic_complex()
{
    return exercise_arithmetic<std::complex<double>>() + exercise_division<std::complex<double>>();
}
int api_arithmetic_int() { return exercise_arithmetic<int>(); }
