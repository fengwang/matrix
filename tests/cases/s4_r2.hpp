// S4-R2 (PR-6): const and mutable views carry a pointer to the first viewed element, the extents and the parent's
// row stride (F06, F05; D-019).
#include <algorithm>
#include <cstddef>
#include <vector>

namespace s4_r2
{
    inline feng::matrix<double> distinct( std::size_t r, std::size_t c )
    {
        feng::matrix<double> m{ r, c };
        for ( std::size_t i = 0; i != r; ++i )
            for ( std::size_t j = 0; j != c; ++j )
                m( i, j ) = static_cast<double>( 100 * i + j ) + 0.5;
        return m;
    }

    template < typename It >
    std::vector<double> walk( It first, It last )
    {
        std::vector<double> ans;
        for ( ; first != last; ++first ) ans.push_back( *first );
        return ans;
    }

    // reads of a 2x3 view at rows [1, 3), columns [2, 5) of distinct( 5, 6 )
    template < typename View >
    void require_reads( View const& v, feng::matrix<double> const& m )
    {
        REQUIRE( v.row() == 2 );
        REQUIRE( v.col() == 3 );
        REQUIRE( v.size() == 6 );
        REQUIRE( v.row_stride() == 6 );
        auto const [vr, vc] = v.shape();
        REQUIRE( vr == 2 );
        REQUIRE( vc == 3 );
        for ( std::size_t r = 0; r != 2; ++r )
            for ( std::size_t c = 0; c != 3; ++c )
            {
                REQUIRE( v( r, c ) == m( r + 1, c + 2 ) );
                REQUIRE( v.at( r, c ) == m( r + 1, c + 2 ) );
                REQUIRE( v[r][c] == m( r + 1, c + 2 ) );
                REQUIRE( &v.at( r, c ) == &m.at( r + 1, c + 2 ) );
            }
        REQUIRE( walk( v.row_begin( 0 ), v.row_end( 0 ) ) == std::vector<double>{ 102.5, 103.5, 104.5 } );
        REQUIRE( walk( v.row_cbegin( 1 ), v.row_cend( 1 ) ) == std::vector<double>{ 202.5, 203.5, 204.5 } );
        for ( std::size_t c = 0; c != 3; ++c )
        {
            double const top = m( 1, c + 2 ), bottom = m( 2, c + 2 );
            REQUIRE( std::distance( v.col_begin( c ), v.col_end( c ) ) == 2 );
            REQUIRE( walk( v.col_begin( c ), v.col_end( c ) ) == std::vector<double>{ top, bottom } );
            REQUIRE( walk( v.col_cbegin( c ), v.col_cend( c ) ) == std::vector<double>{ top, bottom } );
            REQUIRE( walk( v.col_rbegin( c ), v.col_rend( c ) ) == std::vector<double>{ bottom, top } );
        }
        std::vector<double> const all{ 102.5, 103.5, 104.5, 202.5, 203.5, 204.5 };
        REQUIRE( std::distance( v.begin(), v.end() ) == 6 );
        REQUIRE( walk( v.begin(), v.end() ) == all );
        REQUIRE( walk( v.cbegin(), v.cend() ) == all );
        REQUIRE( walk( v.rbegin(), v.rend() ) == std::vector<double>( all.rbegin(), all.rend() ) );
        REQUIRE( v.begin()[4] == 203.5 );
        REQUIRE( v.get_allocator() == m.get_allocator() );
    }

    // every parent element outside rows [1, 3), columns [2, 5) still holds its distinct value
    inline void require_outside_untouched( feng::matrix<double> const& m )
    {
        auto const fresh = distinct( 5, 6 );
        for ( std::size_t i = 0; i != 5; ++i )
            for ( std::size_t j = 0; j != 6; ++j )
                if ( !( i >= 1 && i < 3 && j >= 2 && j < 5 ) )
                    REQUIRE( m( i, j ) == fresh( i, j ) );
    }

