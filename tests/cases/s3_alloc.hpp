// S3-R2 (PR-5): test helpers for element lifetimes and stateful allocators (R04).
// - s3_alloc::tracked: a nothrow element type that counts live objects, constructions and destructions.
// - s3_alloc::registry: per resource id, the set of live allocations; a deallocation of a pointer the resource did
//   not allocate (or with a different count) is recorded as a failure that the tests check.
// - s3_alloc::tracking_allocator<T, POCCA, POCMA, POCS>: stateful (a resource id), is_always_equal false, == compares
//   ids; the three propagation traits are template parameters so a test can loop over all 8 combinations.
#ifndef S3_ALLOC_HPP_INCLUDED
#define S3_ALLOC_HPP_INCLUDED

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace s3_alloc
{
    struct tracked
    {
        static inline long live = 0;
        static inline long constructed = 0;
        static inline long destroyed = 0;

        static void reset() noexcept { live = 0; constructed = 0; destroyed = 0; }

        int value = 0;

        tracked() noexcept { ++live; ++constructed; }
        tracked( int v ) noexcept : value{ v } { ++live; ++constructed; }
        tracked( tracked const& other ) noexcept : value{ other.value } { ++live; ++constructed; }
        tracked( tracked&& other ) noexcept : value{ other.value } { ++live; ++constructed; }
        tracked& operator = ( tracked const& other ) noexcept { value = other.value; return *this; }
        tracked& operator = ( tracked&& other ) noexcept { value = other.value; return *this; }
        ~tracked() noexcept { --live; ++destroyed; }

        friend bool operator == ( tracked const& a, tracked const& b ) noexcept { return a.value == b.value; }
    };

    struct registry
    {
        // resource id -> (pointer -> element count) of its live allocations
        static inline std::map< int, std::map< void const*, std::size_t > > live_allocations;
        static inline std::vector< std::string > failures;
        static inline long allocations = 0;

        static void reset() { live_allocations.clear(); failures.clear(); allocations = 0; }

        static std::size_t live( int id )
        {
            auto const it = live_allocations.find( id );
            return it == live_allocations.end() ? 0 : it->second.size();
        }

        static std::size_t live_total()
        {
            std::size_t n = 0;
            for ( auto const& [id, ptrs] : live_allocations ) n += ptrs.size();
            return n;
        }

        static void on_allocate( int id, void const* p, std::size_t n )
        {
            ++allocations;
            live_allocations[id][p] = n;
        }

        // Returns true when the pointer is live on `id` with count `n`; otherwise records a failure and forgets the
        // pointer wherever it is live, so the memory is still released exactly once.
        static bool on_deallocate( int id, void const* p, std::size_t n )
        {
            auto& mine = live_allocations[id];
            auto const it = mine.find( p );
            if ( it != mine.end() && it->second == n )
            {
                mine.erase( it );
                return true;
            }
            failures.push_back( "resource " + std::to_string( id ) + " deallocated a pointer it did not allocate (count " + std::to_string( n ) + ")" );
            for ( auto& [other, ptrs] : live_allocations ) ptrs.erase( p );
            return false;
        }
    };

    template < typename T, bool POCCA, bool POCMA, bool POCS >
    struct tracking_allocator
    {
        typedef T value_type;
        typedef std::bool_constant< POCCA > propagate_on_container_copy_assignment;
        typedef std::bool_constant< POCMA > propagate_on_container_move_assignment;
        typedef std::bool_constant< POCS > propagate_on_container_swap;
        typedef std::false_type is_always_equal;

        template < typename U >
        struct rebind { typedef tracking_allocator< U, POCCA, POCMA, POCS > other; };

        int id = 0;

        tracking_allocator() noexcept = default;
        explicit tracking_allocator( int resource ) noexcept : id{ resource } {}
        template < typename U >
        tracking_allocator( tracking_allocator< U, POCCA, POCMA, POCS > const& other ) noexcept : id{ other.id } {}

        T* allocate( std::size_t n )
        {
            T* const p = std::allocator< T >{}.allocate( n );
            registry::on_allocate( id, p, n );
            return p;
        }

        void deallocate( T* p, std::size_t n ) noexcept
        {
            registry::on_deallocate( id, p, n );
            std::allocator< T >{}.deallocate( p, n );
        }

        friend bool operator == ( tracking_allocator const& a, tracking_allocator const& b ) noexcept { return a.id == b.id; }
    };

    // Resets the registry and the tracked counters; call at the start of each test.
    inline void reset()
    {
        registry::reset();
        tracked::reset();
    }

    // Every allocation released by the resource that made it, every element destroyed exactly once.
    // Uses Catch macros, so include this header after catch.hpp.
    inline void require_balanced()
    {
        INFO( "first failure: " << ( registry::failures.empty() ? std::string{ "none" } : registry::failures.front() ) );
        REQUIRE( registry::failures.empty() );
        REQUIRE( registry::live_total() == 0 );
        REQUIRE( tracked::live == 0 );
        REQUIRE( tracked::constructed == tracked::destroyed );
    }
}

#endif // S3_ALLOC_HPP_INCLUDED
