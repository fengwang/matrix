// S1-R4 API family `elementwise`: the unary and binary `<cmath>` wrappers, abs, real, imag, conj, arg, polar, clip,
// astype, apply. Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
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
    double exercise_unary()
    {
        feng::matrix<T> const m{ 3, 3, T{ 0.5 } };
        double acc{};
        acc += sink( feng::abs( m ) );
        acc += sink( feng::exp( m ) );
        acc += sink( feng::exp2( m ) );
        acc += sink( feng::expm1( m ) );
        acc += sink( feng::log( m ) );
        acc += sink( feng::log10( m ) );
        acc += sink( feng::log1p( m ) );
        acc += sink( feng::log2( m ) );
        acc += sink( feng::sqrt( m ) );
        acc += sink( feng::cbrt( m ) );
        acc += sink( feng::sin( m ) );
        acc += sink( feng::cos( m ) );
        acc += sink( feng::tan( m ) );
        acc += sink( feng::asin( m ) );
        acc += sink( feng::acos( m ) );
        acc += sink( feng::atan( m ) );
        acc += sink( feng::sinh( m ) );
        acc += sink( feng::cosh( m ) );
        acc += sink( feng::tanh( m ) );
        acc += sink( feng::asinh( m ) );
        acc += sink( feng::acosh( m ) );
        acc += sink( feng::atanh( m ) );
        acc += sink( feng::erf( m ) );
        acc += sink( feng::erfc( m ) );
        acc += sink( feng::tgamma( m ) );
        acc += sink( feng::lgamma( m ) );
        acc += sink( feng::trunc( m ) );
        acc += sink( feng::round( m ) );
        acc += sink( feng::ceil( m ) );
        acc += sink( feng::floor( m ) );
        acc += sink( feng::rint( m ) );
        acc += sink( feng::logb( m ) );
        acc += sink( feng::comp_ellint_1( m ) );
        acc += sink( feng::comp_ellint_2( m ) );
        acc += sink( feng::expint( m ) );
        acc += sink( feng::riemann_zeta( m ) );
        acc += sink( feng::nearbyint( m ) );
        acc += sink( feng::ilogb( m ) );
        acc += static_cast<double>( sink( feng::lrint( m ) ) );
        acc += static_cast<double>( sink( feng::llrint( m ) ) );
        acc += static_cast<double>( sink( feng::lround( m ) ) );
        acc += static_cast<double>( sink( feng::llround( m ) ) );
        acc += sink( feng::fma( m, m, m ) );
        return acc;
    }

    template < typename T >
    double exercise_binary()
    {
        feng::matrix<T> const m{ 3, 3, T{ 0.5 } };
        feng::matrix<T> const n{ 3, 3, T{ 2 } };
        double const x = 2.0;
        double acc{};
        acc += sink( feng::ldexp( m, feng::matrix<int>{ 3, 3, 1 } ) );
        acc += sink( feng::ldexp( m, x ) );
        acc += sink( feng::scalbn( m, feng::matrix<int>{ 3, 3, 1 } ) );
        acc += sink( feng::scalbln( m, feng::matrix<long>{ 3, 3, 1L } ) );
        acc += sink( feng::pow( m, n ) );
        acc += sink( feng::pow( m, 2 ) );
        acc += sink( feng::pow( m, x ) );
        acc += sink( feng::pow( x, m ) );
        acc += sink( feng::hypot( m, n ) );
        acc += sink( feng::hypot( m, x ) );
        acc += sink( feng::hypot( x, m ) );
        acc += sink( feng::fmod( m, n ) );
        acc += sink( feng::fmod( m, x ) );
        acc += sink( feng::fmod( x, m ) );
        acc += sink( feng::remainder( m, n ) );
        acc += sink( feng::remainder( m, x ) );
        acc += sink( feng::remainder( x, m ) );
        acc += sink( feng::copysign( m, n ) );
        acc += sink( feng::copysign( m, x ) );
        acc += sink( feng::copysign( x, m ) );
        acc += sink( feng::nextafter( m, n ) );
        acc += sink( feng::nextafter( m, x ) );
        acc += sink( feng::nextafter( x, m ) );
        acc += sink( feng::fdim( m, n ) );
        acc += sink( feng::fdim( m, x ) );
        acc += sink( feng::fdim( x, m ) );
        acc += sink( feng::fmax( m, n ) );
        acc += sink( feng::fmax( m, x ) );
        acc += sink( feng::fmax( x, m ) );
        acc += sink( feng::fmin( m, n ) );
        acc += sink( feng::fmin( m, x ) );
        acc += sink( feng::fmin( x, m ) );
        acc += sink( feng::atan2( m, n ) );
        acc += sink( feng::atan2( m, x ) );
        acc += sink( feng::atan2( x, m ) );
        return acc;
    }

    template < typename T >
    T exercise_misc()
    {
        feng::matrix<T> m{ 3, 3, T{ 1 } };
        T acc{};
        acc += sink( feng::abs( m ) );
        acc += sink( feng::clip( T{ 0 }, T{ 2 } )( m ) );
        acc += static_cast<T>( sink( m.template astype<double>() ) );
        acc += static_cast<T>( sink( m.template astype<int>() ) );
        m.apply( []( T& v ) { v += T{ 1 }; } );
        m.elementwise_apply( []( T& v ) { v += T{ 1 }; } );
        m.map( []( T& v ) { v += T{ 1 }; } );
        acc += sink( m );
        return acc;
    }

    double exercise_complex()
    {
        using C = std::complex<double>;
        feng::matrix<C> m{ 3, 3, C{ 1.0, 2.0 } };
        double acc{};
        acc += sink( feng::real( m ) );
        acc += sink( feng::imag( m ) );
        acc += sink( feng::abs( m ) );
        acc += sink( feng::arg( m ) );
        acc += sink( feng::norm( m ) );
        acc += std::real( sink( feng::conj( m ) ) );
        acc += std::real( sink( feng::proj( m ) ) );
        acc += std::real( sink( m.astype<std::complex<float>>() ) );
        m.apply( []( C& v ) { v *= 2.0; } );
        acc += std::real( sink( m ) );
        return acc;
    }
} // namespace

double api_elementwise_double() { return exercise_unary<double>() + exercise_binary<double>() + exercise_misc<double>(); }
float api_elementwise_float()
{
    return static_cast<float>( exercise_unary<float>() + exercise_binary<float>() ) + exercise_misc<float>();
}
double api_elementwise_complex() { return exercise_complex(); }
int api_elementwise_int() { return exercise_misc<int>(); }