    inline void require_inside( feng::matrix<double> const& m, std::vector<double> const& expected )
    {
        std::size_t k = 0;
        for ( std::size_t i = 1; i != 3; ++i )
            for ( std::size_t j = 2; j != 5; ++j )
                REQUIRE( m( i, j ) == expected[k++] );
    }

    inline void require_six( feng::matrix<double> const& n )
    {
        REQUIRE( n.row() == 2 );
        REQUIRE( n.col() == 3 );
        REQUIRE( n.size() == 6 );
        REQUIRE( std::vector<double>( n.begin(), n.end() ) == std::vector<double>{ 102.5, 103.5, 104.5, 202.5, 203.5, 204.5 } );
    }
}

TEST_CASE( "S4 offset 2x3 view of a 5x6 matrix reads writes copies and iterates its elements", "[S4][S4-R2]" )
{
    auto m = s4_r2::distinct( 5, 6 );
    auto const& cm = m;

    SECTION( "the const view reads exactly the six elements, forward and reverse" )
    {
        auto const v = feng::make_view( cm, { 1, 3 }, { 2, 5 } );
        s4_r2::require_reads( v, m );
        feng::matrix_view<double, std::allocator<double>> const w{ cm, { 1, 3 }, { 2, 5 } };
        s4_r2::require_reads( w, m );
    }
    SECTION( "the mutable view reads exactly the six elements and converts to the const view" )
    {
        auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } );
        s4_r2::require_reads( v, m );
        feng::matrix_view<double, std::allocator<double>> const w = v;
        s4_r2::require_reads( w, m );
        feng::mutable_matrix_view<double, std::allocator<double>> u{ m, { 1, 3 }, { 2, 5 } };
        s4_r2::require_reads( u, m );
    }
    SECTION( "writes through at, (), [][] change exactly the viewed elements" )
    {
        auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } );
        v.at( 0, 0 ) = -1.0;
        v( 0, 2 ) = -2.0;
        v[1][1] = -3.0;
        *v.row_begin( 1 ) = -4.0;
        s4_r2::require_inside( m, { -1.0, 103.5, -2.0, -4.0, -3.0, 204.5 } );
        s4_r2::require_outside_untouched( m );
    }
    SECTION( "std::fill over begin..end changes exactly the viewed elements" )
    {
        auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } );
        std::fill( v.begin(), v.end(), 7.0 );
        s4_r2::require_inside( m, std::vector<double>( 6, 7.0 ) );
        s4_r2::require_outside_untouched( m );
    }
    SECTION( "writes through a column iterator change exactly that view column" )
    {
        auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } );
        std::fill( v.col_begin( 1 ), v.col_end( 1 ), 9.0 );
        s4_r2::require_inside( m, { 102.5, 9.0, 104.5, 202.5, 9.0, 204.5 } );
        s4_r2::require_outside_untouched( m );
    }
    SECTION( "a matrix from either view and copy from a view hold exactly the six values" )
    {
        auto const cv = feng::make_view( cm, { 1, 3 }, { 2, 5 } );
        auto const mv = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } );
        feng::matrix<double> const a{ cv };
        feng::matrix<double> const b{ mv };
        s4_r2::require_six( a );
        s4_r2::require_six( b );
        REQUIRE( a.get_allocator() == m.get_allocator() );
        feng::matrix<double> n{ 4, 4 };
        n.copy( cv );
        s4_r2::require_six( n );
        feng::matrix<double> p;
        p.copy( mv );
        s4_r2::require_six( p );
    }
}

// S4-T8 (S4-R2): every listed view shape (1x1, full-size, first-row, last-column, 1xN, Nx1) on 5x6, 1x6 and 6x1
// owners of distinct values, through make_view and make_mutable_view, on the stateful test allocator.
#include "./s3_alloc.hpp"
#include <iterator>

