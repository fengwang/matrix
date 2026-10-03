// S2-R2 (PR-3): sizes are checked before the allocator is called.
#include "./s2_death.hpp"

#include <cstddef>
#include <cstdio>
#include <memory>

namespace s2_r2
{
    // Stateless allocator that reports every allocate call on stderr, so a death test can prove none happened.
    template < typename T >
    struct counting_allocator
    {
        typedef T value_type;
        counting_allocator() noexcept = default;
        template < typename U > counting_allocator( counting_allocator<U> const& ) noexcept {}
        T* allocate( std::size_t n )
        {
            std::fputs( "ALLOCATE\n", stderr );
            return std::allocator<T>{}.allocate( n );
        }
        void deallocate( T* p, std::size_t n ) noexcept { std::allocator<T>{}.deallocate( p, n ); }
        template < typename U > bool operator==( counting_allocator<U> const& ) const noexcept { return true; }
    };

    // Same as counting_allocator but with a small max_size, so the max_size bound is the first one to fire.
    template < typename T >
    struct small_allocator : counting_allocator<T>
    {
        small_allocator() noexcept = default;
        template < typename U > small_allocator( small_allocator<U> const& ) noexcept {}
        std::size_t max_size() const noexcept { return 1000; }
        template < typename U > bool operator==( small_allocator<U> const& ) const noexcept { return true; }
    };

    inline constexpr std::size_t two_62 = std::size_t{ 1 } << 62;
}

TEST_CASE( "S2 huge extents abort before allocation", "[S2][S2-R2]" )
{
    using s2_r2::two_62;
    using counted = feng::matrix< double, s2_r2::counting_allocator<double> >;

    SECTION( "the counting allocator is observable" )
    {
        auto const out = s2_death::run( []{ counted m{ 2, 2 }; (void)m; } );
        REQUIRE( out.exited );
        REQUIRE( s2_death::contains( out.err, "ALLOCATE" ) );
    }
    SECTION( "2^62 x 8 overflows rows*cols" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ two_62, 8 }; (void)m; }, "size" );
        auto const out = s2_death::run( []{ counted m{ two_62, 8 }; (void)m; } );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::contains( out.err, "size" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "ALLOCATE" ) );
    }
    SECTION( "2^62 x 1 doubles overflow the byte count" )
    {
        S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ two_62, 1 }; (void)m; }, "byte count" );
        auto const out = s2_death::run( []{ counted m{ s2_r2::counting_allocator<double>{}, two_62, 1 }; (void)m; } );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::contains( out.err, "contract violation" ) );
        REQUIRE( s2_death::contains( out.err, "byte count" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "ALLOCATE" ) );
    }
    SECTION( "100 x 20 is above an allocator max_size of 1000" )
    {
        using small = feng::matrix< double, s2_r2::small_allocator<double> >;
        auto const ok = s2_death::run( []{ small m{ 10, 10 }; (void)m; } );
        REQUIRE( ok.exited );
        REQUIRE( s2_death::contains( ok.err, "ALLOCATE" ) );
        auto const out = s2_death::run( []{ small m{ 100, 20 }; (void)m; } );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::contains( out.err, "contract violation" ) );
        REQUIRE( s2_death::contains( out.err, "max_size" ) );
        REQUIRE( s2_death::contains( out.err, "size" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "ALLOCATE" ) );
    }
    SECTION( "the (row, col, {values}) constructor checks before allocating" )
    {
        // 2^32 * 2^32 wraps to 0 in 64 bits; the product must be rejected, not treated as empty.
        auto const out = s2_death::run( []{ counted m{ std::size_t{ 1 } << 32, std::size_t{ 1 } << 32, { 1.0, 2.0 } }; (void)m; } );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::contains( out.err, "contract violation" ) );
        REQUIRE( s2_death::contains( out.err, "size" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "ALLOCATE" ) );
    }
}

#include <climits>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include <sys/stat.h>

namespace s2_r2
{
    using counted = feng::matrix< double, counting_allocator<double> >;

