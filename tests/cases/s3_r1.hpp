// S3-R1 (PR-5): private RAII storage and the size invariant.
#include "./s3_alloc.hpp"

#include <cstddef>
#include <utility>

namespace s3_r1
{
    using s3_alloc::require_balanced;

    template< typename M > concept has_public_row_field       = requires( M& m ) { m.row_; };
    template< typename M > concept has_public_col_field       = requires( M& m ) { m.col_; };
    template< typename M > concept has_public_dat_field       = requires( M& m ) { m.dat_; };
    template< typename M > concept has_public_allocator_field = requires( M& m ) { m.allocator_; };

    static_assert( !has_public_row_field< feng::matrix<double> > );
    static_assert( !has_public_col_field< feng::matrix<double> > );
    static_assert( !has_public_dat_field< feng::matrix<double> > );
    static_assert( !has_public_allocator_field< feng::matrix<double> > );
    static_assert( !has_public_row_field< feng::matrix<int> > && !has_public_dat_field< feng::matrix<int> > );

    template< typename M >
    void require_consistent( M const& m )
    {
        REQUIRE( m.size() == m.row() * m.col() );
        REQUIRE( static_cast< std::size_t >( m.end() - m.begin() ) == m.size() );
        std::size_t visited = 0;
        for ( auto const& v : m ) { (void)v; ++visited; }
        REQUIRE( visited == m.size() );
    }

    // Goes through a reference so the compiler sees no direct self-move.
    template< typename M >
    void self_move_assign( M& m, M& alias ) { m = std::move( alias ); }

    inline feng::matrix<double> filled( std::size_t r, std::size_t c )
    {
        feng::matrix<double> m{ r, c };
        for ( std::size_t i = 0; i != r * c; ++i ) m.data()[i] = static_cast<double>( i ) + 0.25;
        return m;
    }
}

TEST_CASE( "S3 self-move-assign keeps a consistent 3x4 matrix", "[S3][S3-R1]" )
{
    auto m = s3_r1::filled( 3, 4 );
    s3_r1::self_move_assign( m, m );
    REQUIRE( m.row() == 3 );
    REQUIRE( m.col() == 4 );
    REQUIRE( m.size() == 12 );
    REQUIRE( m.end() - m.begin() == 12 );
    std::size_t i = 0;
    for ( auto const& v : m )
    {
        REQUIRE( v == static_cast<double>( i ) + 0.25 );
        ++i;
    }
    REQUIRE( i == 12 );
}

TEST_CASE( "S3 moved-from matrix is empty and consistent", "[S3][S3-R1]" )
{
    SECTION( "move construction" )
    {
        auto a = s3_r1::filled( 3, 4 );
        feng::matrix<double> b{ std::move( a ) };
        REQUIRE( a.row() == 0 );
        REQUIRE( a.col() == 0 );
        REQUIRE( a.size() == 0 );
        s3_r1::require_consistent( a );
        REQUIRE( b.row() == 3 );
        REQUIRE( b.col() == 4 );
        REQUIRE( b[2][3] == 11.25 );
        s3_r1::require_consistent( b );
    }
    SECTION( "move assignment" )
    {
        auto a = s3_r1::filled( 3, 4 );
        auto b = s3_r1::filled( 2, 5 );
        b = std::move( a );
        REQUIRE( a.row() == 0 );
        REQUIRE( a.col() == 0 );
        REQUIRE( a.size() == 0 );
        s3_r1::require_consistent( a );
        REQUIRE( b.row() == 3 );
        REQUIRE( b.col() == 4 );
        REQUIRE( b[1][0] == 4.25 );
        s3_r1::require_consistent( b );
        a = s3_r1::filled( 1, 2 ); // a moved-from matrix is usable again
        REQUIRE( a.size() == 2 );
        s3_r1::require_consistent( a );
    }
}

namespace s3_r1
{
    template < bool POCS >
    using tracked_matrix = feng::matrix< s3_alloc::tracked, s3_alloc::tracking_allocator< s3_alloc::tracked, false, false, POCS > >;
    template < typename T, bool POCS >
    using counted_matrix = feng::matrix< T, s3_alloc::tracking_allocator< T, false, false, POCS > >;

    template < typename M >
    void fill_tracked( M& m, int base )
    {
        for ( std::size_t i = 0; i != m.size(); ++i ) m.data()[i].value = base + static_cast< int >( i );
    }

