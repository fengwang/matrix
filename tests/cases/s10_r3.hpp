// S10-R3 (PR-13): the cache-blocked GEMM kernel behind operator*= gives results bit-identical to the strided
// inner_product kernel it replaced (matrix_details::gemm_reference), for every element type and shape, and its row
// split over forced worker counts equals the 1-worker run.
#include <complex>
#include <cstddef>
#include <cstring>

namespace s10_r3
{
    template < typename T >
    T value( std::size_t i, int salt )
    {
        if constexpr ( std::is_same_v< T, int > )
            return static_cast< int >( ( i * 7 + static_cast< std::size_t >( salt ) * 3 ) % 11 ) - 5;
        else if constexpr ( std::is_same_v< T, std::complex< double > > )
            return { 0.1 * static_cast< double >( ( i + static_cast< std::size_t >( salt ) ) % 13 ) - 0.37 * static_cast< double >( i % 7 ),
                     1.0 / 3.0 + 1e-3 * static_cast< double >( i ) - 0.21 * static_cast< double >( ( i * 5 ) % 9 ) };
        else
            return static_cast< T >( 0.1 * static_cast< double >( ( i + static_cast< std::size_t >( salt ) ) % 13 ) - 0.37 * static_cast< double >( i % 7 ) + 1e-3 * static_cast< double >( i ) );
    }

    template < typename T >
    feng::matrix< T > sample( std::size_t r, std::size_t c, int salt )
    {
        feng::matrix< T > m( r, c );
        for ( std::size_t i = 0; i != m.size(); ++i )
            m.data()[i] = value< T >( i, salt );
        return m;
    }

    template < typename T >
    bool same_bits( feng::matrix< T > const& x, feng::matrix< T > const& y )
    {
        if ( x.row() != y.row() || x.col() != y.col() ) return false;
        if ( x.size() == 0 ) return true;
        return std::memcmp( x.data(), y.data(), x.size() * sizeof( T ) ) == 0;
    }

    template < typename T >
    void check( std::size_t m, std::size_t k, std::size_t n )
    {
        INFO( m << "x" << k << " * " << k << "x" << n );
        auto const a = sample< T >( m, k, 1 );
        auto const b = sample< T >( k, n, 2 );
        auto const ref = feng::matrix_details::gemm_reference( a, b );
        REQUIRE( ref.row() == m );
        REQUIRE( ref.col() == n );
        auto c = a;
        c *= b;
        REQUIRE( same_bits( c, ref ) );
        REQUIRE( same_bits( a * b, ref ) );
        auto d = a;
        d.direct_multiply( b );
        REQUIRE( same_bits( d, ref ) );
    }

    template < typename T >
    void check_all_shapes()
    {
        check< T >( 0, 4, 3 );
        check< T >( 0, 0, 0 );
        check< T >( 3, 0, 5 );
        check< T >( 1, 1, 1 );
        check< T >( 1, 9, 1 );
        check< T >( 1, 1, 9 );
        check< T >( 1, 7, 9 );
        check< T >( 9, 7, 1 );
        check< T >( 16, 16, 16 );
        check< T >( 17, 17, 17 );
        check< T >( 18, 18, 18 );
        check< T >( 33, 65, 31 );
        check< T >( 300, 7, 5 );
        check< T >( 5, 7, 300 );
        check< T >( 131, 131, 1 );
        check< T >( 70, 140, 270 );
    }

    template < typename T >
    void check_self_square( std::size_t n )
    {
        auto a = sample< T >( n, n, 3 );
        auto const ref = feng::matrix_details::gemm_reference( a, a );
        a *= a;
        REQUIRE( same_bits( a, ref ) );
    }

    template < typename T >
    void check_workers( std::size_t m, std::size_t k, std::size_t n )
    {
        INFO( m << "x" << k << " * " << k << "x" << n );
        auto const a = sample< T >( m, k, 4 );
        auto const b = sample< T >( k, n, 5 );
        feng::matrix< T > one( m, n );
        feng::matrix_details::gemm_blocked( a.data(), b.data(), one.data(), m, k, n, 1 );
        REQUIRE( same_bits( one, feng::matrix_details::gemm_reference( a, b ) ) );
        for ( std::size_t w : { std::size_t{ 2 }, std::size_t{ 3 }, std::size_t{ 7 } } )
        {
            INFO( "workers " << w );
            feng::matrix< T > c( m, n );
            feng::matrix_details::gemm_blocked( a.data(), b.data(), c.data(), m, k, n, w );
            REQUIRE( same_bits( c, one ) );
        }
    }
}