    // Marks the point after which a death test requires no allocate call.
    inline void ready() { std::fputs( "READY\n", stderr ); std::fflush( stderr ); }

    // Child-side checks of a death: SIGABRT, `expected` in the message, no ALLOCATE after READY.
    template < typename Callable >
    void require_death_without_allocate( Callable&& fn, std::string const& expected )
    {
        auto const out = s2_death::run( std::forward< Callable >( fn ) );
        INFO( "child stderr: " << out.err );
        INFO( "expected substring: " << expected );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::contains( out.err, "contract violation" ) );
        REQUIRE( s2_death::contains( out.err, expected ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "AddressSanitizer" ) );
        REQUIRE_FALSE( s2_death::contains( out.err, "runtime error:" ) );
        std::size_t const at = out.err.find( "READY\n" );
        REQUIRE( at != std::string::npos );
        REQUIRE_FALSE( s2_death::contains( out.err.substr( at ), "ALLOCATE" ) );
    }

    inline feng::matrix<double> numbered( std::size_t r, std::size_t c )
    {
        feng::matrix<double> m{ r, c };
        for ( std::size_t i = 0; i != m.size(); ++i ) m.data()[i] = static_cast<double>( i );
        return m;
    }

    inline counted numbered_counted( std::size_t r, std::size_t c )
    {
        counted m{ r, c };
        for ( std::size_t i = 0; i != m.size(); ++i ) m.data()[i] = static_cast<double>( i );
        return m;
    }
}

TEST_CASE( "S2 at and call operator abort out of range", "[S2][S2-R2]" )
{
    using s2_r2::numbered;
    using view_type = feng::matrix_view< double, std::allocator<double> >;

    SECTION( "in-range at and () agree, const and non-const" )
    {
        auto m = numbered( 3, 4 );
        auto const& cm = m;
        for ( std::size_t r = 0; r != 3; ++r )
            for ( std::size_t c = 0; c != 4; ++c )
            {
                REQUIRE( m.at( r, c ) == static_cast<double>( r * 4 + c ) );
                REQUIRE( cm.at( r, c ) == m( r, c ) );
                REQUIRE( &cm.at( r, c ) == &m.at( r, c ) );
            }
        m.at( 2, 3 ) = -1.0;
        REQUIRE( m( 2, 3 ) == -1.0 );
        view_type const v{ m, { 1, 3 }, { 1, 4 } };
        REQUIRE( v.at( 0, 0 ) == m( 1, 1 ) );
        REQUIRE( v.at( 1, 2 ) == -1.0 );
        REQUIRE( v( 1, 2 ) == v.at( 1, 2 ) );
    }
    SECTION( "row or column at the extent aborts with index" )
    {
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); m.at( 3, 0 ) = 1.0; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); m.at( 0, 4 ) = 1.0; }, "index" );
        S2_REQUIRE_DEATH( []{ auto const m = s2_r2::numbered( 3, 4 ); double volatile x = m.at( 3, 0 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto const m = s2_r2::numbered( 3, 4 ); double volatile x = m.at( 0, 4 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); m( 3, 0 ) = 1.0; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); m( 0, 4 ) = 1.0; }, "index" );
        S2_REQUIRE_DEATH( []{ auto const m = s2_r2::numbered( 3, 4 ); double volatile x = m( 3, 0 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto const m = s2_r2::numbered( 3, 4 ); double volatile x = m( 0, 4 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); m.at( std::size_t( -1 ), 0 ) = 1.0; }, "index" );
    }
    SECTION( "m[r] aborts when r >= row()" )
    {
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); double* volatile p = m[3]; (void)p; }, "index" );
        S2_REQUIRE_DEATH( []{ auto const m = s2_r2::numbered( 3, 4 ); double const* volatile p = m[3]; (void)p; }, "index" );
    }
    SECTION( "views check against their own extents" )
    {
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); view_type const v{ m, { 1, 3 }, { 1, 4 } }; double volatile x = v.at( 2, 0 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); view_type const v{ m, { 1, 3 }, { 1, 4 } }; double volatile x = v.at( 0, 3 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); view_type const v{ m, { 1, 3 }, { 1, 4 } }; double volatile x = v( 2, 0 ); (void)x; }, "index" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 3, 4 ); view_type const v{ m, { 1, 3 }, { 1, 4 } }; double volatile x = v( 0, 3 ); (void)x; }, "index" );
    }
}