    // Element-lifetime and resource balance across the shape-changing operations, the allocator kept throughout.
    template < bool POCS >
    void tracked_operations()
    {
        typedef tracked_matrix< POCS > matrix_type;
        typedef typename matrix_type::allocator_type allocator_type;
        s3_alloc::reset();
        {
            matrix_type m{ allocator_type{ 3 }, 3, 4 };
            fill_tracked( m, 0 );
            require_consistent( m );

            m.resize( 5, 6 );
            require_consistent( m );
            REQUIRE( m.get_allocator().id == 3 );
            m.resize( 6, 5 ); // same size: reshapes
            require_consistent( m );
            fill_tracked( m, 0 );
            m.reshape( 5, 6 );
            require_consistent( m );
            REQUIRE( m.row() == 5 );

            auto const c = m.clone( 1, 4, 2, 5 );
            require_consistent( c );
            REQUIRE( c.get_allocator().id == 3 );
            REQUIRE( c[0][0].value == 1 * 6 + 2 );
            m.clone( c, 0, 2, 0, 3 );
            require_consistent( m );
            REQUIRE( m.get_allocator().id == 3 );
            REQUIRE( m.row() == 2 );

            m.shrink_to_size( 4, 4 );
            require_consistent( m );
            REQUIRE( m.get_allocator().id == 3 );
            REQUIRE( m[1][2].value == c[1][2].value );
            REQUIRE( m[3][3].value == 0 );

            auto const t = m.transpose();
            require_consistent( t );
            REQUIRE( t.get_allocator().id == 3 );
            REQUIRE( t[2][1].value == m[1][2].value );

            auto const same = m.template astype< s3_alloc::tracked >();
            require_consistent( same );
            REQUIRE( same.get_allocator().id == 3 );

            // copy(rhs) on unequal resources: the destination keeps its own allocator (F04).
            matrix_type dst{ allocator_type{ 4 }, 1, 2 };
            dst.copy( t );
            require_consistent( dst );
            REQUIRE( dst.get_allocator().id == 4 );
            REQUIRE( dst.row() == 4 );
            REQUIRE( dst[2][1].value == t[2][1].value );
            dst.copy( m ); // same shape: no reallocation
            require_consistent( dst );
            REQUIRE( dst.get_allocator().id == 4 );
            REQUIRE( dst[1][2].value == m[1][2].value );
            REQUIRE( s3_alloc::registry::live( 4 ) == 1 );

            m.clear();
            require_consistent( m );
            REQUIRE( m.get_allocator().id == 3 );
        }
        require_balanced();
        REQUIRE( s3_alloc::registry::live( 3 ) == 0 );
        REQUIRE( s3_alloc::registry::live( 4 ) == 0 );
    }

    // Arithmetic, astype and converting constructors on a stateful allocator: results use the operand's resource.
    template < bool POCS >
    void counted_arithmetic()
    {
        typedef counted_matrix< double, POCS > matrix_type;
        typedef typename matrix_type::allocator_type allocator_type;
        s3_alloc::reset();
        {
            matrix_type m{ allocator_type{ 5 }, 2, 3 };
            for ( std::size_t i = 0; i != m.size(); ++i ) m.data()[i] = static_cast<double>( i ) + 0.5;
            matrix_type sq{ allocator_type{ 5 }, 2, 2 };
            sq[0][0] = 2.0; sq[0][1] = 1.0; sq[1][0] = 1.0; sq[1][1] = 3.0;
            auto const t = m.transpose();
            for ( auto const& r : { m + m, m - m, m + 1.0, 1.0 + m, m - 1.0, 1.0 - m, m * 2.0, 2.0 * m, m / 2.0, sq * m, sq / sq } )
            {
                require_consistent( r );
                REQUIRE( r.get_allocator().id == 5 );
            }
            auto p = m * t;
            require_consistent( p );
            REQUIRE( p.get_allocator().id == 5 );
            p *= sq;
            require_consistent( p );
            REQUIRE( p.get_allocator().id == 5 );
            p /= sq;
            require_consistent( p );
            REQUIRE( p.get_allocator().id == 5 );
            p += 1.0; p -= p; p *= 2.0; p /= 2.0;
            require_consistent( p );
            REQUIRE( p.get_allocator().id == 5 );

            auto const f = m.template astype< float >();
            require_consistent( f );
            REQUIRE( f.get_allocator().id == 5 );
            REQUIRE( f[1][2] == 5.5f );

            counted_matrix< float, POCS > const g{ m };
            require_consistent( g );
            REQUIRE( g.get_allocator().id == 5 );
            REQUIRE( g[1][1] == 4.5f );

            counted_matrix< float, POCS > h{ typename counted_matrix< float, POCS >::allocator_type{ 6 }, 1, 1 };
            h.copy( m );
            require_consistent( h );
            REQUIRE( h.get_allocator().id == 6 );
            REQUIRE( h[0][2] == 2.5f );
        }
        require_balanced();
    }
}

