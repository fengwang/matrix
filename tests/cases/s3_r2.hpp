// S3-R2 (PR-5): element lifetimes and stateful allocators (R04).
#include "./s3_alloc.hpp"

#include <cstddef>
#include <memory>
#include <utility>

namespace s3_r2
{
    template < bool POCCA, bool POCMA, bool POCS >
    using tracked_matrix = feng::matrix< s3_alloc::tracked, s3_alloc::tracking_allocator< s3_alloc::tracked, POCCA, POCMA, POCS > >;

    using s3_alloc::require_balanced;

    // Copy-assign a 3x4 matrix on resource 1 into a 2x5 matrix on resource 2 (unequal allocators), then destroy both.
    template < bool POCCA, bool POCMA, bool POCS >
    void copy_assign_unequal()
    {
        typedef tracked_matrix< POCCA, POCMA, POCS > matrix_type;
        typedef typename matrix_type::allocator_type allocator_type;
        s3_alloc::reset();
        {
            matrix_type const src{ allocator_type{ 1 }, 3, 4, s3_alloc::tracked{ 7 } };
            matrix_type dst{ allocator_type{ 2 }, 2, 5, s3_alloc::tracked{ 9 } };
            REQUIRE( src.get_allocator() != dst.get_allocator() );
            dst = src;
            REQUIRE( dst.get_allocator().id == ( POCCA ? 1 : 2 ) );
            REQUIRE( src.get_allocator().id == 1 );
            REQUIRE( dst.row() == 3 );
            REQUIRE( dst.col() == 4 );
            REQUIRE( dst.size() == 12 );
            for ( auto const& e : dst ) REQUIRE( e.value == 7 );
            REQUIRE( s3_alloc::registry::live( 1 ) == ( POCCA ? 2 : 1 ) );
            REQUIRE( s3_alloc::registry::live( 2 ) == ( POCCA ? 0 : 1 ) );
        }
        REQUIRE( s3_alloc::registry::live( 1 ) == 0 );
        REQUIRE( s3_alloc::registry::live( 2 ) == 0 );
        require_balanced();
    }
}

TEST_CASE( "S3 unequal non-propagating copy-assign frees each resource's own allocations", "[S3][S3-R2]" )
{
    SECTION( "POCMA false, POCS false" ) { s3_r2::copy_assign_unequal< false, false, false >(); }
    SECTION( "POCMA false, POCS true" )  { s3_r2::copy_assign_unequal< false, false, true >(); }
    SECTION( "POCMA true, POCS false" )  { s3_r2::copy_assign_unequal< false, true, false >(); }
    SECTION( "POCMA true, POCS true" )   { s3_r2::copy_assign_unequal< false, true, true >(); }
}

namespace s3_r2
{
    // Fills m with base, base+1, ... in row order.
    template < typename M >
    void fill_from( M& m, int base )
    {
        for ( std::size_t i = 0; i != m.size(); ++i ) m.data()[i].value = base + static_cast< int >( i );
    }

    // m is r x c with size r*c, end-begin == size, and holds base, base+1, ... in row order.
    template < typename M >
    void require_holds( M const& m, std::size_t r, std::size_t c, int base )
    {
        REQUIRE( m.row() == r );
        REQUIRE( m.col() == c );
        REQUIRE( m.size() == r * c );
        REQUIRE( static_cast< std::size_t >( m.end() - m.begin() ) == m.size() );
        int expected = base;
        for ( auto const& e : m ) REQUIRE( e.value == expected++ );
        REQUIRE( expected == base + static_cast< int >( r * c ) );
    }

    template < typename M >
    void require_moved_from( M const& m )
    {
        REQUIRE( m.row() == 0 );
        REQUIRE( m.col() == 0 );
        REQUIRE( m.size() == 0 );
        REQUIRE( m.begin() == m.end() );
    }

    // Goes through references so the compiler sees no direct self-assignment.
    template < typename M > void self_copy_assign( M& m, M const& alias ) { m = alias; }
    template < typename M > void self_move_assign( M& m, M& alias ) { m = std::move( alias ); }