TEST_CASE( "S2 empty matrices have no accessible elements", "[S2][S2-R2]" )
{
    std::pair< std::size_t, std::size_t > const shapes[] = { { 0, 0 }, { 0, 5 }, { 5, 0 } };
    for ( auto const& [r, c] : shapes )
    {
        INFO( "shape " << r << "x" << c );
        feng::matrix<double> m{ r, c };
        REQUIRE( m.row() == r );
        REQUIRE( m.col() == c );
        REQUIRE( m.size() == 0 );
        REQUIRE( m.begin() == m.end() );
        auto const& cm = m;
        REQUIRE( cm.begin() == cm.end() );
        std::pair< std::size_t, std::size_t > const probes[] = { { 0, 0 }, { 0, 4 }, { 4, 0 }, { 4, 4 }, { 5, 5 } };
        for ( auto const& [pr, pc] : probes )
        {
            INFO( "probe (" << pr << ", " << pc << ")" );
            S2_REQUIRE_DEATH( [&]{ feng::matrix<double> e{ r, c }; e.at( pr, pc ) = 1.0; }, "index" );
            S2_REQUIRE_DEATH( [&]{ feng::matrix<double> const e{ r, c }; double volatile x = e.at( pr, pc ); (void)x; }, "index" );
            S2_REQUIRE_DEATH( [&]{ feng::matrix<double> e{ r, c }; e( pr, pc ) = 1.0; }, "index" );
            S2_REQUIRE_DEATH( [&]{ feng::matrix<double> const e{ r, c }; double volatile x = e( pr, pc ); (void)x; }, "index" );
        }
    }
}

TEST_CASE( "S2 negative signed dimensions abort before allocation", "[S2][S2-R2]" )
{
    using s2_r2::counted;
    SECTION( "non-negative signed dimensions build the matrix" )
    {
        int const r = 2, c = 3;
        feng::matrix<double> m{ r, c, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        REQUIRE( m.row() == 2 );
        REQUIRE( m.col() == 3 );
        REQUIRE( m( 1, 2 ) == 6.0 );
    }
    SECTION( "a negative row or column aborts with size" )
    {
        s2_r2::require_death_without_allocate( []{ int volatile r = -1; s2_r2::ready(); counted m{ int( r ), 2, { 1.0 } }; (void)m; }, "size" );
        s2_r2::require_death_without_allocate( []{ int volatile c = -3; s2_r2::ready(); counted m{ 2, int( c ), { 1.0 } }; (void)m; }, "size" );
        s2_r2::require_death_without_allocate( []{ long long volatile r = -1; s2_r2::ready(); counted m{ (long long)( r ), 0LL, {} }; (void)m; }, "size" );
        s2_r2::require_death_without_allocate( []{ short volatile c = -1; s2_r2::ready(); counted m{ 0, (short)( c ), {} }; (void)m; }, "size" );
    }
}

TEST_CASE( "S2 reshape checks the element count and overflow", "[S2][S2-R2]" )
{
    SECTION( "a matching reshape keeps the data" )
    {
        auto m = s2_r2::numbered( 2, 6 );
        double const* const p = m.data();
        m.reshape( 3, 4 );
        REQUIRE( m.row() == 3 );
        REQUIRE( m.col() == 4 );
        REQUIRE( m.data() == p );
        REQUIRE( m( 2, 3 ) == 11.0 );
    }
    SECTION( "a different element count aborts with reshape" )
    {
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 2, 6 ); m.reshape( 4, 4 ); }, "reshape" );
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 2, 6 ); m.reshape( 0, 12 ); }, "reshape" );
    }
    SECTION( "a wrapping r*c aborts with reshape" )
    {
        // 2^32 * 2^32 wraps to 0 and 2^63 * 2 wraps to 0: both equal an empty matrix's size() unless checked.
        S2_REQUIRE_DEATH( []{ feng::matrix<double> m; m.reshape( std::size_t{ 1 } << 32, std::size_t{ 1 } << 32 ); }, "reshape" );
        S2_REQUIRE_DEATH( []{ feng::matrix<double> m{ 0, 3 }; m.reshape( std::size_t{ 1 } << 63, 2 ); }, "reshape" );
        // (2^64 - 1) * 13 wraps to 2^64 - 13; 12 + 2^64 wraps to 12 for r*c = 12 via r = 2^62 + 3, c = 4.
        S2_REQUIRE_DEATH( []{ auto m = s2_r2::numbered( 2, 6 ); m.reshape( ( std::size_t{ 1 } << 62 ) + 3, 4 ); }, "reshape" );
    }
}

