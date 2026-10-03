// S9-R5 (PR-12, D-032): std::expected adapters over linalg_result, the factorization objects and the S5 loaders
// where the library has std::expected. In C++20 the adapters are not declared and the same results stay
// available as linalg_result, the factorization objects' status() and the bool loaders.
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <utility>
#include <string>
#include <type_traits>
#include <version>

#include <unistd.h>

namespace s9_r5
{
    template < typename R >
    concept has_to_expected = requires( R r ) { to_expected( r ); };

    template < typename T, typename F >
    concept has_load_expected = requires( F f ) { load_expected< T >( std::string{}, f ); };

    inline std::string temp( std::string const& name )
    {
        return ( std::filesystem::temp_directory_path() / ( "feng_s9_r5_" + std::to_string( ::getpid() ) + "_" + name ) ).string();
    }

    inline std::string missing() { return temp( "does_not_exist.npy" ); }

    inline feng::matrix< double > invertible() { return feng::matrix< double >{ 2, 2, { 2.0, 1.0, 1.0, 1.0 } }; }
    inline feng::matrix< double > singular() { return feng::matrix< double >{ 2, 2, { 1.0, 2.0, 2.0, 4.0 } }; }
}

TEST_CASE( "S9-R5 io_status and io_format are always declared", "[S9][S9-R5]" )
{
    static_assert( std::is_enum_v< feng::io_status > && std::is_enum_v< feng::io_format > );
    REQUIRE( feng::io_status::ok != feng::io_status::failed );
    REQUIRE( feng::io_format::txt != feng::io_format::binary );
    REQUIRE( feng::io_format::binary != feng::io_format::npy );
}

