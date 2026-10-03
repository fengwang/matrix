// S3-R5 (PR-2): noexcept special members and the allocation check before allocating.
#include "./s2_death.hpp"

#include <type_traits>
#include <utility>

namespace s3_r5
{
    typedef feng::matrix< double > md;
    static_assert( std::is_nothrow_default_constructible_v< md > );
    static_assert( std::is_nothrow_copy_constructible_v< md > );
    static_assert( std::is_nothrow_move_constructible_v< md > );
    static_assert( std::is_nothrow_copy_assignable_v< md > );
    static_assert( std::is_nothrow_move_assignable_v< md > );
    static_assert( std::is_nothrow_destructible_v< md > );
    static_assert( noexcept( std::declval< md& >().swap( std::declval< md& >() ) ) );
    static_assert( std::is_nothrow_swappable_v< md > );
}

TEST_CASE( "S3 huge resize aborts before allocating", "[S3][S3-R5]" )
{
    S2_REQUIRE_DEATH( []{ feng::matrix<double> m; m.resize( 1ULL << 61, 8 ); }, "size" );
}

// S3-R5 (D-012): every public API family is noexcept; one or more calls per S1 API family (docs/stages/S1/spec.md),
// plus the special members, swap, resize, clone, astype, I/O and callback-taking functions. String arguments are
// passed as lvalues and defaulted string parameters explicitly: building a std::string at the call site can throw,
// which is the caller's expression, not the library's.
namespace s3_r5_api
{
    using std::declval;
    typedef feng::matrix< double > md;
    typedef feng::matrix< std::complex< double > > mc;
    typedef feng::matrix< float, std::allocator< float > > mf;
    typedef feng::matrix_view< double, std::allocator< double > > vd;
    inline auto const unary = []( double& x ) noexcept { x += 1.0; };

