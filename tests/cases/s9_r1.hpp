// S9-R1 (PR-12): plain value returns. Factory and shape results are non-const prvalues, so assigning them to an
// existing matrix or move-constructing from them moves the storage instead of copying it (one allocation per
// result, counted with s9_r1::counting_alloc); magic<T, A>( n ) builds in T and A and keeps magic( n )'s values.
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>

namespace s9_r1
{
    struct counts
    {
        static inline long allocations = 0;
        static inline long deallocations = 0;
        static void reset() noexcept { allocations = 0; deallocations = 0; }
    };

    template < typename T >
    struct counting_alloc
    {
        using value_type = T;
        counting_alloc() noexcept = default;
        template < typename U >
        counting_alloc( counting_alloc< U > const& ) noexcept {}
        T* allocate( std::size_t n )
        {
            ++counts::allocations;
            return std::allocator< T >{}.allocate( n );
        }
        void deallocate( T* p, std::size_t n ) noexcept
        {
            ++counts::deallocations;
            std::allocator< T >{}.deallocate( p, n );
        }
        template < typename U >
        friend bool operator == ( counting_alloc const&, counting_alloc< U > const& ) noexcept { return true; }
    };

    using cmat = feng::matrix< double, counting_alloc< double > >;

    // decltype of each call is its declared return type; none may be const-qualified.
    inline void const_free_returns() noexcept
    {
        feng::matrix< double > const m{ 3, 3, 1.0 };
        std::mt19937_64 g{ 1 };
        static_assert( !std::is_const_v< decltype( feng::eye< double >( 3 ) ) > );
        static_assert( !std::is_const_v< decltype( feng::zeros< double >( 3, 3 ) ) > );
        static_assert( !std::is_const_v< decltype( feng::ones< double >( 3, 3 ) ) > );
        static_assert( !std::is_const_v< decltype( feng::magic( 4 ) ) > );
        static_assert( !std::is_const_v< decltype( feng::magic< double, counting_alloc< double > >( 4 ) ) > );
        static_assert( !std::is_const_v< decltype( feng::transpose( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::inverse( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::diag( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::tril( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::triu( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::fliplr( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::flipud( m ) ) > );
        static_assert( !std::is_const_v< decltype( feng::rand( 3, 3, g ) ) > );
        static_assert( !std::is_const_v< decltype( m.transpose() ) > );
        static_assert( !std::is_const_v< decltype( m.inverse() ) > );
        static_assert( !std::is_const_v< decltype( m.clone( 0, 2, 0, 2 ) ) > );
        static_assert( !std::is_const_v< decltype( m.astype< float >() ) > );
    }
}

TEST_CASE( "S9-R1 factory results are moved, not copied", "[S9][S9-R1]" )
{
    using s9_r1::cmat;
    using s9_r1::counts;
    s9_r1::const_free_returns();
    s9_r1::counting_alloc< double > const alloc;

    SECTION( "magic<double, counting_alloc<double>> assigned to an existing matrix" )
    {
        for ( std::uint_least64_t n : { 3u, 4u, 5u, 6u, 8u, 10u } )
        {
            cmat m{ 2, 2 };
            counts::reset();
            m = feng::magic< double, s9_r1::counting_alloc< double > >( n );
            REQUIRE( counts::allocations == 1 );
            REQUIRE( counts::deallocations == 1 ); // the old 2x2 storage only
            auto const ref = feng::magic( n );
            REQUIRE( m.row() == n );
            REQUIRE( m.col() == n );
            for ( std::uint_least64_t r = 0; r != n; ++r )
                for ( std::uint_least64_t c = 0; c != n; ++c )
                    REQUIRE( m[r][c] == static_cast< double >( ref[r][c] ) );
        }
    }

    SECTION( "move construction takes the storage" )
    {
        counts::reset();
        auto y = feng::magic< double, s9_r1::counting_alloc< double > >( 5 );
        REQUIRE( counts::allocations == 1 );
        double const* const data = y.data();
        cmat x( std::move( y ) );
        REQUIRE( counts::allocations == 1 );
        REQUIRE( x.data() == data );
        REQUIRE( x[2][2] == 13.0 );
    }

    SECTION( "factory and shape results initialise and assign with one allocation each" )
    {
        counts::reset();
        auto z = feng::zeros< double >( alloc, 3, 4 );
        REQUIRE( counts::allocations == 1 );
        auto o = feng::ones< double >( alloc, 4, 3 );
        REQUIRE( counts::allocations == 2 );
        cmat t{ 1, 1 };
        long const before = counts::allocations;
        t = z.transpose();
        REQUIRE( counts::allocations == before + 1 );
        t = feng::transpose( o );
        REQUIRE( counts::allocations == before + 2 );
        cmat c( z.clone( 0, 2, 0, 2 ) );
        REQUIRE( counts::allocations == before + 3 );
        REQUIRE( c.row() == 2 );
        REQUIRE( c.col() == 2 );
        REQUIRE( t.row() == 3 );
        REQUIRE( t.col() == 4 );
        REQUIRE( t[0][0] == 1.0 );
    }
}