#if defined( __cpp_lib_expected )
TEST_CASE( "S9-R5 to_expected carries value or status", "[S9][S9-R5]" )
{
    using s9_r5::invertible;
    using s9_r5::singular;
    using E = std::expected< feng::matrix< double >, feng::linalg_status >;

    SECTION( "linalg_result" )
    {
        static_assert( std::is_same_v< decltype( feng::to_expected( feng::try_inverse( invertible() ) ) ), E > );
        static_assert( noexcept( feng::to_expected( std::declval< feng::linalg_result< feng::matrix< double > > >() ) ) );
        auto const ok = feng::to_expected( feng::try_inverse( invertible() ) );
        REQUIRE( ok.has_value() );
        REQUIRE( ( *ok )[0][0] == 1.0 );
        REQUIRE( ( *ok )[0][1] == -1.0 );
        REQUIRE( ( *ok )[1][1] == 2.0 );
        auto const bad = feng::to_expected( feng::try_inverse( singular() ) );
        REQUIRE( !bad.has_value() );
        REQUIRE( bad.error() == feng::linalg_status::singular );
        feng::matrix< double > const b{ 2, 1, { 3.0, 2.0 } };
        static_assert( std::is_same_v< decltype( feng::to_expected( feng::solve( invertible(), b ) ) ), E > );
        auto const s = feng::to_expected( feng::solve( invertible(), b ) );
        REQUIRE( s.has_value() );
        REQUIRE( ( *s )[0][0] == 1.0 );
        REQUIRE( ( *s )[1][0] == 1.0 );
        auto const s_bad = feng::to_expected( feng::solve( singular(), b ) );
        REQUIRE( !s_bad.has_value() );
        REQUIRE( s_bad.error() == feng::linalg_status::singular );
    }

    SECTION( "linalg_result from the factorization members" )
    {
        feng::matrix< double > const b{ 2, 1, { 3.0, 2.0 } };
        auto const lu = feng::lu_factor( invertible() );
        auto const lu_singular = feng::lu_factor( singular() );
        static_assert( std::is_same_v< decltype( feng::to_expected( lu.solve( b ) ) ), E > );
        static_assert( std::is_same_v< decltype( feng::to_expected( lu.inverse() ) ), E > );
        auto const x = feng::to_expected( lu.solve( b ) );
        REQUIRE( x.has_value() );
        REQUIRE( ( *x )[0][0] == 1.0 );
        REQUIRE( ( *x )[1][0] == 1.0 );
        REQUIRE( feng::to_expected( lu_singular.solve( b ) ).error_or( feng::linalg_status::ok ) == feng::linalg_status::singular );
        auto const inv = feng::to_expected( lu.inverse() );
        REQUIRE( inv.has_value() );
        REQUIRE( *inv == feng::matrix< double >{ 2, 2, { 1.0, -1.0, -1.0, 2.0 } } );
        REQUIRE( feng::to_expected( lu_singular.inverse() ).error_or( feng::linalg_status::ok ) == feng::linalg_status::singular );

        auto const svd = feng::svd_factor( invertible() );
        auto const svd_nonfinite = feng::svd_factor( feng::matrix< double >{ 2, 2, { 1.0, std::numeric_limits< double >::quiet_NaN(), 0.0, 1.0 } } );
        static_assert( std::is_same_v< decltype( feng::to_expected( svd.pinverse() ) ), E > );
        auto const p = feng::to_expected( svd.pinverse() );
        REQUIRE( p.has_value() );
        REQUIRE( p->row() == 2 );
        REQUIRE( p->col() == 2 );
        double const expected[2][2] = { { 1.0, -1.0 }, { -1.0, 2.0 } };
        for ( std::size_t r = 0; r != 2; ++r )
            for ( std::size_t c = 0; c != 2; ++c )
                REQUIRE( std::abs( ( *p )[r][c] - expected[r][c] ) < 1.0e-12 );
        auto const p_bad = feng::to_expected( svd_nonfinite.pinverse() );
        REQUIRE( !p_bad.has_value() );
        REQUIRE( p_bad.error() == feng::linalg_status::nonfinite );
    }

    SECTION( "factorization objects" )
    {
        auto const lu = feng::to_expected( feng::lu_factor( invertible() ) );
        static_assert( std::is_same_v< std::remove_const_t< decltype( lu ) >, std::expected< feng::lu_factorization< double >, feng::linalg_status > > );
        REQUIRE( lu.has_value() );
        REQUIRE( lu->rank() == 2 );
        auto const lu_bad = feng::to_expected( feng::lu_factor( singular() ) );
        REQUIRE( !lu_bad.has_value() );
        REQUIRE( lu_bad.error() == feng::linalg_status::singular );

        auto const svd = feng::to_expected( feng::svd_factor( invertible() ) );
        static_assert( std::is_same_v< std::remove_const_t< decltype( svd ) >, std::expected< feng::svd_factorization< double >, feng::linalg_status > > );
        REQUIRE( svd.has_value() );
        auto const svd_bad = feng::to_expected( feng::svd_factor( feng::matrix< double >{ 2, 2, { 1.0, std::numeric_limits< double >::quiet_NaN(), 0.0, 1.0 } } ) );
        REQUIRE( !svd_bad.has_value() );
        REQUIRE( svd_bad.error() == feng::linalg_status::nonfinite );

        auto const ch = feng::to_expected( feng::cholesky_factor( invertible() ) );
        static_assert( std::is_same_v< std::remove_const_t< decltype( ch ) >, std::expected< feng::cholesky_factorization< double >, feng::linalg_status > > );
        REQUIRE( ch.has_value() );
        auto const ch_bad = feng::to_expected( feng::cholesky_factor( singular() ) );
        REQUIRE( !ch_bad.has_value() );
        REQUIRE( ch_bad.error() == feng::linalg_status::not_positive_definite );

        auto const re = feng::to_expected( feng::row_echelon( singular() ) );
        static_assert( std::is_same_v< std::remove_const_t< decltype( re ) >, std::expected< feng::rref_result< double >, feng::linalg_status > > );
        REQUIRE( re.has_value() );
        REQUIRE( re->rank == 1 );
        auto const re_bad = feng::to_expected( feng::row_echelon( feng::matrix< double >{ 1, 1, std::numeric_limits< double >::infinity() } ) );
        REQUIRE( !re_bad.has_value() );
        REQUIRE( re_bad.error() == feng::linalg_status::nonfinite );
    }

    SECTION( "load_expected for each format" )
    {
        auto const m = invertible();
        std::string const npy = s9_r5::temp( "m.npy" ), txt = s9_r5::temp( "m.txt" ), bin = s9_r5::temp( "m.bin" );
        for ( auto const& path : { npy, txt, bin } ) // fresh files under the run's TMPDIR
            std::filesystem::remove( path );
        REQUIRE( m.save_as_npy( npy ) );
        REQUIRE( m.save_as_txt( txt ) );
        REQUIRE( m.save_as_binary( bin ) );
        static_assert( std::is_same_v< decltype( feng::load_expected< double >( npy, feng::io_format::npy ) ), std::expected< feng::matrix< double >, feng::io_status > > );
        static_assert( noexcept( feng::load_expected< double >( npy, feng::io_format::npy ) ) );
        static_assert( s9_r5::has_load_expected< double, feng::io_format > );
        for ( auto const& [path, format] : { std::pair{ npy, feng::io_format::npy }, std::pair{ txt, feng::io_format::txt }, std::pair{ bin, feng::io_format::binary } } )
        {
            auto const r = feng::load_expected< double >( path, format );
            REQUIRE( r.has_value() );
            REQUIRE( *r == m );
            auto const bad = feng::load_expected< double >( s9_r5::missing(), format );
            REQUIRE( !bad.has_value() );
            REQUIRE( bad.error() == feng::io_status::failed );
        }
        std::filesystem::remove( npy );
        std::filesystem::remove( txt );
        std::filesystem::remove( bin );
    }
}
#else
TEST_CASE( "S9-R5 status results stay available in C++20", "[S9][S9-R5]" )
{
    static_assert( !s9_r5::has_to_expected< feng::linalg_result< feng::matrix< double > > > );
    static_assert( !s9_r5::has_to_expected< feng::lu_factorization< double > > );
    static_assert( !s9_r5::has_load_expected< double, feng::io_format > );

    auto const ok = feng::try_inverse( s9_r5::invertible() );
    REQUIRE( ok.ok() );
    REQUIRE( ok.value[1][1] == 2.0 );
    auto const bad = feng::try_inverse( s9_r5::singular() );
    REQUIRE( !bad );
    REQUIRE( bad.status == feng::linalg_status::singular );
    REQUIRE( feng::lu_factor( s9_r5::invertible() ).status() == feng::linalg_status::ok );
    REQUIRE( feng::lu_factor( s9_r5::singular() ).status() == feng::linalg_status::singular );

    feng::matrix< double > m;
    REQUIRE( !m.load_npy( s9_r5::missing() ) );
    std::string const npy = s9_r5::temp( "c20.npy" );
    REQUIRE( s9_r5::invertible().save_as_npy( npy ) );
    REQUIRE( m.load_npy( npy ) );
    REQUIRE( m == s9_r5::invertible() );
    std::filesystem::remove( npy );
}
#endif
