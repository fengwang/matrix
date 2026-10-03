// S6-R1 (PR-8): one partition drives parallel and both reductions; every index once, init once, every worker
// joined (F11, F12).
#include <atomic>
#include <complex>
#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace s6_r1
{
    // Visits [first, last) with `workers` and checks: every index exactly once (atomic counters), nothing outside
    // the range touched, and every chunk's plain slot written before return (a missing join is a TSan race).
    template< typename I >
    void check_visits( I first, I last, std::size_t workers )
    {
        std::size_t const n = first < last ? static_cast<std::size_t>( last - first ) : 0;
        std::size_t const pad = 4;
        std::size_t const base = static_cast<std::size_t>( first < last ? first : last );
        std::unique_ptr<std::atomic<int>[]> visits{ new std::atomic<int>[ base + n + pad ] };
        for ( std::size_t i = 0; i != base + n + pad; ++i ) visits[i].store( 0 );
        std::vector<std::size_t> slots( n, 0 ); // plain, non-atomic; one slot per index, written by its worker
        feng::matrix_details::parallel_workers( [&]( I i )
        {
            visits[static_cast<std::size_t>( i )].fetch_add( 1 );
            slots[static_cast<std::size_t>( i - first )] = static_cast<std::size_t>( i ) + 1;
        }, first, last, workers );
        for ( std::size_t i = 0; i != base + n + pad; ++i )
        {
            bool const inside = i >= static_cast<std::size_t>( first ) && i < base + n && n != 0;
            REQUIRE( visits[i].load() == ( inside ? 1 : 0 ) );
        }
        for ( std::size_t j = 0; j != n; ++j )
            REQUIRE( slots[j] == static_cast<std::size_t>( first ) + j + 1 );
    }

    template< typename I >
    void check_chunks( I first, I last, std::size_t workers )
    {
        std::size_t const n = first < last ? static_cast<std::size_t>( last - first ) : 0;
        std::size_t const w = feng::matrix_details::effective_workers( first, last, workers );
        if ( n == 0 ) { REQUIRE( w == 0 ); return; }
        REQUIRE( w == std::clamp( workers, std::size_t{1}, n ) );
        I expected = first;
        for ( std::size_t k = 0; k != w; ++k )
        {
            auto const [b, e] = feng::matrix_details::chunk_bounds( first, last, workers, k );
            REQUIRE( b == expected ); // contiguous, in order, disjoint
            REQUIRE( b < e );         // non-empty
            std::size_t const len = static_cast<std::size_t>( e - b );
            REQUIRE( len == n / w + ( k < n % w ? 1 : 0 ) );
            expected = e;
        }
        REQUIRE( expected == last ); // covers [first, last)
    }

    // Each chunk sets its own plain slot; the caller reads them after return with no other synchronisation.
    inline void check_chunk_slots( std::size_t first, std::size_t last, std::size_t workers )
    {
        std::size_t const w = feng::matrix_details::effective_workers( first, last, workers );
        std::vector<std::size_t> slot( w, 0 );
        feng::matrix_details::parallel_workers( [&]( std::size_t k )
        {
            auto const [b, e] = feng::matrix_details::chunk_bounds( first, last, workers, k );
            slot[k] = e - b;
        }, std::size_t{0}, w, w );
        std::size_t total = 0;
        for ( auto s : slot ) { REQUIRE( s > 0 ); total += s; }
        REQUIRE( total == last - first );
    }

    inline std::vector<std::size_t> worker_counts( std::size_t n )
    {
        return { 0, 1, 2, 7, 32, n + 1, n + 100 };
    }
}

