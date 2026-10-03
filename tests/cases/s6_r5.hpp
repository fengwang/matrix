// S6-R5 (PR-9): the promotion policy for scalar and mixed-type arithmetic (D-023, F17).
// matrix (+) matrix gives common_element_t<T, U>; matrix (+) scalar keeps T unless the scalar's kind
// (integral < floating < complex) is higher; compound assignment keeps T; the result allocator is A rebound.
#include <complex>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>

#include "./s3_alloc.hpp"

namespace s6_r5
{
    using cf = std::complex<float>;
    using cd = std::complex<double>;

    template< typename T >
    using mat = feng::matrix<T>;

    template< typename T, typename S > using add_t  = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() + std::declval< S const& >() ) >;
    template< typename T, typename S > using radd_t = std::remove_cvref_t< decltype( std::declval< S const& >() + std::declval< mat<T> const& >() ) >;
    template< typename T, typename S > using sub_t  = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() - std::declval< S const& >() ) >;
    template< typename T, typename S > using rsub_t = std::remove_cvref_t< decltype( std::declval< S const& >() - std::declval< mat<T> const& >() ) >;
    template< typename T, typename S > using mul_t  = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() * std::declval< S const& >() ) >;
    template< typename T, typename S > using rmul_t = std::remove_cvref_t< decltype( std::declval< S const& >() * std::declval< mat<T> const& >() ) >;
    template< typename T, typename S > using div_t  = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() / std::declval< S const& >() ) >;
    template< typename T, typename S > using rdiv_t = std::remove_cvref_t< decltype( std::declval< S const& >() / std::declval< mat<T> const& >() ) >;

    template< typename T, typename U > using madd_t = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() + std::declval< mat<U> const& >() ) >;
    template< typename T, typename U > using msub_t = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() - std::declval< mat<U> const& >() ) >;
    template< typename T, typename U > using mmul_t = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() * std::declval< mat<U> const& >() ) >;
    template< typename T, typename U > using mdiv_t = std::remove_cvref_t< decltype( std::declval< mat<T> const& >() / std::declval< mat<U> const& >() ) >;

    // The expected element type, written out independently of the library's traits.
    template< typename T > constexpr int kind() { if constexpr ( std::is_integral_v<T> ) return 0; else if constexpr ( std::is_floating_point_v<T> ) return 1; else return 2; }
    template< typename T > struct real_of { using type = T; };
    template< typename X > struct real_of< std::complex<X> > { using type = X; };
    template< typename T, typename U >
    using expected_common_t = std::conditional_t< kind<T>() == 2 || kind<U>() == 2,
                                                  std::complex< std::common_type_t< typename real_of<T>::type, typename real_of<U>::type > >,
                                                  std::common_type_t< T, U > >;
    template< typename T, typename S >
    using expected_scalar_t = std::conditional_t< ( kind<S>() <= kind<T>() ), T, expected_common_t< T, S > >;

    template< typename T, typename S >
    constexpr bool scalar_pair()
    {
        using R = expected_scalar_t< T, S >;
        static_assert( std::is_same_v< feng::matrix_details::scalar_result_t< T, S >, R > );
        static_assert( std::is_same_v< add_t<T, S>,  mat<R> > );
        static_assert( std::is_same_v< radd_t<T, S>, mat<R> > );
        static_assert( std::is_same_v< sub_t<T, S>,  mat<R> > );
        static_assert( std::is_same_v< rsub_t<T, S>, mat<R> > );
        static_assert( std::is_same_v< mul_t<T, S>,  mat<R> > );
        static_assert( std::is_same_v< rmul_t<T, S>, mat<R> > );
        static_assert( std::is_same_v< div_t<T, S>,  mat<R> > );
        static_assert( std::is_same_v< rdiv_t<T, S>, mat<R> > );
        return true;
    }

    template< typename T, typename U >
    constexpr bool matrix_pair()
    {
        using R = expected_common_t< T, U >;
        static_assert( std::is_same_v< feng::matrix_details::common_element_t< T, U >, R > );
        static_assert( std::is_same_v< madd_t<T, U>, mat<R> > );
        static_assert( std::is_same_v< msub_t<T, U>, mat<R> > );
        static_assert( std::is_same_v< mmul_t<T, U>, mat<R> > );
        static_assert( std::is_same_v< mdiv_t<T, U>, mat<R> > );
        return true;
    }

    template< typename T, typename... S >
    constexpr bool scalar_row() { return ( scalar_pair< T, S >() && ... ); }
    template< typename T, typename... U >
    constexpr bool matrix_row() { return ( matrix_pair< T, U >() && ... ); }

    template< typename... T >
    constexpr bool scalar_grid() { return ( scalar_row< T, int, unsigned, std::uint8_t, float, double, cf, cd >() && ... ); }
    template< typename... T >
    constexpr bool matrix_grid() { return ( matrix_row< T, int, unsigned, std::uint8_t, float, double, cf, cd >() && ... ); }

    static_assert( scalar_grid< int, unsigned, std::uint8_t, float, double, cf, cd >() );
    static_assert( matrix_grid< int, unsigned, std::uint8_t, float, double, cf, cd >() );

    // The traits as D-023 states them, spot-checked by hand.
    static_assert( feng::matrix_details::element_kind<int>::value == 0 );
    static_assert( feng::matrix_details::element_kind<double>::value == 1 );
    static_assert( feng::matrix_details::element_kind<cf>::value == 2 );
    static_assert( std::is_same_v< feng::matrix_details::common_element_t< cf, double >, cd > );
    static_assert( std::is_same_v< feng::matrix_details::common_element_t< int, unsigned >, unsigned > );
    static_assert( std::is_same_v< feng::matrix_details::scalar_result_t< float, double >, float > );
    static_assert( std::is_same_v< feng::matrix_details::scalar_result_t< std::uint8_t, int >, std::uint8_t > );
    static_assert( std::is_same_v< feng::matrix_details::scalar_result_t< int, double >, double > );
    static_assert( std::is_same_v< feng::matrix_details::scalar_result_t< float, cd >, cd > );
    static_assert( std::is_same_v< feng::matrix_details::scalar_result_t< cf, double >, cf > );

    // A scalar is an arithmetic type or a std::complex: matrices, pointers, valarray and vector are not.
    static_assert( feng::matrix_details::matrix_scalar<int> && feng::matrix_details::matrix_scalar<cd> );
    static_assert( !feng::matrix_details::matrix_scalar< mat<double> > );
    static_assert( !feng::matrix_details::matrix_scalar< double const* > );
    static_assert( !feng::matrix_details::matrix_scalar< std::valarray<double> > );
    static_assert( !feng::matrix_details::matrix_scalar< std::vector<double> > );

    // Compound assignment keeps the left element type.
    template< typename T, typename S > using cadd_t = decltype( std::declval< mat<T>& >() += std::declval< S const& >() );
    template< typename T, typename S > using cmul_t = decltype( std::declval< mat<T>& >() *= std::declval< S const& >() );
    static_assert( std::is_same_v< cadd_t< int, double >, mat<int>& > );
    static_assert( std::is_same_v< cmul_t< float, double >, mat<float>& > );
    static_assert( std::is_same_v< cadd_t< double, mat<int> >, mat<double>& > );
    static_assert( std::is_same_v< cmul_t< double, mat<float> >, mat<double>& > );
}

