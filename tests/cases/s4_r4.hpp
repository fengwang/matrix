// S4-R4 (PR-6): the curried map and reduce helpers hold copies of their arguments (F12).
#include <cstddef>
#include <memory>
#include <numeric>
#include <vector>

TEST_CASE( "S4 stored curried reduce outlives its init", "[S4][S4-R4]" )
{
    feng::matrix<double> m{ 3, 5 };
    std::iota( m.begin(), m.end(), 1.0 );

    SECTION( "reduce( func, init ) keeps copies of func and init" )
    {
        std::vector<double> weights_copy;
        double init_copy = 0.0;
        auto f = [&]
        {
            std::vector<double> weights( 4, 1.0 );
            double init = 7.0;
            weights_copy = weights;
            init_copy = init;
            auto func = [weights]( double a, double b ) noexcept { return a + b * weights[0]; };
            return feng::matrix_details::reduce( func, init );
        }();
        auto const plus = []( double a, double b ) { return a + b; };
        REQUIRE( m.size() < 32 );
        REQUIRE( f( m ) == std::accumulate( m.begin(), m.end(), init_copy, plus ) );
        REQUIRE( weights_copy.size() == 4 );
    }
    SECTION( "map( func ) keeps a copy of func" )
    {
        feng::matrix<double> b{ 3, 5 };
        std::iota( b.begin(), b.end(), 100.0 );
        auto g = []
        {
            std::vector<double> scale( 3, 2.0 );
            auto func = [scale]( double x, double y ) noexcept { return x * scale[1] + y; };
            return feng::matrix_details::map( func );
        }();
        auto const r = g( m, b );
        REQUIRE( r.row() == 3 );
        REQUIRE( r.col() == 5 );
        for ( std::size_t i = 0; i != r.size(); ++i )
            REQUIRE( r.data()[i] == m.data()[i] * 2.0 + b.data()[i] );
    }
}

// S4-R4 (F12): reduce( func, init ) folds init in exactly once, serial or parallel, and never forms a pointer past
// m.end(); the values are small integers so every sum is exact in double whatever the grouping.
TEST_CASE( "S4 curried reduce includes init once at every size", "[S4][S4-R4]" )
{
    auto const plus = []( double a, double b ) noexcept { return a + b; };
    for ( std::size_t n : { std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 31 }, std::size_t{ 32 }, std::size_t{ 33 }, std::size_t{ 1000 } } )
    {
        INFO( "size " << n );
        feng::matrix<double> m{ 1, n };
        std::iota( m.begin(), m.end(), 1.0 );
        double const init = 1000003.0;
        auto const f = feng::matrix_details::reduce( plus, init );
        REQUIRE( f( m ) == std::accumulate( m.begin(), m.end(), init, plus ) );
        auto const f0 = feng::matrix_details::reduce( plus, 0.0 );
        REQUIRE( f0( m ) == std::accumulate( m.begin(), m.end(), 0.0, plus ) );
    }

    SECTION( "a stored map whose callable owned a heap value runs after that scope ends" )
    {
        auto g = []
        {
            auto offset = std::make_unique<double>( 3.0 );
            auto func = [offset = std::shared_ptr<double>( std::move( offset ) )]( double x, double y ) noexcept { return x - y + *offset; };
            return feng::matrix_details::map( func );
        }();
        for ( std::size_t n : { std::size_t{ 1 }, std::size_t{ 33 }, std::size_t{ 1000 } } )
        {
            INFO( "size " << n );
            feng::matrix<double> a{ 1, n }, b{ 1, n };
            std::iota( a.begin(), a.end(), 10.0 );
            std::iota( b.begin(), b.end(), 1.0 );
            auto const r = g( a, b );
            REQUIRE( r.size() == n );
            for ( std::size_t i = 0; i != n; ++i )
                REQUIRE( r.data()[i] == 12.0 );
        }
    }
}