TEST_CASE( "S6-R1 parallel visits every index once for injected worker counts", "[S6][S6-R1]" )
{
    SECTION( "[0, n) for n in 1, 10, 100, 1000" )
    {
        for ( std::size_t n : { 1UL, 10UL, 100UL, 1000UL } )
            for ( std::size_t w : s6_r1::worker_counts( n ) )
            {
                CAPTURE( n ); CAPTURE( w );
                s6_r1::check_visits( std::size_t{0}, n, w );
                s6_r1::check_chunks( std::size_t{0}, n, w );
                s6_r1::check_chunk_slots( 0, n, w );
            }
    }
    SECTION( "[5, 17) with 32 workers and other counts" )
    {
        s6_r1::check_visits( 5, 17, 32 );
        s6_r1::check_chunks( 5, 17, 32 );
        s6_r1::check_chunk_slots( 5, 17, 32 );
        for ( std::size_t w : s6_r1::worker_counts( 12 ) )
        {
            CAPTURE( w );
            s6_r1::check_visits( 5UL, 17UL, w );
            s6_r1::check_chunks( 5UL, 17UL, w );
            s6_r1::check_chunk_slots( 5, 17, w );
        }
        s6_r1::check_visits( 1000, 1777, 7 );
        s6_r1::check_chunks( 1000, 1777, 7 );
    }
    SECTION( "empty and reversed ranges call nothing" )
    {
        for ( std::size_t w : { 0UL, 1UL, 2UL, 7UL, 32UL } )
        {
            int calls = 0;
            feng::matrix_details::parallel_workers( [&]( int ) { ++calls; }, 3, 3, w );
            feng::matrix_details::parallel_workers( [&]( int ) { ++calls; }, 9, 3, w );
            feng::matrix_details::parallel_workers( [&]( std::size_t ) { ++calls; }, 0UL, 0UL, w );
            REQUIRE( calls == 0 );
            REQUIRE( feng::matrix_details::effective_workers( 3, 3, w ) == 0 );
            REQUIRE( feng::matrix_details::effective_workers( 9, 3, w ) == 0 );
            s6_r1::check_chunks( 3, 3, w );
        }
    }
    SECTION( "public parallel( func, first, last, threshold ) and parallel( func, last ) visit every index once" )
    {
        for ( std::size_t n : { 0UL, 1UL, 33UL, 2000UL } )
            for ( unsigned long threshold : { 0UL, 1024UL } )
            {
                CAPTURE( n ); CAPTURE( threshold );
                std::unique_ptr<std::atomic<int>[]> visits{ new std::atomic<int>[ n + 8 ] };
                for ( std::size_t i = 0; i != n + 8; ++i ) visits[i].store( 0 );
                feng::matrix_details::parallel( [&]( std::size_t i ) { visits[i].fetch_add( 1 ); }, std::size_t{3}, n + 3, threshold );
                for ( std::size_t i = 0; i != n + 8; ++i )
                    REQUIRE( visits[i].load() == ( i >= 3 && i < n + 3 ? 1 : 0 ) );
            }
        std::unique_ptr<std::atomic<int>[]> visits{ new std::atomic<int>[ 50 ] };
        for ( std::size_t i = 0; i != 50; ++i ) visits[i].store( 0 );
        feng::matrix_details::parallel( [&]( std::size_t i ) { visits[i].fetch_add( 1 ); }, std::size_t{50} );
        for ( std::size_t i = 0; i != 50; ++i )
            REQUIRE( visits[i].load() == 1 );
    }
    SECTION( "default_workers is 1 for small or serial jobs and at least 1 otherwise" )
    {
        REQUIRE( feng::matrix_details::default_workers( 10, 1024 ) == 1 );
        REQUIRE( feng::matrix_details::default_workers( 0, 0 ) == 1 );
        std::size_t const big = feng::matrix_details::default_workers( 1UL << 20, 32 );
#ifdef FENG_MATRIX_PARALLEL
        unsigned const hc = std::thread::hardware_concurrency();
        REQUIRE( big == ( hc == 0 ? 1 : hc ) );
#else
        REQUIRE( big == 1 );
#endif
    }
}