TEST_CASE( "S3 size equals rows times cols after every operation", "[S3][S3-R1]" )
{
    SECTION( "std::allocator, double" )
    {
        auto m = s3_r1::filled( 3, 4 );
        s3_r1::require_consistent( m );

        feng::matrix<double> copy{ m };
        s3_r1::require_consistent( copy );
        REQUIRE( copy == m );

        feng::matrix<double> assigned{ 7, 7 };
        assigned = m;
        s3_r1::require_consistent( assigned );
        REQUIRE( assigned == m );

        feng::matrix<double>& alias = assigned;
        assigned = alias;
        s3_r1::require_consistent( assigned );
        REQUIRE( assigned == m );

        feng::matrix<double> other{ 2, 2, 1.0 };
        other.swap( copy );
        s3_r1::require_consistent( other );
        s3_r1::require_consistent( copy );
        REQUIRE( copy.row() == 2 );
        REQUIRE( other.row() == 3 );

        m.resize( 5, 6 );
        s3_r1::require_consistent( m );
        m.reshape( 6, 5 );
        s3_r1::require_consistent( m );
        REQUIRE( m.row() == 6 );

        auto const c = m.clone( 1, 4, 2, 5 );
        s3_r1::require_consistent( c );
        REQUIRE( c.size() == 9 );

        m.shrink_to_size( 2, 3 );
        s3_r1::require_consistent( m );

        auto const f = m.astype<float>();
        s3_r1::require_consistent( f );

        auto const t = m.transpose();
        s3_r1::require_consistent( t );
        REQUIRE( t.row() == 3 );

        auto const sum = m + m;
        s3_r1::require_consistent( sum );
        auto const prod = m * t;
        s3_r1::require_consistent( prod );
        REQUIRE( prod.row() == 2 );
        REQUIRE( prod.col() == 2 );

        auto const diff = m - m;
        s3_r1::require_consistent( diff );
        REQUIRE( diff[1][2] == 0.0 );

        feng::matrix<double> const sq{ 2, 2, { 2.0, 1.0, 1.0, 3.0 } };
        auto const quot = sq / sq;
        s3_r1::require_consistent( quot );
        REQUIRE( quot.row() == 2 );

        for ( auto const& s : { m + 1.0, 1.0 + m, m - 1.0, 1.0 - m, m * 2.0, 2.0 * m, m / 2.0 } )
        {
            s3_r1::require_consistent( s );
            REQUIRE( s.row() == 2 );
            REQUIRE( s.col() == 3 );
        }
        REQUIRE( ( m * 2.0 )[1][2] == 2.0 * m[1][2] );

        auto compound = m;
        compound += m;      s3_r1::require_consistent( compound );
        compound -= m;      s3_r1::require_consistent( compound );
        compound += 1.0;    s3_r1::require_consistent( compound );
        compound -= 1.0;    s3_r1::require_consistent( compound );
        compound *= 2.0;    s3_r1::require_consistent( compound );
        compound /= 2.0;    s3_r1::require_consistent( compound );
        REQUIRE( compound == m );
        compound *= t;      s3_r1::require_consistent( compound );
        REQUIRE( compound.row() == 2 );
        REQUIRE( compound.col() == 2 );
        compound /= sq;     s3_r1::require_consistent( compound );
        REQUIRE( compound.col() == 2 );

        feng::matrix<float> const converted{ m };
        s3_r1::require_consistent( converted );
        REQUIRE( converted.row() == 2 );
        REQUIRE( converted[1][2] == static_cast<float>( m[1][2] ) );
        feng::matrix<int> const to_int{ m };
        s3_r1::require_consistent( to_int );

        feng::matrix<float> copied{ 4, 4 };
        copied.copy( m );
        s3_r1::require_consistent( copied );
        REQUIRE( copied.row() == 2 );
        REQUIRE( copied.col() == 3 );
        REQUIRE( copied[0][1] == static_cast<float>( m[0][1] ) );
        feng::matrix<float> assigned_from_double{ 1, 1 };
        assigned_from_double = m;
        s3_r1::require_consistent( assigned_from_double );
        REQUIRE( assigned_from_double.size() == 6 );

        m.clear();
        REQUIRE( m.row() == 0 );
        REQUIRE( m.col() == 0 );
        s3_r1::require_consistent( m );
    }
    SECTION( "tracked elements, POCS false" ) { s3_r1::tracked_operations< false >(); }
    SECTION( "tracked elements, POCS true" )  { s3_r1::tracked_operations< true >(); }
    SECTION( "stateful allocator arithmetic, POCS false" ) { s3_r1::counted_arithmetic< false >(); }
    SECTION( "stateful allocator arithmetic, POCS true" )  { s3_r1::counted_arithmetic< true >(); }
}