    // construction
    static_assert( noexcept( md{} ) );
    static_assert( noexcept( md{ 3, 4 } ) );
    static_assert( noexcept( md{ 3, 4, 1.0 } ) );
    static_assert( noexcept( md{ declval< md const& >() } ) );
    static_assert( noexcept( md{ declval< md&& >() } ) );
    static_assert( noexcept( feng::zeros< double >( 3, 4 ) ) );
    static_assert( noexcept( feng::ones< double >( 3 ) ) );
    static_assert( noexcept( feng::eye< double >( 3 ) ) );
    static_assert( noexcept( feng::arange< double >( 0, 4 ) ) );
    static_assert( noexcept( feng::linspace( 0.0, 1.0 ) ) );
    static_assert( noexcept( feng::magic( 4 ) ) );
    static_assert( noexcept( feng::hilbert< double >( 3 ) ) );
    static_assert( noexcept( feng::rand< double >( 3 ) ) );
    static_assert( noexcept( feng::zeros_like( declval< md const& >() ) ) );
    static_assert( noexcept( feng::blkdiag( declval< md const& >(), declval< md const& >() ) ) );
    static_assert( noexcept( feng::make_diag( declval< md const& >() ) ) );
    // access
    static_assert( noexcept( declval< md& >()[ 0 ] ) );
    static_assert( noexcept( declval< md& >()[ 0 ][ 0 ] ) );
    static_assert( noexcept( declval< md const& >().item() ) );
    static_assert( noexcept( declval< md& >().data() ) );
    static_assert( noexcept( declval< md const& >().shape() ) );
    static_assert( noexcept( declval< md const& >().size() ) );
    static_assert( noexcept( declval< md& >().begin() ) );
    static_assert( noexcept( declval< md& >().row_begin( 0 ) ) );
    static_assert( noexcept( declval< md& >().col_begin( 0 ) ) );
    static_assert( noexcept( declval< md& >().diag_begin() ) );
    // views
    static_assert( noexcept( feng::make_view( declval< md const& >(), { 0, 1 }, { 0, 1 } ) ) );
    static_assert( noexcept( declval< md const& >().clone( 0, 1, 0, 1 ) ) );
    static_assert( noexcept( declval< md& >().clone( declval< md const& >(), 0, 1, 0, 1 ) ) );
    static_assert( noexcept( md{ declval< vd const& >() } ) );
    // shape
    static_assert( noexcept( declval< md& >().reshape( 2, 2 ) ) );
    static_assert( noexcept( declval< md& >().resize( 2, 2 ) ) );
    static_assert( noexcept( declval< md const& >().transpose() ) );
    static_assert( noexcept( feng::transpose( declval< md const& >() ) ) );
    static_assert( noexcept( feng::fliplr( declval< md const& >() ) ) );
    static_assert( noexcept( feng::tril( declval< md const& >() ) ) );
    static_assert( noexcept( declval< md& >().clear() ) );
    static_assert( noexcept( declval< md& >().shrink_to_size( 1, 1 ) ) );
    // arithmetic
    static_assert( noexcept( declval< md const& >() + declval< md const& >() ) );
    static_assert( noexcept( declval< md const& >() - 1.0 ) );
    static_assert( noexcept( 2.0 * declval< md const& >() ) );
    static_assert( noexcept( declval< md const& >() * declval< md const& >() ) );
    static_assert( noexcept( declval< md const& >() / 2.0 ) );
    static_assert( noexcept( declval< md& >() += 1.0 ) );
    static_assert( noexcept( -declval< md const& >() ) );
    static_assert( noexcept( declval< md const& >() ^ 2 ) );
    // elementwise
    static_assert( noexcept( feng::sin( declval< md const& >() ) ) );
    static_assert( noexcept( feng::abs( declval< md const& >() ) ) );
    static_assert( noexcept( feng::pow( declval< md const& >(), 2.0 ) ) );
    static_assert( noexcept( feng::real( declval< mc const& >() ) ) );
    static_assert( noexcept( feng::clip( 0.0, 1.0 )( declval< md const& >() ) ) );
    static_assert( noexcept( declval< md const& >().astype< float >() ) );
    // reductions
    static_assert( noexcept( feng::sum( declval< md const& >() ) ) );
    static_assert( noexcept( feng::mean( declval< md const& >() ) ) );
    static_assert( noexcept( feng::variance( declval< md const& >() ) ) );
    static_assert( noexcept( feng::max( declval< md const& >() ) ) );
    static_assert( noexcept( declval< md const& >().minmax() ) );
    static_assert( noexcept( feng::norm_1( declval< md const& >() ) ) );
    static_assert( noexcept( feng::tr( declval< md const& >() ) ) );
    static_assert( noexcept( feng::isequal( declval< md const& >(), declval< md const& >() ) ) );
    static_assert( noexcept( feng::is_symmetric( declval< md const& >() ) ) );
    // linalg
    static_assert( noexcept( declval< md const& >().det() ) );
    static_assert( noexcept( feng::det( declval< md const& >() ) ) );
    static_assert( noexcept( feng::inverse( declval< md const& >() ) ) );
    static_assert( noexcept( feng::lu_solver( declval< md const& >(), declval< md const& >() ) ) );
    static_assert( noexcept( feng::pinv( declval< md const& >() ) ) );
    static_assert( noexcept( feng::expm( declval< md const& >() ) ) );
    // signal
    static_assert( noexcept( feng::conv( declval< md const& >(), declval< md const& >() ) ) );
    static_assert( noexcept( feng::fft( declval< mc const& >() ) ) );
    static_assert( noexcept( feng::fftshift( declval< md const& >() ) ) );
    static_assert( noexcept( feng::pooling( declval< md const& >(), 2, declval< std::string const& >() ) ) );
    // io
    static_assert( noexcept( declval< md const& >().save_as_txt( "x" ) ) );
    static_assert( noexcept( declval< md& >().load_txt( "x" ) ) );
    static_assert( noexcept( declval< md const& >().save_as_binary( "x" ) ) );
    static_assert( noexcept( declval< md& >().load_binary( declval< std::string const& >() ) ) );
    static_assert( noexcept( declval< md& >().load_npy( "x" ) ) );
    static_assert( noexcept( declval< md const& >().save_as_npy( "x" ) ) );
    static_assert( noexcept( declval< std::ostream& >() << declval< md const& >() ) );
    static_assert( noexcept( declval< std::istream& >() >> declval< md& >() ) );
    static_assert( noexcept( feng::disp( declval< md const& >() ) ) );
    // image
    static_assert( noexcept( declval< md const& >().save_as_bmp( declval< std::string const& >(), declval< std::string const& >() ) ) );
    static_assert( noexcept( declval< md const& >().save_as_png( declval< std::string const& >(), declval< std::string const& >() ) ) );
    static_assert( noexcept( declval< md const& >().save_as_pgm( "x" ) ) );
    static_assert( noexcept( declval< md const& >().plot( declval< std::string const& >(), declval< std::string const& >() ) ) );
    static_assert( noexcept( feng::load_bmp( declval< std::string const& >() ) ) );
    static_assert( noexcept( feng::save_as_bmp( declval< std::string const& >(), declval< md const& >(), declval< std::string const& >() ) ) );
    // allocator
    static_assert( noexcept( mf{ std::allocator< float >{}, 2, 2 } ) );
    static_assert( noexcept( declval< md const& >().get_allocator() ) );
    // special members, swap, callbacks
    static_assert( noexcept( declval< md& >() = declval< md const& >() ) );
    static_assert( noexcept( declval< md& >() = declval< md&& >() ) );
    static_assert( noexcept( declval< md& >().~md() ) );
    static_assert( noexcept( declval< md& >().swap( declval< md& >() ) ) );
    static_assert( noexcept( swap( declval< md& >(), declval< md& >() ) ) );
    static_assert( noexcept( declval< md& >().apply( unary ) ) );
    static_assert( noexcept( feng::matrix_details::for_each( declval< double* >(), declval< double* >(), unary ) ) );
}

TEST_CASE( "S3 every public API family is noexcept", "[S3][S3-R5]" )
{
    // The namespace-scope static_asserts above are the check; this case records that they compiled.
    REQUIRE( noexcept( s3_r5_api::md{ 2, 2 } ) );
    REQUIRE( noexcept( std::declval< s3_r5_api::md& >().resize( 3, 3 ) ) );
}