TEST_CASE( "S6-R1 both reductions include init exactly once", "[S6][S6-R1]" )
{
    auto const plus = []( auto a, auto b ) { return a + b; };

    SECTION( "ten elements, init 100, workers 0: 100 + sum, for matrix<double> and matrix<int>" )
    {
        feng::matrix<double> md( 2, 5 );
        std::iota( md.begin(), md.end(), 1.0 );
        feng::matrix<int> mi( 2, 5 );
        std::iota( mi.begin(), mi.end(), 1 );
        REQUIRE( feng::matrix_details::reduce( md.begin(), md.end(), 100.0, plus, 0 ) == 155.0 );
        REQUIRE( feng::matrix_details::reduce( mi.begin(), mi.end(), 100, plus, 0 ) == 155 );
        REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( md )( plus, 100.0, 0 ) == 155.0 );
        REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( mi )( plus, 100, 0 ) == 155 );
    }
    SECTION( "every worker count, sizes 0, 1, 10, 1000: both reductions equal std::accumulate" )
    {
        for ( std::size_t n : { 0UL, 1UL, 10UL, 1000UL } )
        {
            feng::matrix<int> mi( 1, n );
            std::iota( mi.begin(), mi.end(), -7 );
            feng::matrix<double> md( n, 1 );
            std::iota( md.begin(), md.end(), 0.5 );
            int const ei = std::accumulate( mi.begin(), mi.end(), 100, plus );
            double const ed = std::accumulate( md.begin(), md.end(), 100.0, plus );
            for ( std::size_t w : s6_r1::worker_counts( n ) )
            {
                CAPTURE( n ); CAPTURE( w );
                REQUIRE( feng::matrix_details::reduce( mi.begin(), mi.end(), 100, plus, w ) == ei );
                REQUIRE( feng::matrix_details::reduce( md.begin(), md.end(), 100.0, plus, w ) == ed );
                REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( mi )( plus, 100, w ) == ei );
                REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( md )( plus, 100.0, w ) == ed );
            }
            // default worker count
            REQUIRE( feng::matrix_details::reduce( mi.begin(), mi.end(), 100, plus ) == ei );
            REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( mi )( plus, 100 ) == ei );
            REQUIRE( feng::matrix_details::reduce( plus, 100 )( mi ) == ei );
        }
    }
    SECTION( "string concatenation shows order and a single init (iterator reduce)" )
    {
        std::vector<std::string> v;
        for ( char c = 'a'; c != 'a' + 26; ++c ) v.emplace_back( 1, c );
        auto const cat = []( std::string const& a, std::string const& b ) { return a + b; };
        std::string const expected = std::accumulate( v.begin(), v.end(), std::string{ "<" }, cat );
        REQUIRE( expected == "<abcdefghijklmnopqrstuvwxyz" );
        for ( std::size_t w : s6_r1::worker_counts( v.size() ) )
        {
            CAPTURE( w );
            REQUIRE( feng::matrix_details::reduce( v.begin(), v.end(), std::string{ "<" }, cat, w ) == expected );
            REQUIRE( feng::matrix_details::reduce( v.begin() + 5, v.begin() + 17, std::string{ "<" }, cat, w ) == "<fghijklmnopq" );
            REQUIRE( feng::matrix_details::reduce( v.begin(), v.begin(), std::string{ "<" }, cat, w ) == "<" );
        }
        REQUIRE( feng::matrix_details::reduce( v.begin(), v.end(), std::string{ "<" }, cat ) == expected );
    }
    SECTION( "affine-map composition shows order and a single init (curried reduce_impl)" )
    {
        // x -> a x + b stored as complex( a, b ); compose( f, g ) = g after f: associative, not commutative.
        using C = std::complex<double>;
        auto const compose = []( C f, C g ) { return C{ f.real() * g.real(), g.real() * f.imag() + g.imag() }; };
        feng::matrix<C> m( 2, 6 );
        for ( std::size_t i = 0; i != m.size(); ++i )
            m.data()[i] = C{ ( i % 2 == 0 ) ? 2.0 : 1.0, static_cast<double>( i + 1 ) };
        C const init{ 1.0, 3.0 };
        C const expected = std::accumulate( m.begin(), m.end(), init, compose );
        REQUIRE( expected != std::accumulate( m.begin(), m.end(), C{ 1.0, 0.0 }, compose ) );
        for ( std::size_t w : s6_r1::worker_counts( m.size() ) )
        {
            CAPTURE( w );
            REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( m )( compose, init, w ) == expected );
            REQUIRE( feng::matrix_details::reduce( m.begin(), m.end(), init, compose, w ) == expected );
        }
    }
}