    // a: 3x4 on resource 1, holding 0..11; b: 2x5 on resource `other`, holding 100..109. Every scenario runs in
    // its own scope and ends with require_balanced().
    template < bool POCCA, bool POCMA, bool POCS >
    void every_operation( int const other )
    {
        typedef tracked_matrix< POCCA, POCMA, POCS > matrix_type;
        typedef typename matrix_type::allocator_type allocator_type;
        bool const equal = other == 1;
        auto make_a = []{ matrix_type m{ allocator_type{ 1 }, 3, 4 }; fill_from( m, 0 ); return m; };
        auto make_b = [other]{ matrix_type m{ allocator_type{ other }, 2, 5 }; fill_from( m, 100 ); return m; };

        SECTION( "copy-construct" )
        {
            s3_alloc::reset();
            {
                auto const a = make_a();
                matrix_type c{ a };
                REQUIRE( c.get_allocator() == std::allocator_traits< allocator_type >::select_on_container_copy_construction( a.get_allocator() ) );
                REQUIRE( c.get_allocator().id == 1 );
                require_holds( c, 3, 4, 0 );
                require_holds( a, 3, 4, 0 );
            }
            require_balanced();
        }
        SECTION( "copy-assign" )
        {
            s3_alloc::reset();
            {
                auto const a = make_a();
                auto b = make_b();
                b = a;
                REQUIRE( b.get_allocator().id == ( POCCA ? 1 : other ) );
                REQUIRE( a.get_allocator().id == 1 );
                require_holds( b, 3, 4, 0 );
                require_holds( a, 3, 4, 0 );
            }
            require_balanced();
        }
        SECTION( "move-construct" )
        {
            s3_alloc::reset();
            {
                auto a = make_a();
                matrix_type c{ std::move( a ) };
                REQUIRE( c.get_allocator().id == 1 );
                require_holds( c, 3, 4, 0 );
                require_moved_from( a );
            }
            require_balanced();
        }
        SECTION( "move-assign" )
        {
            s3_alloc::reset();
            {
                auto a = make_a();
                auto b = make_b();
                b = std::move( a );
                REQUIRE( b.get_allocator().id == ( POCMA ? 1 : other ) );
                require_holds( b, 3, 4, 0 );
                require_moved_from( a );
                a = make_a(); // a moved-from matrix is usable again
                require_holds( a, 3, 4, 0 );
            }
            require_balanced();
        }
        SECTION( "member swap" )
        {
            s3_alloc::reset();
            {
                auto a = make_a();
                auto b = make_b();
                a.swap( b );
                REQUIRE( a.get_allocator().id == ( POCS ? other : 1 ) );
                REQUIRE( b.get_allocator().id == ( POCS ? 1 : other ) );
                require_holds( a, 2, 5, 100 );
                require_holds( b, 3, 4, 0 );
                if ( !equal )
                {
                    REQUIRE( s3_alloc::registry::live( 1 ) == 1 );
                    REQUIRE( s3_alloc::registry::live( other ) == 1 );
                }
            }
            require_balanced();
        }
        SECTION( "free swap" )
        {
            s3_alloc::reset();
            {
                auto a = make_a();
                auto b = make_b();
                using std::swap;
                swap( a, b );
                REQUIRE( a.get_allocator().id == ( POCS ? other : 1 ) );
                REQUIRE( b.get_allocator().id == ( POCS ? 1 : other ) );
                require_holds( a, 2, 5, 100 );
                require_holds( b, 3, 4, 0 );
            }
            require_balanced();
        }
        SECTION( "self-copy-assign" )
        {
            s3_alloc::reset();
            {
                auto a = make_a();
                self_copy_assign( a, a );
                REQUIRE( a.get_allocator().id == 1 );
                require_holds( a, 3, 4, 0 );
            }
            require_balanced();
        }
        SECTION( "self-move-assign" )
        {
            s3_alloc::reset();
            {
                auto a = make_a();
                self_move_assign( a, a );
                REQUIRE( a.get_allocator().id == 1 );
                require_holds( a, 3, 4, 0 );
            }
            require_balanced();
        }
    }

    template < bool POCCA, bool POCMA, bool POCS >
    void every_resource_pair()
    {
        SECTION( "equal resources" )   { every_operation< POCCA, POCMA, POCS >( 1 ); }
        SECTION( "unequal resources" ) { every_operation< POCCA, POCMA, POCS >( 2 ); }
    }
}

TEST_CASE( "S3 every propagation combination balances each resource", "[S3][S3-R2]" )
{
    SECTION( "POCCA 0, POCMA 0, POCS 0" ) { s3_r2::every_resource_pair< false, false, false >(); }
    SECTION( "POCCA 0, POCMA 0, POCS 1" ) { s3_r2::every_resource_pair< false, false, true >(); }
    SECTION( "POCCA 0, POCMA 1, POCS 0" ) { s3_r2::every_resource_pair< false, true, false >(); }
    SECTION( "POCCA 0, POCMA 1, POCS 1" ) { s3_r2::every_resource_pair< false, true, true >(); }
    SECTION( "POCCA 1, POCMA 0, POCS 0" ) { s3_r2::every_resource_pair< true, false, false >(); }
    SECTION( "POCCA 1, POCMA 0, POCS 1" ) { s3_r2::every_resource_pair< true, false, true >(); }
    SECTION( "POCCA 1, POCMA 1, POCS 0" ) { s3_r2::every_resource_pair< true, true, false >(); }
    SECTION( "POCCA 1, POCMA 1, POCS 1" ) { s3_r2::every_resource_pair< true, true, true >(); }
}
