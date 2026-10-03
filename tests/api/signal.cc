// S1-R4 API family `signal`: conv, conv2, fft, ifft, fftshift, ifftshift, pooling.
// Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
#include "../../matrix.hpp"

#include <complex>
#include <string>

namespace
{
    template < typename M >
    auto sink( M const& m ) -> typename M::value_type
    {
        return m.size() ? m[0][0] : typename M::value_type{};
    }

    template < typename T >
    T exercise_conv()
    {
        feng::matrix<T> const a{ 5, 6, T{ 1 } };
        feng::matrix<T> const k{ 3, 3, T{ 1 } };
        T acc{};
        acc += sink( feng::conv( a, k ) );
        acc += sink( feng::conv( a, k, std::string{ "same" } ) );
        acc += sink( feng::conv( a, k, std::string{ "valid" } ) );
        acc += sink( feng::conv2( a, k ) );
        acc += sink( feng::conv2( a, k, std::string{ "same" } ) );
        return acc;
    }

    template < typename T >
    T exercise_pooling()
    {
        feng::matrix<T> const a{ 4, 6, T{ 1 } };
        T acc{};
        acc += sink( feng::pooling( a, 2, 3 ) );
        acc += sink( feng::pooling( a, 2, 2, "max" ) );
        acc += sink( feng::pooling( a, 2, 2, "min" ) );
        acc += sink( feng::pooling( a, 2 ) );
        acc += sink( feng::pooling( a, 2, "max" ) );
        return acc;
    }

    // fft, ifft and the shifts on every element type of D-031: float, double, long double, int and complex
    template < typename T >
    std::complex<double> exercise_fft()
    {
        feng::matrix<T> const a{ 4, 4, T{ 1 } };
        std::complex<double> acc{};
        acc += sink( feng::fft( a ) );
        acc += sink( feng::ifft( a ) );
        acc += sink( feng::ifft( feng::fft( a ) ) );
        acc += sink( feng::fftshift( a ) );
        acc += sink( feng::ifftshift( a ) );
        return acc;
    }
} // namespace

double api_signal_double() { return exercise_conv<double>() + exercise_pooling<double>(); }
float api_signal_float() { return exercise_conv<float>() + exercise_pooling<float>(); }
std::complex<double> api_signal_complex()
{
    return exercise_conv<std::complex<double>>() + exercise_fft<double>() + exercise_fft<std::complex<double>>()
         + exercise_fft<float>() + exercise_fft<int>() + exercise_fft<long double>()
         + exercise_fft<std::complex<float>>();
}
int api_signal_int() { return exercise_conv<int>() + exercise_pooling<int>(); }
