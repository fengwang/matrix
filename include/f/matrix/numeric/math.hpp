#ifndef MMATH_HPP_INCLUDED_SDOFIHJFDHJSDFLKJXCVKJHSDKHSFDOIUHASFHUSFDKJSHFDHJUERKFDSKJSFDFSDFSDFDKJFSDMSIUHSFDKJHKJBCMJBS
#define MMATH_HPP_INCLUDED_SDOFIHJFDHJSDFLKJXCVKJHSDKHSFDOIUHASFHUSFDKJSHFDHJUERKFDSKJSFDFSDFSDFDKJFSDMSIUHSFDKJHKJBCMJBS

#include "../details/algorithm/for_each.hpp"

namespace f
{
#if 1
    namespace math_private
    {

        struct non_complex_tag {};
        struct complex_tag {};

        template<typename T>
        struct tag
        {
            typedef non_complex_tag result_tag;
        };

        template<typename T>
        struct tag<std::complex<T>>
        {
            typedef complex_tag result_tag;
        };

    }//namespace math_private
#endif

#if 1
    //complex cases
#define GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION( f_name )  \
    template< typename T, typename A > \
    const matrix<T,A> \
    f_name( const matrix<std::complex<T>, A> & mm ) \
    { \
        matrix<T,A> m(mm.row(), mm.col()); \
        for_each( m.begin(), m.end(), mm.begin(), []( T& t, std::complex<T> c ) { t = std:: f_name(c); } ); \
        return m; \
    }

    GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION( real );
    GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION( imag );
    GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION( abs );
    GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION( arg );
    GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION( norm );

#undef GENERATE_COMPLEX_MATRIX_MATH_UNARY_FUNCTION

    //complex only functions
#define GENERATE_COMPLEX_MATRIX_ONLY_MATH_UNARY_FUNCTION( f_name )  \
    template< typename T, typename A > \
    const matrix<std::complex<T>, A> \
    f_name( const matrix<std::complex<T>, A>& mm ) \
    { \
        matrix<std::complex<T>, A> m( mm ); \
        std::for_each( m.begin(), m.end(), [](std::complex<T>& val) { val = std:: f_name(val); } ); \
        return m; \
    }

    GENERATE_COMPLEX_MATRIX_ONLY_MATH_UNARY_FUNCTION( conj );
    GENERATE_COMPLEX_MATRIX_ONLY_MATH_UNARY_FUNCTION( proj );

#undef GENERATE_COMPLEX_MATRIX_ONLY_MATH_UNARY_FUNCTION

    //specialization for polar
    template< typename T, typename A, typename A_ >
    const matrix<std::complex<T>, A> polar( const matrix<T, A>& mm, const matrix<T, A_> nn )
    {
        assert( mm.row() == nn.row() );
        assert( mm.col() == nn.col() );
        matrix<std::complex<T>, A> m( mm.row(), mm.col() );
        for_each( mm.begin(), mm.end(), nn.begin(), m.begin(), []( const T _mm, const T _nn, std::complex<T>& m )
        {
            m = std::polar( _mm, _nn );
        } );
        return m;
    }

    template< typename T, typename A >
    const matrix<std::complex<T>, A> polar( const matrix<T, A>& mm, const T nn )
    {
        matrix<std::complex<T>, A> m( mm.row(), mm.col() );
        for_each( mm.begin(), mm.end(), m.begin(), [nn]( const T _mm, std::complex<T>& m )
        {
            m = std::polar( _mm, nn );
        } );
        return m;
    }

    template< typename T, typename A >
    const matrix<std::complex<T>, A> polar( const T nn, const matrix<T, A>& mm )
    {
        matrix<std::complex<T>, A> m( mm.row(), mm.col() );
        for_each( mm.begin(), mm.end(), m.begin(), [nn]( const T _mm, std::complex<T>& m )
        {
            m = std::polar( nn, _mm );
        } );
        return m;
    }

    //general case, i.e. for both complex and real
#define GENERATE_MATRIX_MATH_UNARY_FUNCTION( f_name )  \
    template< typename T, typename A > \
    const matrix<T, A> \
    f_name( const matrix<T,  A> & mm ) \
    { \
        matrix<T, A> m( mm ); \
        std::for_each( m.begin(), m.end(), [](T& val) { val = std:: f_name(val); } ); \
        return m; \
    }

    GENERATE_MATRIX_MATH_UNARY_FUNCTION( abs );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( acos );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( acosh );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( asin );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( asinh );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( atan );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( atanh );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( cbrt );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( ceil );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( cos );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( cosh );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( erf );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( erfc );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( exp );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( exp2 );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( expm1 );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( fabs );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( floor );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( fma );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( frexp );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( ilogb );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( lgamma );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( llrint );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( llround );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( log );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( log10 );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( log1p );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( log2 );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( logb );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( lrint );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( lround );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( modf );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( nan );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( nanf );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( nanl );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( nearbyint );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( nexttoward );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( remquo );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( rint );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( round );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( scalbln );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( scalbn );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( sin );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( sinh );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( sqrt );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( tan );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( tanh );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( tgamma );
    GENERATE_MATRIX_MATH_UNARY_FUNCTION( trunc );

#undef GENERATE_MATRIX_MATH_UNARY_FUNCTION


#define GENERATE_MATRIX_MATH_BINARY_FUNCTION( f_name )  \
    template< typename T1, typename A1, typename T2, typename A2 > \
    const matrix<T1, A1> \
    f_name( const matrix<T1, A1> & mm, const matrix<T2, A2>& nn ) \
    { \
        assert( mm.row() == nn.row() ); \
        assert( mm.col() == nn.col() ); \
        auto m( mm ); \
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T1 v1, T2 v2 ){ return std:: f_name( v1, v2 ); } ); \
        return m; \
    } \
    template< typename T1, typename A1, typename T2 > \
    const matrix<T1, A1> \
    f_name( const matrix<T1, A1> & mm, const T2 v ) \
    { \
        auto m( mm ); \
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T1 v1 ){ return std:: f_name( v1, v ); } ); \
        return m; \
    } \
    template< typename T1, typename A1, typename T2 > \
    const matrix<T1, A1> \
    f_name( const T2 v, const matrix<T1, A1> & mm ) \
    { \
        auto m( mm ); \
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T1 v1 ){ return std:: f_name( v, v1 ); } ); \
        return m; \
    }

    GENERATE_MATRIX_MATH_BINARY_FUNCTION( atan2 );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( copysign );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( fdim );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( fmax );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( fmin );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( fmod );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( hypot );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( ldexp );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( nextafter );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( nexttoward );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( pow );
    GENERATE_MATRIX_MATH_BINARY_FUNCTION( remainder );

#undef GENERATE_MATRIX_MATH_BINARY_FUNCTION

#endif

}

#endif//_MATH_HPP_INCLUDED_SDOFIHJFDHJSDFLKJXCVKJHSDKHSFDOIUHASFHUSFDKJSHFDHJUERKFDSKJSFDFSDFSDFDKJFSDMSIUHSFDKJHKJBCMJBS