TEST_CASE( "S2 clone bounds abort before reading the source", "[S2][S2-R2]" )
{
    using s2_r2::counted;
    using s2_r2::numbered_counted;
    using s2_r2::require_death_without_allocate;
    using s2_r2::ready;

    SECTION( "an in-bounds clone copies the block" )
    {
        auto const src = s2_r2::numbered( 4, 5 );
        feng::matrix<double> a;
        a.clone( src, 1, 4, 2, 5 );
        REQUIRE( a.row() == 3 );
        REQUIRE( a.col() == 3 );
        REQUIRE( a( 0, 0 ) == src( 1, 2 ) );
        REQUIRE( a( 2, 2 ) == src( 3, 4 ) );
        auto const b = src.clone( { 0, 4 }, { 0, 5 } );
        REQUIRE( b == src );
        feng::matrix<double> const c{ src, { 3, 4 }, { 4, 5 } };
        REQUIRE( c.size() == 1 );
        REQUIRE( c( 0, 0 ) == 19.0 );
    }
    SECTION( "member clone( other, r0, r1, c0, c1 )" )
    {
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 0, 5, 0, 5 ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 0, 4, 0, 6 ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 2, 2, 0, 5 ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 3, 1, 0, 5 ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 0, 4, 5, 5 ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 0, 4, 4, 2 ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, 0, std::size_t( -1 ), 0, 5 ); }, "clone" );
    }
    SECTION( "member clone with brace lists" )
    {
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, { 0, 5 }, { 0, 5 } ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, { 0, 1, 2 }, { 0, 5 } ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, { 0, 4 }, { 1 } ); }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); counted d; ready(); d.clone( s, {}, { 0, 5 } ); }, "clone" );
    }
    SECTION( "const clone( r0, r1, c0, c1 ) and clone( {r0, r1}, {c0, c1} )" )
    {
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); auto d = s.clone( 0, 5, 0, 5 ); (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); auto d = s.clone( 1, 1, 0, 5 ); (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); auto d = s.clone( { 0, 4 }, { 0, 6 } ); (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); auto d = s.clone( { 0 }, { 0, 5 } ); (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); auto d = s.clone( { 0, 4 }, { 0, 1, 5 } ); (void)d; }, "clone" );
    }
    SECTION( "slicing constructors" )
    {
        using range = std::pair< std::size_t, std::size_t >;
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); counted d{ s, { 0, 5 }, { 0, 5 } }; (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); counted d{ s, { 0, 4, 1 }, { 0, 5 } }; (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); counted d{ s, { 0, 4 }, { 5 } }; (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); counted d{ s, range{ 0, 4 }, range{ 3, 3 } }; (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); counted d{ s, 0, 4, 0, 9 }; (void)d; }, "clone" );
        require_death_without_allocate( []{ auto const s = numbered_counted( 4, 5 ); ready(); counted d{ s, 2, 1, 0, 5 }; (void)d; }, "clone" );
        // the converting (other element type) slicing constructor
        S2_REQUIRE_DEATH( []{ feng::matrix<float> const s{ 4, 5 }; feng::matrix<double> d{ s, { 0, 5 }, { 0, 5 } }; (void)d; }, "clone" );
        S2_REQUIRE_DEATH( []{ feng::matrix<float> const s{ 4, 5 }; feng::matrix<double> d{ s, { 0, 4 }, { 0 } }; (void)d; }, "clone" );
    }
}