namespace s4_r2_shapes
{
    using alloc_type = s3_alloc::tracking_allocator< double, false, false, false >;
    using owner_type = feng::matrix< double, alloc_type >;

    struct shape_case
    {
        char const* name;
        std::size_t rows, cols;        // owner
        std::size_t r0, r1, c0, c1;    // view
    };

    inline std::vector< shape_case > const& cases()
    {
        static std::vector< shape_case > const all{
            { "1x1 at (2, 3) of 5x6", 5, 6, 2, 3, 3, 4 },
            { "full-size 5x6", 5, 6, 0, 5, 0, 6 },
            { "first row of 5x6", 5, 6, 0, 1, 0, 6 },
            { "last column of 5x6", 5, 6, 0, 5, 5, 6 },
            { "1x4 at (3, 1) of 5x6", 5, 6, 3, 4, 1, 5 },
            { "4x1 at (1, 2) of 5x6", 5, 6, 1, 5, 2, 3 },
            { "1x4 at (0, 1) of 1x6", 1, 6, 0, 1, 1, 5 },
            { "full-size 1x6", 1, 6, 0, 1, 0, 6 },
            { "3x1 at (2, 0) of 6x1", 6, 1, 2, 5, 0, 1 },
            { "full-size 6x1", 6, 1, 0, 6, 0, 1 },
        };
        return all;
    }

    // the owner's element (i, j) before any write: distinct over the whole owner
    inline double original( std::size_t i, std::size_t j ) { return 1000.0 * static_cast< double >( i ) + static_cast< double >( j ) + 0.25; }

    inline owner_type make_owner( shape_case const& s, int id )
    {
        owner_type m{ alloc_type{ id }, s.rows, s.cols };
        double* const p = m.data();
        for ( std::size_t i = 0; i != s.rows; ++i )
            for ( std::size_t j = 0; j != s.cols; ++j )
                p[ i * s.cols + j ] = original( i, j );
        return m;
    }

    inline bool inside( shape_case const& s, std::size_t i, std::size_t j ) { return i >= s.r0 && i < s.r1 && j >= s.c0 && j < s.c1; }

    // oracle: the viewed values in row order, by index arithmetic on the owner's storage
    inline std::vector< double > viewed( shape_case const& s, owner_type const& m )
    {
        std::vector< double > ans;
        double const* const p = m.data();
        for ( std::size_t i = s.r0; i != s.r1; ++i )
            for ( std::size_t j = s.c0; j != s.c1; ++j )
                ans.push_back( p[ i * s.cols + j ] );
        return ans;
    }

    template < typename It >
    std::vector< double > walk( It first, It last )
    {
        std::vector< double > ans;
        for ( ; first != last; ++first ) ans.push_back( *first );
        return ans;
    }