// S10-T7 (S10-R3, S10-R5): the work-based default worker counts (matrix_details::work_workers) keep small calls on
// the caller, so this case, run by the tsan lane, drives the same partition with more than one worker through the
// GEMM row split (injected and default counts), parallel_work and the default reductions on inputs above the grains.
TEST_CASE( "S6-R1 GEMM row split and work-based defaults run the partition on several workers", "[S6][S6-R1]" )
{
    SECTION( "gemm_blocked: injected worker counts equal the 1-worker run" )
    {
        std::size_t const m = 37, k = 29, n = 41;
        feng::matrix<double> a( m, k ), b( k, n );
        for ( std::size_t i = 0; i != a.size(); ++i ) a.data()[i] = 0.25 * static_cast<double>( i % 17 ) - 1.0;
        for ( std::size_t i = 0; i != b.size(); ++i ) b.data()[i] = 0.5 * static_cast<double>( i % 13 ) - 2.0;
        feng::matrix<double> one( m, n );
        feng::matrix_details::gemm_blocked( a.data(), b.data(), one.data(), m, k, n, 1 );
        for ( std::size_t w : { 2UL, 3UL, 7UL, m, m + 5 } )
        {
            CAPTURE( w );
            feng::matrix<double> c( m, n );
            feng::matrix_details::gemm_blocked( a.data(), b.data(), c.data(), m, k, n, w );
            REQUIRE( c == one );
        }
    }
    SECTION( "operator* above the GEMM grain equals the 1-worker run" )
    {
        // 128 * 64 * 64 = 2^19 multiply-adds: two workers by default in parallel builds
        std::size_t const m = 128, k = 64, n = 64;
        REQUIRE( m * k * n >= 2 * feng::matrix_details::gemm_grain );
        feng::matrix<double> a( m, k ), b( k, n );
        for ( std::size_t i = 0; i != a.size(); ++i ) a.data()[i] = 0.125 * static_cast<double>( i % 11 ) - 0.5;
        for ( std::size_t i = 0; i != b.size(); ++i ) b.data()[i] = 0.375 * static_cast<double>( i % 7 ) - 1.0;
        feng::matrix<double> one( m, n );
        feng::matrix_details::gemm_blocked( a.data(), b.data(), one.data(), m, k, n, 1 );
        REQUIRE( ( a * b ) == one );
    }
    SECTION( "parallel_work and elementwise add above the elementwise grain visit every index once" )
    {
        std::size_t const n = 2 * feng::matrix_details::elementwise_grain + 3;
#ifdef FENG_MATRIX_PARALLEL
        if ( std::thread::hardware_concurrency() > 1 )
            REQUIRE( feng::matrix_details::work_workers( n, feng::matrix_details::elementwise_grain ) == 2 );
#endif
        std::unique_ptr<std::atomic<int>[]> visits{ new std::atomic<int>[ n ] };
        for ( std::size_t i = 0; i != n; ++i ) visits[i].store( 0 );
        std::vector<std::size_t> slots( n, 0 );
        feng::matrix_details::parallel_work( [&]( std::size_t i ) { visits[i].fetch_add( 1 ); slots[i] = i + 1; },
                                             std::size_t{ 0 }, n, 1, feng::matrix_details::elementwise_grain );
        bool once = true;
        for ( std::size_t i = 0; i != n; ++i ) once = once && visits[i].load() == 1 && slots[i] == i + 1;
        REQUIRE( once );

        feng::matrix<int> x( 1, n ), y( 1, n );
        std::iota( x.begin(), x.end(), 0 );
        std::iota( y.begin(), y.end(), 7 );
        auto const z = x + y;
        bool sums = true;
        for ( std::size_t i = 0; i != n; ++i ) sums = sums && z.data()[i] == static_cast<int>( 2 * i + 7 );
        REQUIRE( sums );
    }
    SECTION( "default reductions above the reduce grain equal reduce_range with the same worker count" )
    {
        std::size_t const n = 3 * feng::matrix_details::reduce_grain + 7;
        std::size_t const w = feng::matrix_details::work_workers( n, feng::matrix_details::reduce_grain );
        feng::matrix<double> m( 1, n );
        for ( std::size_t i = 0; i != n; ++i ) m.data()[i] = 0.1 * static_cast<double>( i % 9 ) - 0.3;
        auto const plus = []( double p, double q ) { return p + q; };
        auto const at = [&m]( std::size_t i ) noexcept -> double const& { return m.data()[i]; };
        double const chunked = feng::matrix_details::reduce_range<double>( at, n, 0.0, plus, w );
        REQUIRE( feng::matrix_details::reduce( m.begin(), m.end(), 0.0, plus ) == chunked );
        REQUIRE( feng::matrix_details::reduce_impl_private::reduce_impl( m )( plus, 0.0 ) == chunked );
        REQUIRE( feng::sum( m ) == chunked );
    }
}