TEST_CASE( "S6-R5 scalar result types", "[S6][S6-R5]" )
{
    using namespace s6_r5;

    SECTION( "matrix<int> * 0.5 promotes to double" )
    {
        mat<int> const m{ 1, 2, { 1, 2 } };
        auto const r = m * 0.5;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<double> > );
        REQUIRE( r[0][0] == 0.5 );
        REQUIRE( r[0][1] == 1.0 );
        auto const l = 0.5 * m;
        REQUIRE( l[0][0] == 0.5 );
        REQUIRE( l[0][1] == 1.0 );
        auto const d = m / 4.0;
        REQUIRE( d[0][0] == 0.25 );
        REQUIRE( d[0][1] == 0.5 );
        auto const s = 3.5 - m;
        REQUIRE( s[0][0] == 2.5 );
        REQUIRE( s[0][1] == 1.5 );
    }

    SECTION( "matrix<double> * 2 compiles and keeps double" )
    {
        mat<double> const m{ 1, 2, { 1.5, -2.0 } };
        auto const r = m * 2;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<double> > );
        REQUIRE( r[0][0] == 3.0 );
        REQUIRE( r[0][1] == -4.0 );
        auto const p = 1 + m;
        REQUIRE( p[0][0] == 2.5 );
        REQUIRE( p[0][1] == -1.0 );
    }

    SECTION( "matrix<float> * 2.0 stays float, the scalar converted to float first" )
    {
        mat<float> const m{ 1, 2, { 1.0f, 3.0f } };
        auto const r = m * 2.0;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<float> > );
        REQUIRE( r[0][0] == 2.0f );
        REQUIRE( r[0][1] == 6.0f );
        // 0.1 rounded to float first, then multiplied in float.
        auto const t = m * 0.1;
        REQUIRE( t[0][1] == 3.0f * static_cast<float>( 0.1 ) );
    }

    SECTION( "matrix<uint8_t> + 1 stays uint8_t and wraps" )
    {
        mat<std::uint8_t> const m{ 1, 2, { std::uint8_t( 10 ), std::uint8_t( 255 ) } };
        auto const r = m + 1;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<std::uint8_t> > );
        REQUIRE( r[0][0] == 11 );
        REQUIRE( r[0][1] == 0 );
        auto const s = 1 - m;
        REQUIRE( s[0][0] == std::uint8_t( 1 - 10 ) );
        REQUIRE( s[0][1] == 2 );
    }

    SECTION( "complex matrices with real scalars keep the complex type" )
    {
        mat<cd> const m{ 1, 2, { cd( 1.0, 2.0 ), cd( -1.0, 0.5 ) } };
        auto const r = m * 2;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<cd> > );
        REQUIRE( r[0][0] == cd( 2.0, 4.0 ) );
        REQUIRE( r[0][1] == cd( -2.0, 1.0 ) );
        auto const a = 1.0 + m;
        REQUIRE( a[0][0] == cd( 2.0, 2.0 ) );
        auto const s = 1 - m;
        REQUIRE( s[0][1] == cd( 2.0, -0.5 ) );
        mat<cf> const f{ 1, 1, { cf( 1.0f, 1.0f ) } };
        auto const g = f * 2.0;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( g ) >, mat<cf> > );
        REQUIRE( g[0][0] == cf( 2.0f, 2.0f ) );
    }

    SECTION( "real matrices with complex scalars promote to complex" )
    {
        mat<int> const m{ 1, 2, { 1, 2 } };
        auto const r = m * cd( 0.0, 1.0 );
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<cd> > );
        REQUIRE( r[0][0] == cd( 0.0, 1.0 ) );
        REQUIRE( r[0][1] == cd( 0.0, 2.0 ) );
    }

    SECTION( "scalar / matrix is the scalar times the inverse" )
    {
        mat<int> const m{ 2, 2, { 2, 0, 0, 4 } };
        auto const r = 1.0 / m;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<double> > );
        REQUIRE( r[0][0] == 0.5 );
        REQUIRE( r[0][1] == 0.0 );
        REQUIRE( r[1][1] == 0.25 );
        mat<double> const d{ 2, 2, { 2.0, 0.0, 0.0, 4.0 } };
        auto const q = 2 / d;
        REQUIRE( q[0][0] == 1.0 );
        REQUIRE( q[1][1] == 0.5 );
    }

    SECTION( "compound assignment keeps T, the scalar converted as by static_cast" )
    {
        mat<int> m{ 1, 2, { 3, 5 } };
        m *= 0.5;
        // 0.5 becomes static_cast<int>( 0.5 ) == 0 before the multiplication.
        REQUIRE( m[0][0] == 0 );
        REQUIRE( m[0][1] == 0 );
        mat<int> n{ 1, 2, { 3, 5 } };
        n *= 2.9;
        REQUIRE( n[0][0] == 6 );
        REQUIRE( n[0][1] == 10 );
        mat<float> f{ 1, 1, { 1.0f } };
        f += 2.0;
        REQUIRE( f[0][0] == 3.0f );
    }

    SECTION( "the result allocator is A rebound to the result type" )
    {
        using alloc_i = s3_alloc::tracking_allocator< int, false, false, false >;
        using alloc_d = s3_alloc::tracking_allocator< double, false, false, false >;
        s3_alloc::reset();
        {
            feng::matrix< int, alloc_i > const m{ alloc_i{ 7 }, 1, 2 };
            auto const r = m * 0.5;
            static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, feng::matrix< double, alloc_d > > );
            REQUIRE( r.get_allocator().id == 7 );
            auto const l = 2.0 + m;
            REQUIRE( l.get_allocator().id == 7 );
            auto const k = m + 1;
            static_assert( std::is_same_v< std::remove_cvref_t< decltype( k ) >, feng::matrix< int, alloc_i > > );
            REQUIRE( k.get_allocator().id == 7 );
        }
        s3_alloc::require_balanced();
    }
}