    template < typename View >
    void require_reads( shape_case const& s, View const& v, owner_type const& m )
    {
        std::size_t const vr = s.r1 - s.r0, vc = s.c1 - s.c0;
        double const* const p = m.data();
        auto const at = [&]( std::size_t r, std::size_t c ) { return p[ ( s.r0 + r ) * s.cols + ( s.c0 + c ) ]; };
        REQUIRE( v.row() == vr );
        REQUIRE( v.col() == vc );
        REQUIRE( v.size() == vr * vc );
        REQUIRE( v.row_stride() == s.cols );
        auto const [sr, sc] = v.shape();
        REQUIRE( sr == vr );
        REQUIRE( sc == vc );
        for ( std::size_t r = 0; r != vr; ++r )
            for ( std::size_t c = 0; c != vc; ++c )
            {
                REQUIRE( v( r, c ) == at( r, c ) );
                REQUIRE( v.at( r, c ) == at( r, c ) );
                REQUIRE( v[r][c] == at( r, c ) );
                REQUIRE( &v.at( r, c ) == p + ( s.r0 + r ) * s.cols + ( s.c0 + c ) );
            }
        for ( std::size_t r = 0; r != vr; ++r )
        {
            std::vector< double > row;
            for ( std::size_t c = 0; c != vc; ++c ) row.push_back( at( r, c ) );
            REQUIRE( std::distance( v.row_begin( r ), v.row_end( r ) ) == static_cast< std::ptrdiff_t >( vc ) );
            REQUIRE( walk( v.row_begin( r ), v.row_end( r ) ) == row );
            REQUIRE( walk( v.row_cbegin( r ), v.row_cend( r ) ) == row );
        }
        for ( std::size_t c = 0; c != vc; ++c )
        {
            std::vector< double > col;
            for ( std::size_t r = 0; r != vr; ++r ) col.push_back( at( r, c ) );
            std::vector< double > const rcol( col.rbegin(), col.rend() );
            REQUIRE( std::distance( v.col_begin( c ), v.col_end( c ) ) == static_cast< std::ptrdiff_t >( vr ) );
            REQUIRE( std::distance( v.col_rbegin( c ), v.col_rend( c ) ) == static_cast< std::ptrdiff_t >( vr ) );
            REQUIRE( walk( v.col_begin( c ), v.col_end( c ) ) == col );
            REQUIRE( walk( v.col_cbegin( c ), v.col_cend( c ) ) == col );
            REQUIRE( walk( v.col_rbegin( c ), v.col_rend( c ) ) == rcol );
            REQUIRE( walk( v.col_crbegin( c ), v.col_crend( c ) ) == rcol );
        }
        std::vector< double > const all = viewed( s, m );
        std::vector< double > const rall( all.rbegin(), all.rend() );
        REQUIRE( std::distance( v.begin(), v.end() ) == static_cast< std::ptrdiff_t >( all.size() ) );
        REQUIRE( std::distance( v.rbegin(), v.rend() ) == static_cast< std::ptrdiff_t >( all.size() ) );
        REQUIRE( walk( v.begin(), v.end() ) == all );
        REQUIRE( walk( v.cbegin(), v.cend() ) == all );
        REQUIRE( walk( v.rbegin(), v.rend() ) == rall );
        REQUIRE( walk( v.crbegin(), v.crend() ) == rall );
        REQUIRE( v.get_allocator() == m.get_allocator() );
    }

    // after a write of `written` (row order) through a view, the viewed elements hold it and the rest are original
    inline void require_written( shape_case const& s, owner_type const& m, std::vector< double > const& written )
    {
        REQUIRE( viewed( s, m ) == written );
        double const* const p = m.data();
        for ( std::size_t i = 0; i != s.rows; ++i )
            for ( std::size_t j = 0; j != s.cols; ++j )
                if ( !inside( s, i, j ) )
                    REQUIRE( p[ i * s.cols + j ] == original( i, j ) );
    }

    // values -1, -2, ... in row order of the view
    inline std::vector< double > marks( shape_case const& s )
    {
        std::vector< double > ans( ( s.r1 - s.r0 ) * ( s.c1 - s.c0 ) );
        for ( std::size_t k = 0; k != ans.size(); ++k ) ans[k] = -1.0 - static_cast< double >( k );
        return ans;
    }

    template < typename Write >
    void check_write( shape_case const& s, Write write )
    {
        auto m = make_owner( s, 3 );
        auto v = feng::make_mutable_view( m, { s.r0, s.r1 }, { s.c0, s.c1 } );
        write( v, s.c1 - s.c0 );
        require_written( s, m, marks( s ) );
    }

    inline void require_matrix( owner_type const& n, std::size_t r, std::size_t c, std::vector< double > const& values )
    {
        REQUIRE( n.row() == r );
        REQUIRE( n.col() == c );
        REQUIRE( n.size() == values.size() );
        REQUIRE( std::vector< double >( n.begin(), n.end() ) == values );
    }
}