TEST_CASE( "S10-R3 blocked GEMM equals the reference kernel", "[S10][S10-R3]" )
{
    SECTION( "int" ) { s10_r3::check_all_shapes< int >(); }
    SECTION( "float" ) { s10_r3::check_all_shapes< float >(); }
    SECTION( "double" ) { s10_r3::check_all_shapes< double >(); }
    SECTION( "complex<double>" ) { s10_r3::check_all_shapes< std::complex< double > >(); }
    SECTION( "a *= a" )
    {
        s10_r3::check_self_square< double >( 1 );
        s10_r3::check_self_square< double >( 18 );
        s10_r3::check_self_square< std::complex< double > >( 33 );
        s10_r3::check_self_square< int >( 64 );
    }
    SECTION( "forced worker counts equal the 1-worker run" )
    {
        s10_r3::check_workers< double >( 37, 29, 41 );
        s10_r3::check_workers< double >( 2, 5, 3 );
        s10_r3::check_workers< double >( 50, 50, 1 );
        s10_r3::check_workers< float >( 0, 3, 4 );
        s10_r3::check_workers< std::complex< double > >( 23, 19, 17 );
        s10_r3::check_workers< int >( 13, 11, 9 );
    }
}

// S10-R3 (D-008, par-thresholds): the default worker counts are work-based (matrix_details::work_workers); every
// thresholded path equals its serial reference. Elementwise ops visit each index once, so any split gives the
// 1-worker bits; reductions keep reduce_range's chunked fold with the worker count work_workers picks.
namespace s10_r3
{
    template < typename T >
    feng::matrix< T > serial_add( feng::matrix< T > const& a, feng::matrix< T > const& b )
    {
        feng::matrix< T > c = a;
        T* x = c.data();
        T const* y = b.data();
        feng::matrix_details::parallel_workers( [x, y]( std::size_t i ) { x[i] += y[i]; }, std::size_t{ 0 }, c.size(), 1 );
        return c;
    }

    template < typename T >
    void check_elementwise( std::size_t r, std::size_t c )
    {
        INFO( "elementwise " << r << "x" << c );
        auto const a = sample< T >( r, c, 6 );
        auto const b = sample< T >( r, c, 7 );
        auto const ref_add = serial_add( a, b );
        REQUIRE( same_bits( a + b, ref_add ) );
        auto s = a;
        s += b;
        REQUIRE( same_bits( s, ref_add ) );

        feng::matrix< T > ref_minus = a;
        for ( std::size_t i = 0; i != ref_minus.size(); ++i ) ref_minus.data()[i] -= b.data()[i];
        auto m = a;
        m -= b;
        REQUIRE( same_bits( m, ref_minus ) );

        feng::matrix< T > ref_neg = a;
        for ( std::size_t i = 0; i != ref_neg.size(); ++i ) ref_neg.data()[i] = -ref_neg.data()[i];
        REQUIRE( same_bits( -a, ref_neg ) );

        auto const twice = []( T& v ) { v = v + v; };
        feng::matrix< T > ref_apply = a;
        for ( std::size_t i = 0; i != ref_apply.size(); ++i ) twice( ref_apply.data()[i] );
        auto ap = a;
        ap.apply( twice );
        REQUIRE( same_bits( ap, ref_apply ) );

        // for_each (behind transform and the unary maps)
        auto const f = []( T const& x ) { return x * x - x; };
        feng::matrix< T > ref_map( r, c );
        for ( std::size_t i = 0; i != a.size(); ++i ) ref_map.data()[i] = f( a.data()[i] );
        REQUIRE( same_bits( feng::matrix_details::transform( a, f ), ref_map ) );

        // row-wise copy into a larger matrix and clone back out
        feng::matrix< T > big( r + 2, c + 3 );
        for ( std::size_t i = 0; i != big.size(); ++i ) big.data()[i] = value< T >( i, 8 );
        feng::matrix< T > ref_big = big;
        for ( std::size_t i = 0; i != r; ++i )
            for ( std::size_t j = 0; j != c; ++j )
                ref_big[i + 1][j + 2] = a[i][j];
        big.copy( a, { 1, 1 + r }, { 2, 2 + c } );
        REQUIRE( same_bits( big, ref_big ) );
        if ( r != 0 && c != 0 ) // clone rejects an empty range
        {
            feng::matrix< T > cl;
            cl.clone( big, 1, 1 + r, 2, 2 + c );
            REQUIRE( same_bits( cl, a ) );
        }
    }

    template < typename T >
    void check_reduce( std::size_t n )
    {
        INFO( "reduce n = " << n );
        feng::matrix< T > m( 1, n );
        for ( std::size_t i = 0; i != n; ++i ) m.data()[i] = value< T >( i, 9 );
        auto const plus = []( T const& x, T const& y ) { return x + y; };
        std::size_t const w = feng::matrix_details::work_workers( n, feng::matrix_details::reduce_grain );
        auto const at = [&m]( std::size_t i ) noexcept -> T const& { return m.data()[i]; };
        T const chunked = feng::matrix_details::reduce_range< T >( at, n, T{}, plus, w );
        T serial{};
        for ( std::size_t i = 0; i != n; ++i ) serial = plus( serial, m.data()[i] );
        if ( w == 1 ) REQUIRE( std::memcmp( &chunked, &serial, sizeof( T ) ) == 0 );
        T const by_iter = feng::matrix_details::reduce( m.begin(), m.end(), T{}, plus );
        T const by_impl = feng::matrix_details::reduce_impl_private::reduce_impl( m )( plus, T{} );
        T const by_sum = feng::sum( m );
        REQUIRE( std::memcmp( &by_iter, &chunked, sizeof( T ) ) == 0 );
        REQUIRE( std::memcmp( &by_impl, &chunked, sizeof( T ) ) == 0 );
        REQUIRE( std::memcmp( &by_sum, &chunked, sizeof( T ) ) == 0 );
    }

