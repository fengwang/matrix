#include <cassert>
TEST_CASE( "Matrix mean", "[mean]" )
{
    unsigned long N = 10;
    for ( unsigned long sz = 1; sz != N; ++sz )
    {
        for ( unsigned long tz = 1; tz != N; ++tz )
        {
            feng::matrix<double> const& mat = feng::rand<double>( sz, tz );
            //feng::matrix<double> const& mat = feng::ones<double>( sz, tz );
            auto const mat_mean = feng::mean( mat );
            auto const acc_mean = std::accumulate( mat.begin(), mat.end(), 0.0 ) / mat.size();
            REQUIRE( std::abs(mat_mean-acc_mean) < 1.0e-7 );
        }
    }
}

// S4 C8: mean/variance/standard_deviation must return double for integer and
// floating point value types. integer matrices are promoted via astype<double>()
// (sum/size on an integer sum is unsigned integer division); the n-1 sample
// formula of standard_deviation is preserved by design (PRD revision C-10):
// std of {1,2} is sqrt(0.5/1) = sqrt(0.5) ~ 0.70711, NOT the population 0.5.
TEST_CASE( "Matrix mean/variance/standard_deviation: double for real value types (C8)", "[mean][variance][standard_deviation]" )
{
    // E10 canonical pin: 1x2 integer matrix
    feng::matrix<int> const m12{ 1, 2, { 1, 2 } };
    static_assert( std::is_same_v< decltype( feng::mean( m12 ) ), double > );
    static_assert( std::is_same_v< decltype( feng::variance( m12 ) ), double > );
    static_assert( std::is_same_v< decltype( feng::standard_deviation( m12 ) ), double > );
    REQUIRE( std::abs( feng::mean( m12 ) - 1.5 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::variance( m12 ) - 0.25 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::standard_deviation( m12 ) - std::sqrt( 0.5 ) ) < 1.0e-12 ); // n-1: sqrt(0.5/1)

    // 2x2 integer matrix (kills the {1,2;1,2} shape ambiguity: different std than 1x2)
    feng::matrix<int> const m22{ 2, 2, { 1, 2, 1, 2 } };
    REQUIRE( std::abs( feng::mean( m22 ) - 1.5 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::variance( m22 ) - 0.25 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::standard_deviation( m22 ) - std::sqrt( 1.0 / 3.0 ) ) < 1.0e-12 ); // sqrt(1.0/3)

    // 1x1 integer matrix: size<=1 branch of standard_deviation
    feng::matrix<int> const m11{ 1, 1, { 7 } };
    REQUIRE( feng::mean( m11 ) == 7.0 );
    REQUIRE( feng::variance( m11 ) == 0.0 );
    REQUIRE( feng::standard_deviation( m11 ) == 0.0 );

    // float matrices: promoted to double (values unchanged within rounding)
    feng::matrix<float> const f12{ 1, 2, { 1.0f, 2.0f } };
    static_assert( std::is_same_v< decltype( feng::mean( f12 ) ), double > );
    static_assert( std::is_same_v< decltype( feng::variance( f12 ) ), double > );
    static_assert( std::is_same_v< decltype( feng::standard_deviation( f12 ) ), double > );
    REQUIRE( std::abs( feng::mean( f12 ) - 1.5 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::variance( f12 ) - 0.25 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::standard_deviation( f12 ) - std::sqrt( 0.5 ) ) < 1.0e-12 );

    // double matrices: regression net, unchanged within rounding (copy-free fast path)
    feng::matrix<double> const d12{ 1, 2, { 1.0, 2.0 } };
    REQUIRE( std::abs( feng::mean( d12 ) - 1.5 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::variance( d12 ) - 0.25 ) < 1.0e-12 );
    REQUIRE( std::abs( feng::standard_deviation( d12 ) - std::sqrt( 0.5 ) ) < 1.0e-12 );

    // L1 (S4 sharded review): negative integer matrix — the pre-fix failure mode was
    // UNSIGNED integer division (negative sums wrap); positive-only pins cannot
    // distinguish signed-from from unsigned-from arithmetic.
    feng::matrix<int> const mneg{ 1, 2, { -1, 2 } };
    static_assert( std::is_same_v< decltype( feng::mean( mneg ) ), double > );
    REQUIRE( std::abs( feng::mean( mneg ) - 0.5 ) < 1.0e-12 ); // (−1+2)/2 = 0.5, not a wrapped unsigned
    REQUIRE( std::abs( feng::variance( mneg ) - 2.25 ) < 1.0e-12 ); // deviations {−1.5, 1.5}, squared {2.25, 2.25}
    REQUIRE( std::abs( feng::standard_deviation( mneg ) - std::sqrt( 4.5 ) ) < 1.0e-12 ); // n-1: sqrt(sum/1) = sqrt(4.5)
}