TEST_CASE( "S4 every listed view shape reads writes and copies exactly", "[S4][S4-R2]" )
{
    using namespace s4_r2_shapes;
    s3_alloc::reset();
    for ( auto const& s : cases() )
    {
        INFO( "view " << s.name );
        std::size_t const vr = s.r1 - s.r0, vc = s.c1 - s.c0;
        {
            auto m = make_owner( s, 1 );
            auto const& cm = m;
            auto const cv = feng::make_view( cm, { s.r0, s.r1 }, { s.c0, s.c1 } );
            require_reads( s, cv, m );
            auto const mv = feng::make_mutable_view( m, { s.r0, s.r1 }, { s.c0, s.c1 } );
            require_reads( s, mv, m );
            feng::matrix_view< double, alloc_type > const conv = mv;
            require_reads( s, conv, m );
        }
        // writes through the mutable view change exactly the viewed elements
        check_write( s, []( auto& v, std::size_t ) { for ( std::size_t r = 0; r != v.row(); ++r ) for ( std::size_t c = 0; c != v.col(); ++c ) v.at( r, c ) = -1.0 - static_cast< double >( r * v.col() + c ); } );
        check_write( s, []( auto& v, std::size_t ) { for ( std::size_t r = 0; r != v.row(); ++r ) for ( std::size_t c = 0; c != v.col(); ++c ) v( r, c ) = -1.0 - static_cast< double >( r * v.col() + c ); } );
        check_write( s, []( auto& v, std::size_t ) { for ( std::size_t r = 0; r != v.row(); ++r ) for ( std::size_t c = 0; c != v.col(); ++c ) v[r][c] = -1.0 - static_cast< double >( r * v.col() + c ); } );
        check_write( s, []( auto& v, std::size_t ) { double x = -1.0; for ( auto it = v.begin(); it != v.end(); ++it ) *it = x--; } );
        check_write( s, []( auto& v, std::size_t ) {
            for ( std::size_t c = 0; c != v.col(); ++c )
            {
                std::size_t r = 0;
                for ( auto it = v.col_begin( c ); it != v.col_end( c ); ++it, ++r ) *it = -1.0 - static_cast< double >( r * v.col() + c );
            }
        } );
        check_write( s, []( auto& v, std::size_t ) {
            for ( std::size_t c = 0; c != v.col(); ++c )
            {
                std::size_t r = v.row();
                for ( auto it = v.col_rbegin( c ); it != v.col_rend( c ); ++it ) { --r; *it = -1.0 - static_cast< double >( r * v.col() + c ); }
            }
        } );
        check_write( s, []( auto& v, std::size_t ) { std::size_t k = v.size(); for ( auto it = v.rbegin(); it != v.rend(); ++it ) { --k; *it = -1.0 - static_cast< double >( k ); } } );
        {
            auto m = make_owner( s, 3 );
            auto v = feng::make_mutable_view( m, { s.r0, s.r1 }, { s.c0, s.c1 } );
            std::fill( v.begin(), v.end(), -7.0 );
            require_written( s, m, std::vector< double >( vr * vc, -7.0 ) );
        }
        // copies from either view hold exactly the viewed values in row order
        {
            auto m = make_owner( s, 4 );
            auto const& cm = m;
            auto const expected = viewed( s, m );
            auto const cv = feng::make_view( cm, { s.r0, s.r1 }, { s.c0, s.c1 } );
            auto const mv = feng::make_mutable_view( m, { s.r0, s.r1 }, { s.c0, s.c1 } );
            owner_type const a{ cv };
            owner_type const b{ mv };
            require_matrix( a, vr, vc, expected );
            require_matrix( b, vr, vc, expected );
            REQUIRE( a.get_allocator().id == 4 );
            REQUIRE( b.get_allocator().id == 4 );
            owner_type n{ alloc_type{ 5 }, 2, 2 };
            n.copy( cv );
            require_matrix( n, vr, vc, expected );
            owner_type p{ alloc_type{ 5 } };
            p.copy( mv );
            require_matrix( p, vr, vc, expected );
            // S3 (F04): the destination of copy keeps its own allocator
            REQUIRE( n.get_allocator().id == 5 );
            REQUIRE( p.get_allocator().id == 5 );
            require_written( s, m, expected );
        }
    }
    s3_alloc::require_balanced();
}