    template < typename T >
    void check_thresholded()
    {
        for ( auto [r, c] : { std::pair< std::size_t, std::size_t >{ 0, 0 }, { 0, 5 }, { 1, 1 }, { 16, 16 }, { 17, 17 }, { 18, 18 },
                              { 70000, 5 }, { 5, 70000 } } )
            check_elementwise< T >( r, c );
        for ( std::size_t n : { std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 16 }, std::size_t{ 17 }, std::size_t{ 18 },
                                std::size_t{ 1000 }, feng::matrix_details::reduce_grain, 3 * feng::matrix_details::reduce_grain + 7 } )
            check_reduce< T >( n );
        check< T >( 16, 16, 16 );
        check< T >( 17, 17, 17 );
        check< T >( 18, 18, 18 );
        check< T >( 1, 1, 1 );
        check< T >( 0, 4, 3 );
        check< T >( 400, 64, 64 );  // tall: above the GEMM grain
        check< T >( 64, 64, 400 );  // wide
    }
}

TEST_CASE( "S10-R3 thresholded parallel helpers equal the serial path", "[S10][S10-R3]" )
{
    SECTION( "work_workers: 1 for little work or in serial builds, else min( cores, work / grain )" )
    {
        using feng::matrix_details::work_workers;
        REQUIRE( work_workers( 0, 1024 ) == 1 );
        REQUIRE( work_workers( 16, feng::matrix_details::elementwise_grain ) == 1 );
        REQUIRE( work_workers( feng::matrix_details::elementwise_grain - 1, feng::matrix_details::elementwise_grain ) == 1 );
        REQUIRE( work_workers( 5, 0 ) >= 1 );
        std::size_t const three = work_workers( 3 * feng::matrix_details::gemm_grain + 1, feng::matrix_details::gemm_grain );
        std::size_t const huge = work_workers( std::numeric_limits< std::size_t >::max(), 1 );
#ifdef FENG_MATRIX_PARALLEL
        unsigned const hc = std::thread::hardware_concurrency();
        std::size_t const cores = hc == 0 ? 1 : hc;
        REQUIRE( three == std::min< std::size_t >( cores, 3 ) );
        REQUIRE( huge == cores );
#else
        REQUIRE( three == 1 );
        REQUIRE( huge == 1 );
#endif
    }
    SECTION( "int" ) { s10_r3::check_thresholded< int >(); }
    SECTION( "float" ) { s10_r3::check_thresholded< float >(); }
    SECTION( "double" ) { s10_r3::check_thresholded< double >(); }
    SECTION( "complex<double>" ) { s10_r3::check_thresholded< std::complex< double > >(); }
}

// S10-R3 (B-042, fft-plans): fft2 builds one plan per length per call (twiddle tables, Bluestein chirp and its
// transform) and reuses it for every row and column; the per-row transform it replaced is
// matrix_details::fft2_reference. The plan's values come from the same expressions, so the bits agree.
namespace s10_r3
{
    template < typename C >
    bool same_complex( feng::matrix< C > const& x, feng::matrix< C > const& y )
    {
        if ( x.row() != y.row() || x.col() != y.col() ) return false;
        for ( std::size_t i = 0; i != x.size(); ++i )
            if ( !( x.data()[i].real() == y.data()[i].real() && x.data()[i].imag() == y.data()[i].imag() ) ) return false;
        return true;
    }

    template < typename T >
    void check_fft()
    {
        for ( auto [r, c] : { std::pair< std::size_t, std::size_t >{ 0, 0 }, { 1, 1 }, { 1, 13 }, { 1, 32 }, { 13, 1 }, { 32, 1 },
                              { 16, 16 }, { 17, 17 }, { 18, 18 }, { 8, 12 }, { 12, 8 }, { 125, 125 }, { 128, 64 }, { 250, 3 } } )
        {
            INFO( "fft " << r << "x" << c );
            auto const x = sample< T >( r, c, 10 );
            auto const f = feng::fft( x );
            auto const fr = feng::matrix_details::fft2_reference( x, -1 );
            REQUIRE( f.row() == r );
            REQUIRE( f.col() == c );
            REQUIRE( same_complex( f, fr ) );
            auto const g = feng::ifft( x );
            REQUIRE( same_complex( g, feng::matrix_details::fft2_reference( x, +1 ) ) );
        }
    }
}

TEST_CASE( "S10-R3 fft with reused plans equals the per-row transform", "[S10][S10-R3]" )
{
    SECTION( "int" ) { s10_r3::check_fft< int >(); }
    SECTION( "float" ) { s10_r3::check_fft< float >(); }
    SECTION( "double" ) { s10_r3::check_fft< double >(); }
    SECTION( "complex<double>" ) { s10_r3::check_fft< std::complex< double > >(); }
}