// S3-R5 (D-012): the size check runs on the allocator that will own the storage, before allocate() is called.
// budget_allocator: stateful, max_size() = budget bytes / sizeof(T) (default budget 100 bytes: 12 doubles, 100
// chars); copy construction selects a default-budget allocator; no propagation; == compares budgets. Once armed,
// allocate() writes a marker to stderr, so a death test can show that the abort came before any allocation.
#include <cstdio>
#include <unistd.h>

namespace s3_r5_budget
{
    inline bool armed = false;
    inline char const marker[] = "budget_allocator::allocate called";

    template < typename T >
    struct budget_allocator
    {
        typedef T value_type;
        typedef std::false_type propagate_on_container_copy_assignment;
        typedef std::false_type propagate_on_container_move_assignment;
        typedef std::false_type propagate_on_container_swap;
        typedef std::false_type is_always_equal;

        std::size_t budget = 100;

        budget_allocator() noexcept = default;
        explicit budget_allocator( std::size_t b ) noexcept : budget{ b } {}
        template < typename U >
        budget_allocator( budget_allocator< U > const& other ) noexcept : budget{ other.budget } {}

        T* allocate( std::size_t n )
        {
            if ( armed ) { [[maybe_unused]] auto const w = ::write( STDERR_FILENO, marker, sizeof( marker ) - 1 ); }
            return std::allocator< T >{}.allocate( n );
        }
        void deallocate( T* p, std::size_t n ) noexcept { std::allocator< T >{}.deallocate( p, n ); }
        std::size_t max_size() const noexcept { return budget / sizeof( T ); }
        budget_allocator select_on_container_copy_construction() const noexcept { return budget_allocator{}; }

        template < typename U >
        friend bool operator == ( budget_allocator const& a, budget_allocator< U > const& b ) noexcept { return a.budget == b.budget; }
    };

    typedef budget_allocator< double > ad;
    typedef feng::matrix< double, ad > md;
    typedef feng::matrix< char, budget_allocator< char > > mc;
    static_assert( std::is_same_v< md::allocator_type::value_type, md::value_type > ); // the accepted value_type case

    // Dies with a "size" message, and the dying child never reached an armed allocate().
    template < typename Callable >
    void require_size_abort_before_allocate( Callable fn )
    {
        S2_REQUIRE_DEATH( fn, "size" );
        s2_death::outcome const out = s2_death::run( fn );
        INFO( "child stderr: " << out.err );
        REQUIRE( out.signaled );
        REQUIRE_FALSE( s2_death::contains( out.err, marker ) );
    }
}

TEST_CASE( "S3 copy-assign checks the size before allocating", "[S3][S3-R5]" )
{
    using namespace s3_r5_budget;

    SECTION( "the marker shows when allocate() runs" )
    {
        s2_death::outcome const out = s2_death::run( []{ armed = true; md m{ ad{ 1000 }, 2, 2 }; } );
        REQUIRE( out.exited );
        REQUIRE( s2_death::contains( out.err, marker ) );
    }
    SECTION( "copy assignment checks against the destination's allocator" )
    {
        require_size_abort_before_allocate( []{ md src{ ad{ 1000 }, 5, 5 }; md dst{ ad{ 100 } }; armed = true; dst = src; } );
    }
    SECTION( "copy construction checks against the selected allocator" )
    {
        require_size_abort_before_allocate( []{ md src{ ad{ 1000 }, 5, 5 }; armed = true; md dst{ src }; (void)dst; } );
    }
    SECTION( "converting construction checks against the rebound allocator" )
    {
        require_size_abort_before_allocate( []{ mc src{ budget_allocator< char >{ 100 }, 5, 5 }; armed = true; md dst{ src }; (void)dst; } );
    }
    SECTION( "astype checks against the rebound allocator" )
    {
        require_size_abort_before_allocate( []{ mc src{ budget_allocator< char >{ 100 }, 5, 5 }; armed = true; auto dst = src.astype< double >(); (void)dst; } );
    }
    SECTION( "move assignment between unequal allocators checks against the destination's" )
    {
        require_size_abort_before_allocate( []{ md src{ ad{ 1000 }, 5, 5 }; md dst{ ad{ 100 } }; armed = true; dst = std::move( src ); } );
    }
    SECTION( "element-wise swap checks against each side's allocator" )
    {
        require_size_abort_before_allocate( []{ md a{ ad{ 1000 }, 5, 5 }; md b{ ad{ 100 } }; armed = true; a.swap( b ); } );
    }
    SECTION( "resize checks against the matrix's allocator" )
    {
        require_size_abort_before_allocate( []{ md m{ ad{ 100 } }; armed = true; m.resize( 5, 5 ); } );
    }
    SECTION( "within budget every path still works" )
    {
        md src{ ad{ 1000 }, 3, 3, 2.0 };
        md dst{ ad{ 100 } };
        dst = src;
        REQUIRE( dst.size() == 9 );
        md copy{ src };
        REQUIRE( copy.get_allocator().budget == 100 );
        REQUIRE( copy[2][2] == 2.0 );
        auto const as_char = src.astype< char >();
        REQUIRE( as_char.size() == 9 );
    }
}