TEST_CASE( "S6-R5 mixed matrix result types", "[S6][S6-R5]" )
{
    using namespace s6_r5;

    SECTION( "int + double gives double" )
    {
        mat<int> const a{ 1, 2, { 1, 2 } };
        mat<double> const b{ 1, 2, { 0.5, 0.25 } };
        auto const r = a + b;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<double> > );
        REQUIRE( r[0][0] == 1.5 );
        REQUIRE( r[0][1] == 2.25 );
        auto const s = b - a;
        REQUIRE( s[0][0] == -0.5 );
        REQUIRE( s[0][1] == -1.75 );
    }

    SECTION( "double + complex<float> gives complex<double>" )
    {
        mat<double> const a{ 1, 1, { 1.0 } };
        mat<cf> const b{ 1, 1, { cf( 0.5f, 2.0f ) } };
        auto const r = a + b;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<cd> > );
        REQUIRE( r[0][0] == cd( 1.5, 2.0 ) );
    }

    SECTION( "product and division of mixed types" )
    {
        mat<int> const a{ 1, 2, { 1, 2 } };
        mat<double> const b{ 2, 1, { 0.5, 0.25 } };
        auto const p = a * b;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( p ) >, mat<double> > );
        REQUIRE( p.row() == 1 );
        REQUIRE( p.col() == 1 );
        REQUIRE( p[0][0] == 1.0 );
        mat<int> const c{ 1, 2, { 1, 2 } };
        mat<double> const d{ 2, 2, { 2.0, 0.0, 0.0, 4.0 } };
        auto const q = c / d;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( q ) >, mat<double> > );
        REQUIRE( q[0][0] == 0.5 );
        REQUIRE( q[0][1] == 0.5 );
    }

    SECTION( "same-type results are unchanged" )
    {
        mat<int> const a{ 1, 2, { 1, 2 } };
        mat<int> const b{ 1, 2, { 3, 4 } };
        auto const r = a + b;
        static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, mat<int> > );
        REQUIRE( r[0][0] == 4 );
        REQUIRE( r[0][1] == 6 );
    }

    SECTION( "compound assignment with another element type keeps T" )
    {
        mat<int> a{ 1, 2, { 1, 2 } };
        mat<double> const b{ 1, 2, { 0.5, 1.5 } };
        a += b;
        // each element of b becomes static_cast<int> first: 0 and 1.
        REQUIRE( a[0][0] == 1 );
        REQUIRE( a[0][1] == 3 );
        mat<double> c{ 1, 2, { 1.0, 2.0 } };
        mat<int> const d{ 1, 2, { 1, 1 } };
        c -= d;
        REQUIRE( c[0][0] == 0.0 );
        REQUIRE( c[0][1] == 1.0 );
    }

    SECTION( "the result allocator is the left allocator rebound" )
    {
        using alloc_i = s3_alloc::tracking_allocator< int, false, false, false >;
        using alloc_d = s3_alloc::tracking_allocator< double, false, false, false >;
        s3_alloc::reset();
        {
            feng::matrix< int, alloc_i > const a{ alloc_i{ 9 }, 1, 2 };
            mat<double> const b{ 1, 2, { 0.5, 0.5 } };
            auto const r = a + b;
            static_assert( std::is_same_v< std::remove_cvref_t< decltype( r ) >, feng::matrix< double, alloc_d > > );
            REQUIRE( r.get_allocator().id == 9 );
            REQUIRE( r[0][0] == 0.5 );
            feng::matrix< int, alloc_i > const c{ alloc_i{ 9 }, 1, 2, 1 };
            auto const s = c + a;
            static_assert( std::is_same_v< std::remove_cvref_t< decltype( s ) >, feng::matrix< int, alloc_i > > );
            REQUIRE( s.get_allocator().id == 9 );
        }
        s3_alloc::require_balanced();
    }
}
