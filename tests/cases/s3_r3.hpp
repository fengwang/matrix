// S3-R3 (PR-5): the element-type constraint feng::matrix_element.
#include <complex>
#include <cstdint>
#include <string>

static_assert( feng::matrix_element< int > );
static_assert( feng::matrix_element< double > );
static_assert( feng::matrix_element< std::uint8_t > );
static_assert( feng::matrix_element< std::complex< float > > );
static_assert( feng::matrix_element< std::complex< double > > );
static_assert( feng::matrix_element< std::complex< long double > > );
static_assert( !feng::matrix_element< bool > );
static_assert( !feng::matrix_element< std::string > );

namespace s3_r3
{
    template < typename... Ts >
    inline constexpr bool all_elements = ( feng::matrix_element< Ts > && ... );

    struct throwing_copy
    {
        throwing_copy() noexcept = default;
        throwing_copy( throwing_copy const& ) noexcept( false ) {}
        throwing_copy( throwing_copy&& ) noexcept = default;
        throwing_copy& operator=( throwing_copy const& ) noexcept = default;
        throwing_copy& operator=( throwing_copy&& ) noexcept = default;
    };

    struct throwing_move
    {
        throwing_move() noexcept = default;
        throwing_move( throwing_move const& ) noexcept = default;
        throwing_move( throwing_move&& ) noexcept( false ) {}
        throwing_move& operator=( throwing_move const& ) noexcept = default;
        throwing_move& operator=( throwing_move&& ) noexcept = default;
    };

    struct throwing_destructor
    {
        ~throwing_destructor() noexcept( false ) {}
    };

    // D-018: the size constructors default-construct elements.
    struct no_default
    {
        explicit no_default( int ) noexcept {}
    };

    struct throwing_default
    {
        throwing_default() noexcept( false ) {}
    };
}

// Every standard arithmetic type except bool, plus the fixed-width aliases.
static_assert( s3_r3::all_elements< char, signed char, unsigned char, short, unsigned short, int, unsigned int, long,
                                    unsigned long, long long, unsigned long long, float, double, long double,
                                    std::size_t, std::int8_t, std::int16_t, std::int32_t, std::int64_t, std::uint8_t,
                                    std::uint16_t, std::uint32_t, std::uint64_t > );
static_assert( s3_r3::all_elements< std::complex< float >, std::complex< double >, std::complex< long double > > );
static_assert( !feng::matrix_element< s3_r3::throwing_copy > );
static_assert( !feng::matrix_element< s3_r3::throwing_move > );
static_assert( !feng::matrix_element< s3_r3::throwing_destructor > );
static_assert( !feng::matrix_element< s3_r3::no_default > );
static_assert( !feng::matrix_element< s3_r3::throwing_default > );
static_assert( !feng::matrix_element< int const > );
static_assert( !feng::matrix_element< int& > );

TEST_CASE( "S3 matrix_element accepts arithmetic and complex types and rejects bool and std::string", "[S3][S3-R3]" )
{
    static_assert( feng::matrix_element< int > && feng::matrix_element< double > && feng::matrix_element< std::uint8_t > );
    static_assert( feng::matrix_element< std::complex< float > > && feng::matrix_element< std::complex< double > > && feng::matrix_element< std::complex< long double > > );
    static_assert( !feng::matrix_element< bool > && !feng::matrix_element< std::string > );
    feng::matrix< std::uint8_t > const mask{ 2, 3 };
    feng::matrix< std::complex< long double > > const z{ 3, 2 };
    REQUIRE( mask.size() == 6 );
    REQUIRE( z.size() == 6 );
}