TEST_CASE( "S4 copy from a view of the destination snapshots first", "[S4][S4-R2]" )
{
    using namespace s4_r2_shapes;
    s3_alloc::reset();
    for ( auto const& s : cases() )
    {
        INFO( "view " << s.name );
        std::size_t const vr = s.r1 - s.r0, vc = s.c1 - s.c0;
        {
            auto m = make_owner( s, 6 );
            auto const expected = viewed( s, m );
            auto const& cm = m;
            m.copy( feng::make_view( cm, { s.r0, s.r1 }, { s.c0, s.c1 } ) );
            require_matrix( m, vr, vc, expected );
            REQUIRE( m.get_allocator().id == 6 );
        }
        {
            auto m = make_owner( s, 6 );
            auto const expected = viewed( s, m );
            m.copy( feng::make_mutable_view( m, { s.r0, s.r1 }, { s.c0, s.c1 } ) );
            require_matrix( m, vr, vc, expected );
            REQUIRE( m.get_allocator().id == 6 );
        }
    }
    s3_alloc::require_balanced();
}

// S4-R2, S4-R1 (D-020): in the checked-iterator build the view element iterator aborts on a dereference or
// subscript outside [0, count) and on a position outside [0, count], for both view types. The unchecked build
// keeps the case with in-range checks only, so the name matches in every lane.
#include "./s2_death.hpp"

TEST_CASE( "S4 checked view iterators abort outside the view", "[S4][S4-R2]" )
{
#ifdef FENG_MATRIX_CHECKED_ITERATORS
    // const view: rows [1, 3), columns [2, 5) of a 5x6 owner, six elements
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto const& cm = m; auto v = feng::make_view( cm, { 1, 3 }, { 2, 5 } ); double volatile x = *v.end(); (void)x; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto const& cm = m; auto v = feng::make_view( cm, { 1, 3 }, { 2, 5 } ); double volatile x = v.begin()[6]; (void)x; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto const& cm = m; auto v = feng::make_view( cm, { 1, 3 }, { 2, 5 } ); double volatile x = v.end()[-7]; (void)x; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto const& cm = m; auto v = feng::make_view( cm, { 1, 3 }, { 2, 5 } ); auto it = v.begin(); --it; (void)it; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto const& cm = m; auto v = feng::make_view( cm, { 1, 3 }, { 2, 5 } ); auto it = v.end() + 1; (void)it; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto const& cm = m; auto v = feng::make_view( cm, { 1, 3 }, { 2, 5 } ); auto it = v.cbegin() - 1; (void)it; }, "matrix iterator" );
    // mutable view of the same elements
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } ); *v.end() = 1.0; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } ); v.begin()[6] = 1.0; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } ); v.begin()[-1] = 1.0; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } ); auto it = v.begin(); it -= 1; (void)it; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } ); auto it = v.end(); ++it; (void)it; }, "matrix iterator" );
    S2_REQUIRE_DEATH( []{ auto m = s4_r2::distinct( 5, 6 ); auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } ); auto it = v.begin() + 7; (void)it; }, "matrix iterator" );
#endif
    // the boundary positions themselves are legal in every build
    auto m = s4_r2::distinct( 5, 6 );
    auto v = feng::make_mutable_view( m, { 1, 3 }, { 2, 5 } );
    REQUIRE( v.end() - v.begin() == 6 );
    REQUIRE( v.begin()[5] == m( 2, 4 ) );
    REQUIRE( *( v.end() - 1 ) == m( 2, 4 ) );
    REQUIRE( v.end()[-6] == m( 1, 2 ) );
}