TEST_CASE( "S2 byte counts above PTRDIFF_MAX abort before allocation", "[S2][S2-R2]" )
{
    using s2_r2::counted;
    // The counting allocator keeps the default max_size (SIZE_MAX / sizeof(T)), so only the PTRDIFF_MAX bound
    // stands between 2^60 doubles (2^63 bytes) and the allocator.
    s2_r2::require_death_without_allocate( []{ s2_r2::ready(); counted m{ std::size_t{ 1 } << 60, 1 }; (void)m; }, "PTRDIFF_MAX" );
    s2_r2::require_death_without_allocate( []{ s2_r2::ready(); counted m{ s2_r2::counting_allocator<double>{}, 1, std::size_t{ 1 } << 61 }; (void)m; }, "size" );
    S2_REQUIRE_DEATH( []{ feng::matrix<char> m{ std::size_t{ 1 } << 63, 1 }; (void)m; }, "PTRDIFF_MAX" );
    S2_REQUIRE_DEATH( []{ feng::matrix<char> m{ std::size_t{ 1 } << 63, 1 }; (void)m; }, "size" );
}

TEST_CASE( "S2 resize and shrink_to_size check sizes before allocation", "[S2][S2-R2]" )
{
    using s2_r2::counted;
    // 2^32 x 2^32 wraps to 0, which equals an empty matrix's size(); it must not be taken as a reshape.
    s2_r2::require_death_without_allocate( []{ counted m; s2_r2::ready(); m.resize( std::size_t{ 1 } << 32, std::size_t{ 1 } << 32 ); }, "size" );
    s2_r2::require_death_without_allocate( []{ counted m{ 2, 2 }; s2_r2::ready(); m.resize( std::size_t{ 1 } << 62, 8 ); }, "size" );
    s2_r2::require_death_without_allocate( []{ counted m{ 2, 2 }; s2_r2::ready(); m.shrink_to_size( std::size_t{ 1 } << 62, 8 ); }, "size" );
    s2_r2::require_death_without_allocate( []{ counted m{ 2, 2 }; s2_r2::ready(); m.shrink_to_size( std::size_t{ 1 } << 60, 1 ); }, "PTRDIFF_MAX" );
}

TEST_CASE( "S2 death helper isolates the child", "[S2][S2-R2]" )
{
    SECTION( "the child runs with SIG_DFL for SIGABRT" )
    {
        auto const out = s2_death::run( []{ struct sigaction sa{}; ::sigaction( SIGABRT, nullptr, &sa ); if ( sa.sa_handler != SIG_DFL ) ::_exit( 4 ); } );
        REQUIRE( out.exited );
        REQUIRE( out.exit_code == 0 );
    }
    SECTION( "the child's stdout is /dev/null" )
    {
        auto const out = s2_death::run( []
        {
            struct stat o{}, n{};
            if ( ::fstat( STDOUT_FILENO, &o ) != 0 || ::stat( "/dev/null", &n ) != 0 ) ::_exit( 5 );
            if ( o.st_rdev != n.st_rdev || !S_ISCHR( o.st_mode ) ) ::_exit( 3 );
        } );
        REQUIRE( out.exited );
        REQUIRE( out.exit_code == 0 );
    }
    SECTION( "an abort prints only the contract message" )
    {
        auto const out = s2_death::run( []{ feng::matrix<double> m{ 1, 1 }; m.at( 1, 1 ) = 0.0; } );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( s2_death::line_count( out.err ) == 1 );
        REQUIRE( out.err.rfind( "feng::matrix: contract violation: ", 0 ) == 0 );
    }
#if defined( __cpp_exceptions )
    SECTION( "a throwing callable exits with code 2" )
    {
        auto const out = s2_death::run( []{ throw std::runtime_error( "s2 death helper" ); } );
        REQUIRE( out.exited );
        REQUIRE( out.exit_code == 2 );
    }
#endif
}
