#ifndef FENG_MATRIX_HPP_INCLUDED_
#define FENG_MATRIX_HPP_INCLUDED_

static_assert( __cplusplus >= 201709L, "C++20 is a must for this library, please update your compiler, or enable corresponding option such as -std=c++2a" );

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>

namespace feng
{
    constexpr std::uint_least64_t matrix_version = 20200324ULL;

    #ifdef NPARALLEL
    constexpr std::uint_least64_t parallel_mode = 0;
    #else
    constexpr std::uint_least64_t parallel_mode = 1;
    #endif

    #ifdef NDEBUG
    constexpr std::uint_least64_t debug_mode = 0;
    #else
    constexpr std::uint_least64_t debug_mode = 1;
    #endif

    namespace matrix_private
    {   // for macro `better_assert`
        template< typename... Args >
        void print_assertion(std::ostream& out, Args&&... args)
        {
            if constexpr( debug_mode )
            {
                out.precision( 20 );
                (out << ... << args) << std::endl;
                abort();
            }
        }
    }

    #ifdef better_assert
    #undef better_assert
    #endif
    //
    // enhancing 'assert' macro, usage:
    //
    // int a;
    // ...
    // better_assert( a > 0 ); //same as 'assert'
    // better_assert( a > 0, "a is expected larger than 0, but now a = " a ); //with more info dumped to std::cerr
    //
    #define better_assert(EXPRESSION, ... ) ((EXPRESSION) ? (void)0 : matrix_private::print_assertion(std::cerr, "[Assertion Failure]: '", #EXPRESSION, "' in File: ", __FILE__, " in Line: ",  __LINE__ __VA_OPT__(,) __VA_ARGS__))

    template < typename Type, class Allocator >
    struct matrix;

    namespace matrix_details
    {

        template< typename Integer_Type >
        struct integer_iterator
        {
            static_assert(std::is_integral_v<Integer_Type>, "Integral required");

            typedef Integer_Type                    value_type;
            typedef value_type&                     reference;
            typedef value_type*                     pointer;
            typedef const reference                 const_reference;
            typedef std::random_access_iterator_tag	iterator_category;
            typedef std::ptrdiff_t                  difference_type;
            typedef integer_iterator<value_type>    self_type;

            value_type                              value_;

            explicit integer_iterator( value_type value ) noexcept : value_{value} {}
            integer_iterator( self_type const& ) noexcept= default;
            integer_iterator( self_type && ) noexcept = default;
            self_type& operator = ( self_type const& ) noexcept = default;
            self_type& operator = ( self_type && ) noexcept = default;

            reference& operator*() noexcept
            {
                return value_;
            }

            const_reference& operator*() const noexcept
            {
                return value_;
            }

            self_type& operator += ( difference_type val ) noexcept
            {
                value_ += val;
                return *this;
            }

            self_type& operator -= ( difference_type val ) noexcept
            {
                value_ -= val;
                return *this;
            }

            self_type& operator++() noexcept
            {
                ++value_;
                return *this;
            }

            self_type const operator++(int) noexcept
            {
                self_type ans{*this};
                ++(*this);
                return ans;
            }

            friend difference_type operator - ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ - rhs.value_;
            }

            friend self_type const operator + ( self_type const& lhs, difference_type const& rhs ) noexcept
            {
                return self_type{ lhs.value_ + rhs };
            }

            friend self_type const operator + ( difference_type const& lhs, self_type const& rhs ) noexcept
            {
                return rhs + lhs;
            }

            friend bool operator == ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ == rhs.value_;
            }

            friend bool operator != ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ != rhs.value_;
            }

            friend bool operator < ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ < rhs.value_;
            }

            friend bool operator > ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ > rhs.value_;
            }

            friend bool operator <= ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ <= rhs.value_;
            }

            friend bool operator >= ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ >= rhs.value_;
            }
        };

        template< typename Integer_Type >
        struct integer_range
        {
            static_assert(std::is_integral_v<Integer_Type>, "Integral required");
            typedef Integer_Type value_type;
            value_type first_;
            value_type last_;

            integer_range( value_type first, value_type last ) noexcept : first_{first}, last_{last} {}

            auto const begin() const noexcept
            {
                return integer_iterator<value_type>{first_};
            }

            auto const end() const noexcept
            {
                return integer_iterator<value_type>{last_};
            }
        };

        template< typename Integer_Type >
        integer_range<Integer_Type> range( Integer_Type first, Integer_Type last ) noexcept
        {
            return {first, last};
        }

        template< typename Integer_Type >
        integer_range<Integer_Type> range( Integer_Type last ) noexcept
        {
            return { Integer_Type{0}, last };
        }

        template<typename Integer_Type, typename Function>
        void repeat( Function function, Integer_Type n )
        {
            while ( n-- )
                function();
        }

        template< typename Function, typename Integer_Type >
        void parallel( Function const& func, Integer_Type dim_first, Integer_Type dim_last, unsigned long threshold = 1024 ) // 1d parallel
        {
            if constexpr( parallel_mode == 0 )
            {
                for ( auto a : range( dim_first, dim_last ) )
                    func( a );
                return;
            }
            else // <- this is constexpr-if, `else` is a must
            {
                unsigned int const total_cores = std::thread::hardware_concurrency();

                // case of non-parallel or small jobs
                if ( (total_cores <= 1) || ((dim_last - dim_first) <= threshold) )
                {
                    for ( auto a : range( dim_first, dim_last ) )
                        func( a );
                    return;
                }

                // case of small job numbers
                std::vector<std::thread> threads;
                if ( dim_last - dim_first <= total_cores )
                {
                    for ( auto index = dim_first; index != dim_last; ++index )
                        threads.emplace_back( std::thread{[&func, index](){ func( index ); }} );
                    for ( auto& th : threads )
                        th.join();
                    return;
                }

                // case of more jobs than CPU cores
                auto const& job_slice = [&func]( Integer_Type a, Integer_Type b )
                {
                    if ( a >= b ) return;
                    while ( a != b )
                        func(a++);
                };

                threads.reserve( total_cores-1 );
                std::uint_least64_t tasks_per_thread = ( dim_last - dim_first + total_cores - 1 ) / total_cores;

                for ( auto index : range( total_cores-1 ) )
                {
                    Integer_Type first = tasks_per_thread * index + dim_first;
                    first = std::min( first, dim_last );
                    Integer_Type last =  first + tasks_per_thread;
                    last = std::min( last, dim_last );
                    threads.emplace_back( std::thread{ job_slice, first, last } );
                }

                job_slice( tasks_per_thread*(total_cores-1), dim_last );

                for ( auto& th : threads )
                    th.join();
            }
        }

        template< typename Function, typename Integer_Type >
        void parallel( Function const& func, Integer_Type dim_last )
        {
            parallel( func, Integer_Type{0}, dim_last );
        }

        namespace bmp_details
        {
            static std::vector<std::uint8_t> const generate_bmp_header( std::uint_least64_t const the_row, std::uint_least64_t const the_col )
            {
                auto const& ul_to_byte = []( std::uint_least64_t val ) { return static_cast< std::uint8_t >( val & 0xffUL ); };
                std::uint8_t file[14] = { 0x42, 0x4D, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
                std::uint8_t info[40] = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x13, 0x0B, 0, 0, 0x13, 0x0B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::uint_least64_t const padding_size = ( 4 - ( ( the_col * 3 ) & 0x3 ) ) & 0x3;
                std::uint_least64_t const data_size = the_col * the_row * 3 + the_row * padding_size;
                std::uint_least64_t const all_size = data_size + sizeof( file ) + sizeof( info );
                file[2]  = ul_to_byte( all_size );
                file[3]  = ul_to_byte( all_size >> 8 );
                file[4]  = ul_to_byte( all_size >> 16 );
                file[5]  = ul_to_byte( all_size >> 24 );
                info[4]  = ul_to_byte( the_col );
                info[5]  = ul_to_byte( the_col >> 8 );
                info[6]  = ul_to_byte( the_col >> 16 );
                info[7]  = ul_to_byte( the_col >> 24 );
                info[8]  = ul_to_byte( the_row );
                info[9]  = ul_to_byte( the_row >> 8 );
                info[10] = ul_to_byte( the_row >> 16 );
                info[11] = ul_to_byte( the_row >> 24 );
                info[20] = ul_to_byte( data_size );
                info[21] = ul_to_byte( data_size >> 8 );
                info[22] = ul_to_byte( data_size >> 16 );
                info[23] = ul_to_byte( data_size >> 24 );
                std::vector<std::uint8_t> header( 14+40, std::uint8_t{} );
                std::copy( file, file+14, header.begin() );
                std::copy( info, info+40, header.begin()+14 );
                return header;
            }

            inline std::uint8_t operator "" _u8(unsigned long long value)
            {
                return static_cast<std::uint8_t>(value);
            }

            static std::function<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>(double)>
            make_transformation_function(  std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> const& color_1, double value_1,
                                           std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> const& color_2, double value_2 )
            {
                auto const [r1, g1, b1] = color_1;
                auto const [r2, g2, b2] = color_2;

                return [r1=r1, g1=g1, b1=b1, r2=r2, g2=g2, b2=b2, value_1, value_2]( double x )
                {
                    double dr = static_cast<double>(r1) - static_cast<double>(r2);
                    double dg = static_cast<double>(g1) - static_cast<double>(g2);
                    double db = static_cast<double>(b1) - static_cast<double>(b2);
                    double dv = value_1 - value_2;
                    double off_ratio = (x - value_2) / dv;
                    double r = dr * off_ratio + r2;
                    double g = dg * off_ratio + g2;
                    double b = db * off_ratio + b2;
                    r = (r > 255.0) ? 255.0 : r; r = (r < 0.0) ? 0.0 : r;
                    g = (g > 255.0) ? 255.0 : g; g = (g < 0.0) ? 0.0 : g;
                    b = (b > 255.0) ? 255.0 : b; b = (b < 0.0) ? 0.0 : b;
                    return std::make_tuple(  static_cast<std::uint8_t>(static_cast<int>(r)),
                                             static_cast<std::uint8_t>(static_cast<int>(g)),
                                             static_cast<std::uint8_t>(static_cast<int>(b)) );
                };
            }

            //static std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>
            static std::function<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>(double)>
            make_color_map( std::vector<double> const& values, std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>> const& colors )
            {
                better_assert( (values.size() == colors.size()), "make_color_map::length of values and colors not match!" );
                better_assert( std::abs(*(values.begin())) < 1.0e-10 && "make_color_map::value should start from 0!" );
                better_assert( std::abs(*(values.rbegin())-1.0) < 1.0e-10 && "make_color_map::value should end at 1!" );

                return [=]( double x )
                {
                    for ( auto index : matrix_details::range( values.size() - 1 ) )
                        if ( x <= values[index+1] )
                            return  make_transformation_function( colors[index], values[index], colors[index+1], values[index+1] )(x);
                    better_assert( !"make_color_map::should never reach here! The input value x is ", x, ", and the values.size() is ", values.size() );
                    return std::make_tuple(0_u8, 0_u8, 0_u8);
                };
            }
            static std::function<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>(double)>
            make_color_map( std::initializer_list<double> const& values, std::initializer_list<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>> const& colors )
            {
                return make_color_map( std::vector<double>{values}, std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>>{colors} );
            }

            typedef std::function< std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>( double ) > color_value_type;
            static const std::map< std::string, color_value_type > color_maps
            {
                std::make_pair
                (
                    std::string{ "default" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0 },
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 255_u8),
                            std::make_tuple(0_u8, 255_u8, 255_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "season" },
                    make_color_map
                    (
                        { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},
                        {
                            std::make_tuple(255_u8, 255_u8, 255_u8),
                            std::make_tuple(153_u8, 204_u8, 255_u8),
                            std::make_tuple( 51_u8, 153_u8, 255_u8),
                            std::make_tuple(  0_u8, 128_u8, 255_u8),
                            std::make_tuple(  0_u8, 255_u8, 255_u8),
                            std::make_tuple(  0_u8, 255_u8, 128_u8),
                            std::make_tuple(  0_u8, 255_u8,   0_u8),
                            std::make_tuple(128_u8, 255_u8,   0_u8),
                            std::make_tuple(255_u8, 255_u8,   0_u8),
                            std::make_tuple(255_u8, 128_u8,   0_u8),
                            std::make_tuple(255_u8,   0_u8,   0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "pink" },
                    make_color_map
                    (
                        { 0.0, 0.0159, 1.0/9.0, 2.0/9.0, 3.0/9.0, 4.0/9.0, 5.0/9.0, 6.0/9.0, 7.0/9.0,1.0},
                        {
                            std::make_tuple(30_u8, 0_u8, 0_u8),
                            std::make_tuple(50_u8, 26_u8, 26_u8),
                            std::make_tuple(109_u8, 64_u8, 64_u8),
                            std::make_tuple(152_u8, 98_u8, 98_u8),
                            std::make_tuple(185_u8, 120_u8, 120_u8),
                            std::make_tuple(202_u8, 154_u8, 138_u8),
                            std::make_tuple(213_u8, 186_u8, 155_u8),
                            std::make_tuple(224_u8, 214_u8, 170_u8),
                            std::make_tuple(235_u8, 235_u8, 190_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "bone" },
                    make_color_map
                    (
                        { 0.0, 0.3651, 0.7460,1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(81_u8, 81_u8, 113_u8),
                            std::make_tuple(166_u8, 198_u8, 198_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "hot" },
                    make_color_map
                    (
                        { 0.0, 0.3651, 0.7460,1.0},
                        {
                            std::make_tuple(10_u8, 0_u8, 0_u8),
                            std::make_tuple(255_u8, 0_u8, 0_u8),
                            std::make_tuple(255_u8, 255_u8, 0_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "copper" },
                    make_color_map
                    (
                        { 0.0, 2.0/9.0, 4.0/9.0, 0.8095, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(70_u8, 44_u8, 28_u8),
                            std::make_tuple(141_u8, 88_u8, 56_u8),
                            std::make_tuple(255_u8, 161_u8, 102_u8),
                            std::make_tuple(255_u8, 199_u8, 126_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "lines" },
                    make_color_map
                    (
                        { 0.0,0.015873015873015872,0.031746031746031744,0.047619047619047616,0.06349206349206349,0.07936507936507936,0.09523809523809523,0.1111111111111111,0.12698412698412698,0.14285714285714285,0.15873015873015872,0.1746031746031746,0.19047619047619047,0.20634920634920634,0.2222222222222222,0.23809523809523808,0.25396825396825395,0.2698412698412698,0.2857142857142857,0.30158730158730157,0.31746031746031744,0.3333333333333333,0.3492063492063492,0.36507936507936506,0.38095238095238093,0.3968253968253968,0.4126984126984127,0.42857142857142855,0.4444444444444444,0.4603174603174603,0.47619047619047616,0.49206349206349204,0.5079365079365079,0.5238095238095238,0.5396825396825397,0.5555555555555556,0.5714285714285714,0.5873015873015873,0.6031746031746031,0.6190476190476191,0.6349206349206349,0.6507936507936508,0.6666666666666666,0.6825396825396826,0.6984126984126984,0.7142857142857143,0.7301587301587301,0.746031746031746,0.7619047619047619,0.7777777777777778,0.7936507936507936,0.8095238095238095,0.8253968253968254,0.8412698412698413,0.8571428571428571,0.873015873015873,0.8888888888888888,0.9047619047619048,0.9206349206349206,0.9365079365079365,0.9523809523809523,0.9682539682539683,0.9841269841269841,1.0 },
                        {
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    83_u8,    25_u8 ),
                            std::make_tuple( 127_u8,   127_u8,    32_u8 ),
                            std::make_tuple( 126_u8,    47_u8,   127_u8 ),
                            std::make_tuple( 119_u8,   127_u8,    48_u8 ),
                            std::make_tuple( 77_u8,   127_u8,   127_u8 ),
                            std::make_tuple( 127_u8,    20_u8,    47_u8 ),
                            std::make_tuple( 0_u8,   114_u8,   127_u8 )
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "cool" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(0_u8, 255_u8, 255_u8),
                            std::make_tuple(255_u8, 0_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "spring" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(255_u8, 0_u8, 255_u8),
                            std::make_tuple(255_u8, 255_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "summer" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(0_u8, 127_u8, 102_u8),
                            std::make_tuple(255_u8, 255_u8, 102_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "autumn" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(255_u8, 0_u8, 0_u8),
                            std::make_tuple(255_u8, 255_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "winter" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 255_u8),
                            std::make_tuple(0_u8, 255_u8, 127_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "hsv" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(255_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 255_u8, 0_u8),
                            std::make_tuple(0_u8, 255_u8, 255_u8),
                            std::make_tuple(255_u8, 0_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "jikjak" },
                    make_color_map
                    (
                        { 0.0, 1.5/25.0, 6.0/25.0, 12.5/25.0, 20.0/25.0, 25.0/25.0 },
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(63_u8, 127_u8, 255_u8),
                            std::make_tuple(63_u8, 255_u8, 127_u8),
                            std::make_tuple(191_u8, 63_u8, 191_u8),
                            std::make_tuple(255_u8, 0_u8, 191_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "hiphop" },
                    make_color_map
                    (
                        { 0.0, 1.0/25.0, 6.0/25.0, 12.5/25.0, 20.0/25.0, 25.0/25.0 },
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(63_u8, 127_u8, 255_u8),
                            std::make_tuple(0_u8, 255_u8, 127_u8),
                            std::make_tuple(191_u8, 191_u8, 0_u8),
                            std::make_tuple(255_u8, 127_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "tictoc" },
                    make_color_map
                    (
                        { 0.0, 0.05, 0.1, 0.2, 0.3, 0.45, 0.7, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 100_u8),
                            std::make_tuple(0_u8, 66_u8, 200_u8),
                            std::make_tuple(0_u8, 200_u8, 100_u8),
                            std::make_tuple(128_u8, 166_u8, 0_u8),
                            std::make_tuple(200_u8, 88_u8, 0_u8),
                            std::make_tuple(255_u8, 0_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "oops" },
                    make_color_map
                    (
                        { 0.0, 0.2, 0.4, 0.6, 0.8, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 160_u8),
                            std::make_tuple(0_u8, 160_u8, 240_u8),
                            std::make_tuple(80_u8, 240_u8, 160_u8),
                            std::make_tuple(160_u8, 160_u8, 0_u8),
                            std::make_tuple(255_u8, 80_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "ulala" },
                    make_color_map
                    (
                        { 0.0, 0.08, 0.1, 0.2, 0.3, 0.4, 0.5,  0.6, 0.7, 0.8, 0.9, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 25_u8),
                            std::make_tuple(0_u8, 0_u8, 50_u8),
                            std::make_tuple(0_u8, 0_u8, 100_u8),
                            std::make_tuple(0_u8, 33_u8, 150_u8),
                            std::make_tuple(0_u8, 66_u8, 200_u8),
                            std::make_tuple(0_u8, 99_u8, 255_u8),
                            std::make_tuple(0_u8, 133_u8, 200_u8),
                            std::make_tuple(0_u8, 200_u8, 167_u8),
                            std::make_tuple(0_u8, 255_u8, 133_u8),
                            std::make_tuple(33_u8, 170_u8, 66_u8),
                            std::make_tuple(255_u8, 85_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "zigzag" },
                    make_color_map
                    (
                        { 0.0, 0.04, 0.08, 0.12, 0.2, 0.3, 0.4, 0.5, 0.65, 0.8, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 11_u8),
                            std::make_tuple(0_u8, 0_u8, 22_u8),
                            std::make_tuple(0_u8, 0_u8, 33_u8),
                            std::make_tuple(0_u8, 0_u8, 133_u8),
                            std::make_tuple(0_u8, 0_u8, 167_u8),
                            std::make_tuple(0_u8, 0_u8, 255_u8),
                            std::make_tuple(0_u8, 127_u8, 167_u8),
                            std::make_tuple(0_u8, 255_u8, 133_u8),
                            std::make_tuple(33_u8, 170_u8, 66_u8),
                            std::make_tuple(255_u8, 85_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "parula" },
                    make_color_map
                    (
                        { 0.0, 0.1270, 0.2222, 0.4603, 0.7619, 1.0},
                        {
                            std::make_tuple(67_u8, 33_u8,  167_u8),
                            std::make_tuple(71_u8, 76_u8,  240_u8),
                            std::make_tuple(62_u8, 111_u8,  255_u8),
                            std::make_tuple(0_u8, 183_u8,  201_u8),
                            std::make_tuple(209_u8, 191_u8,  39_u8),
                            std::make_tuple(255_u8, 255_u8, 22_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "hotgreen" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(255_u8, 255_u8, 255_u8),
                            std::make_tuple(0_u8, 255_u8, 255_u8),
                            std::make_tuple(0_u8, 255_u8, 0_u8),
                            std::make_tuple(0_u8, 0_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "greenhot" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(0_u8, 40_u8, 10_u8),
                            std::make_tuple(0_u8, 200_u8, 30_u8),
                            std::make_tuple(100_u8, 255_u8, 200_u8),
                            std::make_tuple(240_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "valala" },
                    make_color_map
                    (
                        { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 50_u8), // 0.0
                            std::make_tuple(0_u8, 0_u8, 100_u8), // 0.1
                            std::make_tuple(0_u8, 0_u8, 150_u8), // 0.2
                            std::make_tuple(0_u8, 0_u8, 200_u8), // 0.3
                            std::make_tuple(0_u8, 0_u8, 255_u8), // 0.4
                            std::make_tuple(0_u8, 85_u8, 255_u8), // 0.5
                            std::make_tuple(0_u8, 170_u8, 255_u8), // 0.6
                            std::make_tuple(0_u8, 255_u8, 255_u8), // 0.7
                            std::make_tuple(85_u8, 255_u8, 255_u8), // 0.8
                            std::make_tuple(170_u8, 255_u8, 255_u8), // 0.9
                            std::make_tuple(255_u8, 255_u8, 255_u8) // 1.0
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "vala" },
                    make_color_map
                    (
                        { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 50_u8), // 0.0
                            std::make_tuple(0_u8, 0_u8, 100_u8), // 0.1
                            std::make_tuple(0_u8, 50_u8, 125_u8), // 0.2
                            std::make_tuple(50_u8, 150_u8, 200_u8), // 0.3
                            std::make_tuple(220_u8, 220_u8, 220_u8), // 0.4
                            std::make_tuple(240_u8, 240_u8, 240_u8), // 0.5
                            std::make_tuple(255_u8, 255_u8, 255_u8), // 0.6
                            std::make_tuple(255_u8, 100_u8, 195_u8), // 0.7
                            std::make_tuple(255_u8, 50_u8, 130_u8), // 0.8
                            std::make_tuple(255_u8, 0_u8, 65_u8), // 0.9
                            std::make_tuple(255_u8, 0_u8, 0_u8) // 1.0
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "hela" },
                    make_color_map
                    (
                        { 0.0, 0.4, 0.6, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8,  225_u8),
                            std::make_tuple(240_u8, 240_u8,  240_u8),
                            std::make_tuple(255_u8, 255_u8,  255_u8),
                            std::make_tuple(255_u8, 0_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "whela" },
                    make_color_map
                    (
                        { 0.0, 0.2, 0.4, 0.5, 0.6, 0.8, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8,  127_u8),
                            std::make_tuple(200_u8, 200_u8,  255_u8),
                            std::make_tuple(225_u8, 225_u8,  255_u8),
                            std::make_tuple(255_u8, 255_u8,  255_u8),
                            std::make_tuple(255_u8, 225_u8,  255_u8),
                            std::make_tuple(255_u8, 200_u8,  255_u8),
                            std::make_tuple(255_u8, 0_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "hotblue" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(255_u8, 255_u8,  255_u8),
                            std::make_tuple(0_u8, 255_u8,  255_u8),
                            std::make_tuple(0_u8, 0_u8,  255_u8),
                            std::make_tuple(0_u8, 0_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "bhot" },
                    make_color_map
                    (
                        { 0.0, 1.0/15.0, 1.0/7.0, 1.0/2.0, 1.0},
                        {
                            std::make_tuple(0_u8, 10_u8,  40_u8),
                            std::make_tuple(0_u8, 10_u8,  40_u8),
                            std::make_tuple(0_u8, 30_u8,  200_u8),
                            std::make_tuple(100_u8, 200_u8,  255_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "bluehot" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(0_u8, 10_u8,  40_u8),
                            std::make_tuple(0_u8, 30_u8,  200_u8),
                            std::make_tuple(100_u8, 200_u8,  255_u8),
                            std::make_tuple(240_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "bluewhite" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8,  0_u8),
                            std::make_tuple(25_u8, 100_u8,  255_u8),
                            std::make_tuple(150_u8, 200_u8,  255_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "bwhite" },
                    make_color_map
                    (
                        { 0.0, 1.0/5.0, 2.0/5.0, 3.0/5.0, 4.0/5.0, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8,  0_u8),
                            std::make_tuple(0_u8, 50_u8,  200_u8),
                            std::make_tuple(0_u8, 100_u8,  230_u8),
                            std::make_tuple(50_u8, 150_u8,  240_u8),
                            std::make_tuple(100_u8, 200_u8,  250_u8),
                            std::make_tuple(255_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "tealhot" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(0_u8, 40_u8,  40_u8),
                            std::make_tuple(0_u8, 200_u8,  200_u8),
                            std::make_tuple(100_u8, 255_u8,  255_u8),
                            std::make_tuple(240_u8, 255_u8, 255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "jet" },
                    make_color_map
                    (
                        { 0.0, 1.0/3.0, 2.0/3.0, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8,  255_u8),
                            std::make_tuple(0_u8, 255_u8,  255_u8),
                            std::make_tuple(255_u8, 255_u8,  0_u8),
                            std::make_tuple(255_u8, 0_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "obscure" },
                    make_color_map
                    (
                        { 0.0, 1.0/4.0, 1.0/2.0, 1.0},
                        {
                            std::make_tuple(255_u8, 255_u8,  255_u8),
                            std::make_tuple(0_u8, 0_u8,  194_u8),
                            std::make_tuple(85_u8, 255_u8,  128_u8),
                            std::make_tuple(255_u8, 0_u8, 0_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "gray" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(255_u8, 255_u8,  255_u8)
                        }
                    )
                ),
                std::make_pair
                (
                    std::string{ "grey" },
                    make_color_map
                    (
                        { 0.0, 1.0},
                        {
                            std::make_tuple(0_u8, 0_u8, 0_u8),
                            std::make_tuple(255_u8, 255_u8,  255_u8)
                        }
                    )
                )
            };

        }//namespace bmp_details

        template< template<class, class> class Matrix, template<class> class Allocator >
        std::optional<std::vector<std::uint8_t>> encode_bmp_stream(
                Matrix<std::uint8_t, Allocator<std::uint8_t>> const& channel_r,
                Matrix<std::uint8_t, Allocator<std::uint8_t>> const& channel_g,
                Matrix<std::uint8_t, Allocator<std::uint8_t>> const& channel_b
                ) noexcept
        {
            auto const [r_r, r_c] = channel_r.shape();
            auto const [g_r, g_c] = channel_g.shape();
            auto const [b_r, b_c] = channel_b.shape();
            // all 3 channels must be of the same size
            if ( r_r != g_r || g_r != b_r || r_c != g_c || g_c != b_c )
                return {std::nullopt};

            auto const [the_row, the_col] = std::make_tuple( r_r, r_c );
            std::uint_least64_t const padding_size  = ( 4 - ( ( the_col * 3 ) & 0x3 ) ) & 0x3;

            //generate header
            auto const& header = bmp_details::generate_bmp_header( the_row, the_col );

            // pre-allocate all memory for bmp mapping
            std::vector<std::uint8_t> encoding( header.size()+3*channel_r.size()+the_row*padding_size, std::uint8_t{} );
            std::copy( header.begin(), header.end(), encoding.begin() );

            auto&& fill_row = [&,the_col=the_col]( auto row_index )
            {
                auto start_pos = encoding.data() + header.size() + (padding_size + channel_r.col()*3) * row_index;
                for ( auto c : range( the_col ) )
                {
                    *start_pos++ =  channel_b[row_index][c];
                    *start_pos++ =  channel_g[row_index][c];
                    *start_pos++ =  channel_r[row_index][c];
                }
            };
            matrix_details::parallel( fill_row, 0UL, the_row, 0UL );

            return {encoding};
        }

        namespace for_each_impl_private
        {
            template < std::uint_least64_t Index, typename Type, typename... Types >
            struct extract_type_forward
            {
                typedef typename extract_type_forward < Index - 1, Types... >::result_type result_type;
            };
            template < typename Type, typename... Types >
            struct extract_type_forward< 1, Type, Types... >
            {
                typedef Type result_type;
            };
            template < typename Type, typename... Types >
            struct extract_type_forward< 0, Type, Types... >
            {
                struct index_parameter_for_extract_type_forwrod_should_not_be_0;
                typedef index_parameter_for_extract_type_forwrod_should_not_be_0 result_type;
            };
            template < std::uint_least64_t Index, typename... Types >
            struct extract_type_backward
            {
                typedef typename extract_type_forward < sizeof...( Types ) - Index + 1, Types... >::result_type result_type;
            };
            template < std::uint_least64_t Index, typename... Types >
            struct extract_type
            {
                typedef typename extract_type_forward< Index, Types... >::result_type result_type;
            };
            template < typename Function, typename InputIterator1, typename... InputIteratorn >
            Function _for_each_n( Function f, std::uint_least64_t n, InputIterator1 begin1, InputIteratorn... beginn )
            {
                auto const& func = [&]( std::uint_least64_t idx )
                {
                    f( *(begin1+idx), *(beginn+idx)... );
                };
                parallel( func, 0UL, n );
                return f;
            }
            template < typename Function, typename InputIterator1, typename... InputIteratorn >
            Function _for_each( Function f, InputIterator1 begin1, InputIterator1 end1, InputIteratorn... beginn )
            {
                return _for_each_n( f, std::distance( begin1, end1 ), begin1, beginn... );
                /*
                while ( begin1 != end1 )
                    f( *begin1++, *beginn++... );
                return f;
                */
            }

            struct dummy
            {
            };
            template < typename... Types_N >
            struct for_each_impl_with_dummy
            {
                typedef typename extract_type_backward< 1, Types_N... >::result_type return_type;
                template < typename Predict, typename... Types >
                Predict impl( Predict p, dummy, Types... types ) const
                {
                    return _for_each( p, types... );
                }
                template < typename S, typename... Types >
                return_type impl( S s, Types... types ) const
                {
                    return impl( types..., s );
                }
            };
        }

        template < typename... Types >
        typename for_each_impl_private::extract_type_backward< 1, Types... >::result_type
        for_each( Types... types )
        {
            static_assert( sizeof...( types ) > 2, "f::for_each requires at least 3 arguments" );
            return for_each_impl_private::for_each_impl_with_dummy< Types... >().impl( types..., for_each_impl_private::dummy() );
        }

        template<typename Iterator, typename Function >
        typename std::invoke_result<Function, typename std::iterator_traits<Iterator>::value_type, typename std::iterator_traits<Iterator>::value_type>::type
        reduce( Iterator begin, Iterator end, typename std::iterator_traits<Iterator>::value_type init, Function const& func )
        {
            typedef typename std::iterator_traits<Iterator>::value_type value_type;
            typedef typename std::invoke_result<Function, value_type, value_type>::type result_type;

            unsigned int const total_cores = std::thread::hardware_concurrency();
            unsigned long const total_elements = std::distance( begin, end );

            // case of small size, reduce inplace
            if (total_elements <= total_cores)
                return std::accumulate( begin, end, init, func );

            // parallel reduce to cache
            //std::vector<result_type> cache{ static_cast<unsigned long>(total_cores), static_cast<result_type>(init) };// of size `total_cores`, of same value `init`
            std::vector<result_type> cache;
            cache.resize( total_cores );
            std::fill( cache.begin(), cache.end(), init );
            auto const& thread_reducer = [&]( unsigned long idx )
            {
                for ( auto id = idx; id < total_elements; id += total_cores )
                    cache[idx] = func( cache[idx], *(begin+id) );
            };
            parallel( thread_reducer, static_cast<unsigned int>(0), total_cores, 0UL );

            // inplace reduce from cache
            return std::accumulate( cache.begin(), cache.end(), init, func );
        }

        static bool create_directory_if_not_present( std::string const& file_name )
        {
            std::filesystem::path file_path{ file_name };
            auto directory = file_path.parent_path();
            if ( ! std::filesystem::exists(directory) )
                return std::filesystem::create_directories(directory);
            return true;
        }

    }//namespace misc

    // for column, diagonal and anti-diagonal iteration
    template < typename Iterator_Type >
    struct stride_iterator
    {
        typedef stride_iterator                                                     self_type;
        typedef typename std::iterator_traits< Iterator_Type >::value_type          value_type;
        typedef typename std::iterator_traits< Iterator_Type >::reference           reference;
        typedef typename std::iterator_traits< Iterator_Type >::difference_type     difference_type;
        typedef typename std::iterator_traits< Iterator_Type >::pointer             pointer;
        typedef typename std::iterator_traits< Iterator_Type >::iterator_category   iterator_category;

        Iterator_Type                                                               iterator_;
        difference_type                                                             step_;

        stride_iterator( const Iterator_Type& it, const difference_type& dt ) noexcept : iterator_( it ) , step_( dt ) { }

        stride_iterator() noexcept : iterator_( 0 ) , step_( 1 ) { }
        stride_iterator( const self_type& ) noexcept = default;
        stride_iterator( self_type&& ) noexcept = default;
        self_type& operator=( const self_type& ) noexcept = default;
        self_type& operator=( self_type&& ) noexcept = default;

        self_type& operator++() noexcept
        {
            iterator_ += step_;
            return *this;
        }
        const self_type operator++( int ) noexcept
        {
            self_type ans( *this );
            operator++();
            return ans;
        }
        self_type& operator+=( const difference_type dt ) noexcept
        {
            iterator_ += dt * step_;
            return *this;
        }
        friend const self_type operator+( const self_type& lhs, const difference_type rhs ) noexcept
        {
            self_type ans( lhs );
            ans += rhs;
            return ans;
        }
        friend const self_type operator+( const difference_type lhs, const self_type& rhs ) noexcept
        {
            return rhs + lhs;
        }
        self_type& operator--() noexcept
        {
            iterator_ -= step_;
            return *this;
        }
        const self_type operator--( int ) noexcept
        {
            self_type ans( *this );
            operator--();
            return ans;
        }
        self_type& operator-=( const difference_type dt ) noexcept
        {
            iterator_ -= dt * step_;
            return *this;
        }
        friend const self_type operator-( const self_type& lhs, const difference_type rhs ) noexcept
        {
            self_type ans( lhs );
            ans -= rhs;
            return ans;
        }
        reference operator[]( const difference_type dt ) noexcept
        {
            return iterator_[dt * step_];
        }
        const reference operator[]( const difference_type dt ) const noexcept
        {
            return iterator_[dt * step_];
        }
        reference operator*() noexcept
        {
            return *iterator_;
        }
        const reference operator*() const noexcept
        {
            return *iterator_;
        }
        friend bool operator==( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ == rhs.iterator_;
        }
        friend bool operator!=( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return true;
            return lhs.iterator_ != rhs.iterator_;
        }
        friend bool operator<( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ < rhs.iterator_;
        }
        friend bool operator<=( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ <= rhs.iterator_;
        }
        friend bool operator>( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ > rhs.iterator_;
        }
        friend bool operator>=( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ >= rhs.iterator_;
        }
        friend difference_type operator-( const self_type& lhs, const self_type& rhs ) noexcept
        {
            better_assert( lhs.step_ == rhs.step_ && "stride iterators of different steps" );
            return ( lhs.iterator_ - rhs.iterator_ ) / lhs.step_;
        }
    };

    template < typename Type, typename Allocator >
    struct crtp_typedef
    {
        typedef typename std::decay< Type >::type                       value_type;
        typedef value_type*                                             iterator;
        typedef const value_type*                                       const_iterator;
        typedef Allocator                                               allocator_type;
        typedef std::uint_least64_t                                     size_type;
        typedef std::ptrdiff_t                                          difference_type;
        typedef std::pair<size_type, size_type>                         range_type;
        typedef typename Allocator::pointer                             pointer;
        typedef typename Allocator::const_pointer                       const_pointer;
        typedef stride_iterator< value_type* >                          matrix_stride_iterator;
        typedef value_type*                                             row_type;
        typedef const value_type*                                       const_row_type;
        typedef stride_iterator< value_type* >                          col_type;
        typedef stride_iterator< const value_type* >                    const_col_type;
        typedef stride_iterator< value_type* >                          diag_type;
        typedef stride_iterator< const value_type* >                    const_diag_type;
        typedef stride_iterator< value_type* >                          anti_diag_type;
        typedef stride_iterator< const value_type* >                    const_anti_diag_type;
        typedef std::reverse_iterator< iterator >                       reverse_iterator;
        typedef std::reverse_iterator< const_iterator >                 const_reverse_iterator;
        typedef std::reverse_iterator< matrix_stride_iterator >         reverse_matrix_stride_iterator;
        typedef std::reverse_iterator< row_type >                       reverse_row_type;
        typedef std::reverse_iterator< const_row_type >                 const_reverse_row_type;
        typedef std::reverse_iterator< col_type >                       reverse_col_type;
        typedef std::reverse_iterator< const_col_type >                 const_reverse_col_type;
        typedef std::reverse_iterator< diag_type >                      reverse_upper_diag_type;
        typedef std::reverse_iterator< const_diag_type >                const_reverse_upper_diag_type;
        typedef std::reverse_iterator< diag_type >                      reverse_lower_diag_type;
        typedef std::reverse_iterator< const_diag_type >                const_reverse_lower_diag_type;
        typedef std::reverse_iterator< diag_type >                      reverse_diag_type;
        typedef std::reverse_iterator< const_diag_type >                const_reverse_diag_type;
        typedef std::reverse_iterator< anti_diag_type >                 reverse_anti_diag_type;
        typedef std::reverse_iterator< const_anti_diag_type >           const_reverse_anti_diag_type;
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_item
    {
        typedef Matrix zen_type;
        auto item() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            auto const[row, col] = zen.shape();
            better_assert( row==1UL, "row of the matrix should be 1 to call item(), but got row=", row );
            better_assert( col==1UL, "column of the matrix should be 1 to call item(), but got column=", col );
            return zen[0][0];
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_shape
    {
        typedef Matrix zen_type;
        auto shape() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            return std::make_tuple( zen.row(), zen.col() );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    using crtp_shape_view = crtp_shape<Matrix, Type, Allocator>;

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_get_allocator
    {
        typedef Matrix zen_type;
        auto get_allocator() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            return zen.allocator_;
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_anti_diag_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::difference_type difference_type;
        typedef typename type_proxy_type::anti_diag_type anti_diag_type;
        typedef typename type_proxy_type::const_anti_diag_type const_anti_diag_type;
        typedef typename type_proxy_type::reverse_anti_diag_type reverse_anti_diag_type;
        typedef typename type_proxy_type::const_reverse_anti_diag_type const_reverse_anti_diag_type;
        anti_diag_type upper_anti_diag_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return anti_diag_type( zen.begin() + zen.col() - index - 1, zen.col() - 1 );
        }
        anti_diag_type upper_anti_diag_end( const size_type index = 0 ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type const depth = std::min( zen.col()-index, zen.row() );
            return upper_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type upper_anti_diag_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_anti_diag_type( zen.begin() + zen.col() - index - 1, zen.col() - 1 );
        }
        const_anti_diag_type upper_anti_diag_end( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type const depth = std::min( zen.col()-index, zen.row() );
            return upper_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type upper_anti_diag_cbegin( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_begin( index );
        }
        const_anti_diag_type upper_anti_diag_cend( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_end( index );
        }
        reverse_anti_diag_type upper_anti_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( upper_anti_diag_end( index ) );
        }
        reverse_anti_diag_type upper_anti_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( upper_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type upper_anti_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( upper_anti_diag_end( index ) );
        }
        const_reverse_anti_diag_type upper_anti_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( upper_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type upper_anti_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_rbegin( index );
        }
        const_reverse_anti_diag_type upper_anti_diag_crend( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_rend( index );
        }
        anti_diag_type lower_anti_diag_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return anti_diag_type( zen.begin() + ( zen.col() * ( index + 1 ) ) - 1, zen.col() - 1 );
        }
        anti_diag_type lower_anti_diag_end( const size_type index = 0 ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );

            size_type const depth = std::min( zen.row()-index, zen.col() );
            return lower_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type lower_anti_diag_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_anti_diag_type( zen.begin() + ( zen.col() * ( index + 1 ) ) - 1, zen.col() - 1 );
        }
        const_anti_diag_type lower_anti_diag_end( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );

            size_type const depth = std::min( zen.row()-index, zen.col() );
            return lower_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type lower_anti_diag_cbegin( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_begin( index );
        }
        const_anti_diag_type lower_anti_diag_cend( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_end( index );
        }
        reverse_anti_diag_type lower_anti_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( lower_anti_diag_end( index ) );
        }
        reverse_anti_diag_type lower_anti_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( lower_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type lower_anti_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( lower_anti_diag_end( index ) );
        }
        const_reverse_anti_diag_type lower_anti_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( lower_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type lower_anti_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_rbegin( index );
        }
        const_reverse_anti_diag_type lower_anti_diag_crend( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_rend( index );
        }
        anti_diag_type anti_diag_begin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_begin( index );
            }

            return lower_anti_diag_begin( -index );
        }
        anti_diag_type anti_diag_end( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_end( index );
            }

            return lower_anti_diag_end( -index );
        }
        const_anti_diag_type anti_diag_begin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_begin( index );
            }

            return lower_anti_diag_begin( -index );
        }
        const_anti_diag_type anti_diag_end( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_end( index );
            }

            return lower_anti_diag_end( -index );
        }
        const_anti_diag_type anti_diag_cbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_cbegin( index );
            }

            return lower_anti_diag_cbegin( -index );
        }
        const_anti_diag_type anti_diag_cend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_cend( index );
            }

            return lower_anti_diag_cend( -index );
        }
        reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rbegin( index );
            }

            return lower_anti_diag_rbegin( -index );
        }
        reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rend( index );
            }

            return lower_anti_diag_rend( -index );
        }
        const_reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rbegin( index );
            }

            return lower_anti_diag_rbegin( -index );
        }
        const_reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rend( index );
            }

            return lower_anti_diag_rend( -index );
        }
        const_reverse_anti_diag_type anti_diag_crbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_crbegin( index );
            }

            return lower_anti_diag_crbegin( -index );
        }
        const_reverse_anti_diag_type anti_diag_crend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_crend( index );
            }

            return lower_anti_diag_crend( -index );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_apply
    {
        typedef Matrix  zen_type;
        typedef Type    value_type;

        template < typename Function >
        void apply( const Function& func ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            value_type* x = zen.data();
            auto && parallel_function = [x, &func]( std::uint_least64_t offset ) { func( x[offset] ); };
            matrix_details::parallel( parallel_function, 0UL, zen.size(), 0UL );
        }

        template < typename Function >
        void elementwise_apply( const Function& func ) noexcept
        {
            apply( func );
        }

        template < typename Function >
        void map( const Function& func ) noexcept
        {
            apply( func );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_bracket_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        row_type operator[]( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            better_assert( index < zen.row() && "Row index outof boundary!", " with index ", index, " and zen.row() ", zen.row()  );
            return zen.row_begin( index );
        }
        const_row_type operator[]( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( index < zen.row() && "Row index outof boundary!", " with index ", index, " and zen.row() ", zen.row()  );
            return zen.row_cbegin( index );
        }
        value_type operator()( size_type r, size_type c ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( r < zen.row() && "Row index out of boundary!" );
            better_assert( c < zen.col() && "Column index out of boundary!" );
            return zen[r][c];
        }
        value_type& operator()( size_type r, size_type c ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            better_assert( r < zen.row() && "Row index out of boundary!" );
            better_assert( c < zen.col() && "Column index out of boundary!" );
            return zen[r][c];
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_bracket_operator_view
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        const_row_type operator[]( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( index < zen.row() && "Row index outof boundary!" );
            return zen.row_cbegin( index );
        }
        value_type operator()( size_type r, size_type c ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( r < zen.row() && "Row index out of boundary!" );
            better_assert( c < zen.col() && "Column index out of boundary!" );
            return zen[r][c];
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_clear
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        void clear() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            zen.get_allocator().deallocate( zen.data(), zen.size() );
            zen.dat_ = nullptr;
            zen.row_ = 0;
            zen.col_ = 0;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_clone
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        template < typename Other_Matrix >
        zen_type& clone( const Other_Matrix& other, std::initializer_list<size_type> r, std::initializer_list<size_type> c ) noexcept
        {
            better_assert( r.size() == 2 && "row size should be 2!" );
            better_assert( c.size() == 2 && "col size should be 2!" );
            return clone( other, r[0], r[1], c[0], c[1] );
        }
        template < typename Other_Matrix >
        zen_type& clone( const Other_Matrix& other, size_type const r0, size_type const r1, size_type const c0, size_type const c1 ) noexcept
        {
            better_assert( r1 > r0 && "row range error!" );
            better_assert( c1 > c0 && "col range error!" );

            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type tmp{ zen.get_allocator(), r1-r0, c1-c0 };

            auto && parallel_function = [&]( std::uint_least64_t r )
            {
                std::copy_n( other.row_begin(r+r0)+c0, tmp.col(), tmp.row_begin(r) );
            };

            matrix_details::parallel( parallel_function, 0UL, tmp.row(), 0UL );

            zen.swap( tmp );
            return zen;
        }

        zen_type const clone( std::initializer_list<size_type> r, std::initializer_list<size_type> c ) const noexcept
        {
            better_assert( r.size() == 2 && "row size should be 2!" );
            better_assert( c.size() == 2 && "col size should be 2!" );
            auto const [r0, r1] = std::tuple{ *r.begin(), *(r.begin()+1) };
            auto const [c0, c1] = std::tuple{ *c.begin(), *(c.begin()+1) };

            return clone( r0, r1, c0, c1 );
        }
        zen_type const clone( size_type const r0, size_type const r1, size_type const c0, size_type const c1 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            zen_type ans{ zen.get_allocator() };
            ans.clone( zen, r0, r1, c0, c1 );
            return ans;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_col_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::col_type col_type;
        typedef typename type_proxy_type::const_col_type const_col_type;
        typedef typename type_proxy_type::reverse_col_type reverse_col_type;
        typedef typename type_proxy_type::const_reverse_col_type const_reverse_col_type;
        col_type col_begin( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return col_type( zen.row_begin(0) + index, zen.col() );
        }
        col_type col_end( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return col_begin( index ) + zen.row();
        }
        const_col_type col_begin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_col_type( zen.row_begin(0) + index, zen.col() );
        }
        const_col_type col_end( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.col_begin( index ) + zen.row();
        }
        const_col_type col_cbegin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_col_type( zen.begin() + index, zen.col() );
        }
        const_col_type col_cend( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.col_begin( index ) + zen.row();
        }
        reverse_col_type col_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_col_type( col_end( index ) );
        }
        reverse_col_type col_rend( const size_type index = 0 ) noexcept
        {
            return reverse_col_type( col_begin( index ) );
        }
        const_reverse_col_type col_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_end( index ) );
        }
        const_reverse_col_type col_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_begin( index ) );
        }
        const_reverse_col_type col_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_end( index ) );
        }
        const_reverse_col_type col_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_begin( index ) );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    using crtp_col_iterator_view =  crtp_col_iterator<Matrix, Type, Allocator>;

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_copy
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        template < typename Other_Matrix >
        void copy( const Other_Matrix& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen.allocator_ = rhs.allocator_;
            zen.resize( rhs.row(), rhs.col() );
            std::copy( rhs.begin(), rhs.end(), zen.begin() );//<- should be overloaded when with cuda_allocator
            // TODO: copy-and-swap
        }

        // copy 'other' matrix, placing in position [ (r0, r1), (c0, c1) ]
        template < typename Other_Matrix >
        void copy( const Other_Matrix& other, std::initializer_list<size_type> r, std::initializer_list<size_type> c ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            better_assert( r.size() == 2, " expecting 2 arguments inside r, but received ", r.size(), " parameters." );
            better_assert( c.size() == 2, " expecting 2 arguments inside r, but received ", c.size(), " parameters." );

            auto [r0, r1] = std::tuple{ *r.begin(), *(r.begin()+1) };
            auto [c0, c1] = std::tuple{ *c.begin(), *(c.begin()+1) };

            better_assert( r1 <= zen.row(), " expecting matrix row no less than row arg, but the matrix row is ", zen.row(), " and the row arg is ", r1 );
            better_assert( c1 <= zen.col(), " expecting matrix col no less than col arg, but the matrix col is ", zen.col(), " and the row arg is ", c1 );

            better_assert( r0 <= r1, " first row arg is larger than the second! The first arg is ", r0, " and the second arg is ", r1 );
            better_assert( c0 <= c1, " first col arg is larger than the second! The first arg is ", r0, " and the second arg is ", r1 );

            better_assert( r1 - r0 == other.row(), " row dim does not match, expected ", other.row(), " rows, but passed parameters are ", r0, " and ", r1 );
            better_assert( c1 - c0 == other.col(), " col dim does not match, expected ", other.col(), " cols, but passed parameters are ", c0, " and ", c1 );

            auto const& copy_function = [&, r0=r0, c0=c0]( size_type const row_index )
            {
                std::copy( other.row_begin(row_index), other.row_end(row_index), zen.row_begin(r0+row_index)+c0 );
            };
            matrix_details::parallel( copy_function, 0UL, other.row(), 0UL );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_data
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::pointer pointer;
        typedef typename type_proxy_type::const_pointer const_pointer;
        pointer data() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.dat_;
        }
        const_pointer data() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.dat_;
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_det
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::range_type range_type;
        value_type det() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row()==zen.col(), " matrix::det(), the row and matrix are supposed to be same, but now row is ", zen.row(), " and col is ", zen.col() );

            if ( 0 == zen.size() )
            {
                return value_type{};
            }

            if ( 1 == zen.size() )
            {
                return *( zen.begin() );
            }

            if ( 4 == zen.size() )
            {
                return zen[0][0] * zen[1][1] - zen[1][0] * zen[0][1];
            }

            size_type const n = zen.row();
            size_type const m = n >> 1;
            zen_type const P( zen, range_type( 0, m ), range_type( 0, m ) );
            zen_type const Q( zen, range_type( 0, m ), range_type( m, n ) );
            zen_type const R( zen, range_type( m, n ), range_type( 0, m ) );
            zen_type const S( zen, range_type( m, n ), range_type( m, n ) );
            zen_type const& tmp = S - ( R * ( P.inverse() ) * Q );

            return P.det() * tmp.det();
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_diag_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::difference_type difference_type;
        typedef typename type_proxy_type::diag_type diag_type;
        typedef typename type_proxy_type::const_diag_type const_diag_type;
        typedef typename type_proxy_type::reverse_upper_diag_type reverse_upper_diag_type;
        typedef typename type_proxy_type::const_reverse_upper_diag_type const_reverse_upper_diag_type;
        typedef typename type_proxy_type::reverse_lower_diag_type reverse_lower_diag_type;
        typedef typename type_proxy_type::const_reverse_lower_diag_type const_reverse_lower_diag_type;
        typedef typename type_proxy_type::reverse_diag_type reverse_diag_type;
        typedef typename type_proxy_type::const_reverse_diag_type const_reverse_diag_type;
        diag_type upper_diag_begin( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return diag_type( zen.begin() + index, zen.col() + 1 );
        }
        diag_type upper_diag_end( const size_type index ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type depth = std::min( zen.col()-index, zen.row() );
            return diag_type( upper_diag_begin( index ) + depth );
        }
        const_diag_type upper_diag_begin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.begin() + index, zen.col() + 1 );
        }
        const_diag_type upper_diag_end( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.col()-index, zen.row() );
            return upper_diag_begin( index ) + depth;
        }
        const_diag_type upper_diag_cbegin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.cbegin() + index, zen.col() + 1 );
        }
        const_diag_type upper_diag_cend( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.col()-index, zen.row() );
            return upper_diag_cbegin( index ) + depth;
        }
        reverse_upper_diag_type upper_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_upper_diag_type( upper_diag_end( index ) );
        }
        reverse_upper_diag_type upper_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_upper_diag_type( upper_diag_begin( index ) );
        }
        const_reverse_upper_diag_type upper_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_end( index ) );
        }
        const_reverse_upper_diag_type upper_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_begin( index ) );
        }
        const_reverse_upper_diag_type upper_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_end( index ) );
        }
        const_reverse_upper_diag_type upper_diag_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_begin( index ) );
        }
        diag_type lower_diag_begin( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }
        diag_type lower_diag_end( const size_type index ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type depth = std::min( zen.row()-index, zen.col() );
            return lower_diag_begin( index ) + depth;
        }
        const_diag_type lower_diag_begin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }
        const_diag_type lower_diag_end( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.row()-index, zen.col() );
            return lower_diag_begin( index ) + depth;
        }
        const_diag_type lower_diag_cbegin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }
        const_diag_type lower_diag_cend( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.row()-index, zen.col() );
            return lower_diag_begin( index ) + depth;
        }
        reverse_lower_diag_type lower_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_lower_diag_type( lower_diag_end( index ) );
        }
        reverse_lower_diag_type lower_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_lower_diag_type( lower_diag_begin( index ) );
        }
        const_reverse_lower_diag_type lower_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_end( index ) );
        }
        const_reverse_lower_diag_type lower_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_begin( index ) );
        }
        const_reverse_lower_diag_type lower_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_end( index ) );
        }
        const_reverse_lower_diag_type lower_diag_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_begin( index ) );
        }
        diag_type diag_begin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_begin( index );
            return lower_diag_begin( -index );
        }
        diag_type diag_end( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_end( index );
            return lower_diag_end( -index );
        }
        const_diag_type diag_begin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_begin( index );
            return lower_diag_begin( -index );
        }
        const_diag_type diag_end( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_end( index );
            return lower_diag_end( -index );
        }
        const_diag_type diag_cbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_cbegin( index );
            return lower_diag_cbegin( -index );
        }
        const_diag_type diag_cend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_cend( index );
            return lower_diag_cend( -index );
        }
        reverse_diag_type diag_rbegin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_rbegin( index );
            return lower_diag_rbegin( -index );
        }
        reverse_diag_type diag_rend( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_rend( index );
            return lower_diag_rend( -index );
        }
        const_reverse_diag_type diag_rbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_rbegin( index );
            return lower_diag_rbegin( -index );
        }
        const_reverse_diag_type diag_rend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_rend( index );
            return lower_diag_rend( -index );
        }
        const_reverse_diag_type diag_crbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_crbegin( index );
            return lower_diag_crbegin( -index );
        }
        const_reverse_diag_type diag_crend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_crend( index );
            return lower_diag_crend( -index );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_direct_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::iterator iterator;
        typedef typename type_proxy_type::reverse_iterator reverse_iterator;
        typedef typename type_proxy_type::const_iterator const_iterator;
        typedef typename type_proxy_type::const_reverse_iterator const_reverse_iterator;
        iterator begin() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.data();
        }
        iterator end() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.begin()+zen.size();
        }
        const_iterator begin() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.data();
        }
        const_iterator end() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.begin()+zen.size();
        }
        const_iterator cbegin() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.data();
        }
        const_iterator cend() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.begin()+zen.size();
        }
        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator( end() );
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator( begin() );
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator( end() );
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator( begin() );
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator( end() );
        }
        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator( begin() );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_divide_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& operator/=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen.elementwise_apply( [&rhs]( value_type& v ) { v /= rhs; } );
            return zen;
        }
        zen_type& operator/=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen *= rhs.inverse();
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_inverse
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::range_type range_type;
        // TODO: block-wise inverse here
        const zen_type inverse() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row() == zen.col(), " Expecting square matrix, but now with row = ", zen.row(), " and col = ", zen.col() );

            // case of empty matrix
            if ( zen.size() == 0 )
                return zen_type{};

            if ( zen.size() == 1 )
                return zen_type{ 1, 1, {value_type{1} / zen[0][0]} };

            if ( zen.size() == 4 )
            {
                auto const [a, b, c, d] = std::make_tuple( zen[0][0], zen[0][1], zen[1][0], zen[1][1] );
                value_type const factor = a*d - b*c;
                return zen_type{ 2, 2, { d / factor, -b / factor, -c / factor, a / factor } };
            }

            size_type const N = zen.row();
            size_type const n = N >> 1;
            zen_type const& A = zen.clone({0, n}, {0, n}); zen_type const& B = zen.clone({0, n}, {n, N});
            zen_type const& C = zen.clone({n, N}, {0, n}); zen_type const& D = zen.clone({n, N}, {n, N});

            auto const& D_ = D.inverse();
            auto const& D_C = D_ * C;
            auto const& BD_C = B * D_C;
            auto const& A__BD_C = A - BD_C;
            auto const& A__BD_C_ = A__BD_C.inverse();
            auto const& BD_ = B * D_;
            auto const& A__BD_C_BD_ = A__BD_C_ * BD_;

            auto const& E = A__BD_C_;           auto const& F = -A__BD_C_BD_;
            auto const& G = -D_C * A__BD_C_;    auto const& H = D_ + D_C * A__BD_C_BD_;

            zen_type ans{ N, N };
            ans.copy( E, {0, n}, {0, n} ); ans.copy( F, {0, n}, {n, N} );
            ans.copy( G, {n, N}, {0, n} ); ans.copy( H, {n, N}, {n, N} );
            return ans;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_load_txt
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Allocator >::value_type value_type;
        typedef typename crtp_typedef< Type, Allocator >::size_type size_type;
        bool load_txt( std::string const& file_name ) noexcept
        {
            return load_txt(file_name.c_str());
        }
        bool load_txt( const char* const file_name ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::ifstream ifs( file_name );
            better_assert( ifs && "matrix::load_txt -- failed to open file" );
            if (!ifs.good())
            {
                std::cerr << "matrix::load_txt -- Failed to open file " << file_name << "\n";
                return false;
            }
            std::stringstream iss;
            std::copy( std::istreambuf_iterator< char >( ifs ), std::istreambuf_iterator< char >(), std::ostreambuf_iterator< char >( iss ) ); //TODO: parallel here?
            std::string cache = iss.str();
            //for_each( cache.begin(), cache.end(), []( auto & ch ) { if ( ch == ',' || ch == ';' ) ch = ' '; } );
            auto && replace_delimiter_func = [&cache]( size_type idx ){ auto& ch = cache[idx]; ch = (ch==','||ch==';') ? ' ' : ch; };
            matrix_details::parallel( replace_delimiter_func, 0UL, cache.size(), 0UL );

            iss.str( cache );
            std::vector< value_type > buff;
            std::copy( std::istream_iterator< value_type >( iss ), std::istream_iterator< value_type >(), std::back_inserter( buff ) ); // TODO: parallel here?
            size_type const total_elements = buff.size();
            const std::string& stream_buff = iss.str();
            size_type const r_             = std::count( stream_buff.begin(), stream_buff.end(), '\n' );
            size_type const r              = *( stream_buff.rbegin() ) == '\n' ? r_ : r_ + 1;
            size_type const c              = total_elements / r;

            if ( r * c != total_elements )
            {
                std::cerr << "Error: Failed to match matrix size.\n \tthe size of matrix stored in file \"" << file_name << "\" is " << buff.size() << ".\n";
                std::cerr << " \tthe size of the destination matrix is " << r << " by " << c << " elements.\n";
                return false;
            }

            zen.resize( r, c );
            std::copy( buff.begin(), buff.end(), zen.begin() );
            ifs.close();
            return true;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_load_binary
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Allocator >::value_type value_type;
        typedef typename crtp_typedef< Type, Allocator >::size_type size_type;
        bool load_binary( std::string const& file_name ) noexcept
        {
            return load_binary( file_name.c_str() );
        }
        bool load_binary( char const* const file_name ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::ifstream ifs( file_name, std::ios::binary );
            better_assert( ifs && "matrix::load_binary -- failed to open file" );

            if ( !ifs )
                return false;

            std::vector< char > buffer{ ( std::istreambuf_iterator< char >( ifs ) ), ( std::istreambuf_iterator< char >() ) };
            better_assert( buffer.size() >= sizeof( size_type ) + sizeof( size_type ) && "matrix::load_library -- file too small, maybe be damaged" );

            if ( buffer.size() <= sizeof( size_type ) + sizeof( size_type ) )
                return false;

            size_type r;
            std::copy( buffer.begin(), buffer.begin() + sizeof( r ), reinterpret_cast< std::int8_t* >( std::addressof( r ) ) );
            size_type c;
            std::copy( buffer.begin() + sizeof( r ), buffer.begin() + sizeof( r ) + sizeof( c ), reinterpret_cast< std::int8_t* >( std::addressof( c ) ) );
            zen.resize( r, c );
            better_assert( buffer.size() == sizeof( r ) + sizeof( c ) + sizeof( Type ) * zen.size() && "matrix::load_binary -- data does not match, maybe damaged" );

            if ( buffer.size() != sizeof( r ) + sizeof( c ) + sizeof( Type ) * zen.size() )
                return false;

            std::copy( buffer.begin() + sizeof( r ) + sizeof( c ), buffer.end(), reinterpret_cast< std::int8_t* >( zen.data() ) );
            return true;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_load_npy
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Allocator >::value_type value_type;
        typedef typename crtp_typedef< Type, Allocator >::size_type size_type;
        bool load_npy( std::string const& file_name ) noexcept
        {
            return load_npy( file_name.c_str() );
        }
        bool load_npy( char const* const file_name ) noexcept
        {
            if constexpr (debug_mode)
                std::cout << "Loading npy file from " << file_name << std::endl;

            zen_type& zen = static_cast< zen_type& >( *this );
            std::ifstream ifs( file_name, std::ios::binary );
            better_assert( ifs, "matrix::load_npy -- failed to open file ", file_name );

            if ( !ifs )
                return false;

            std::vector< char > buffer{ ( std::istreambuf_iterator< char >( ifs ) ), ( std::istreambuf_iterator< char >() ) };

            //get version
            std::size_t const version = *(reinterpret_cast< std::uint8_t* >( buffer.data()+6 ));
            if constexpr (debug_mode)
                std::cout << "Version: " << version << std::endl;

            //get header length and header
            std::uint32_t header_length;
            std::string header;
            if ( version == 1 ) //version 1 using 2 bytes
            {
                std::uint32_t const _l0 = *(reinterpret_cast<std::uint8_t*>( buffer.data() + 8 ));
                std::uint32_t const _l1 = *(reinterpret_cast<std::uint8_t*>( buffer.data() + 9 ));
                header_length = (_l1 << 8) + _l0;
                header = std::string{ buffer.data() + 10, buffer.data() + 10 + header_length };
            }
            else //version 2/3 using 4 bytes
            {
                std::uint32_t const _l0 = *(reinterpret_cast<unsigned char*>( buffer.data() + 8 ));
                std::uint32_t const _l1 = *(reinterpret_cast<unsigned char*>( buffer.data() + 9 ));
                std::uint32_t const _l2 = *(reinterpret_cast<unsigned char*>( buffer.data() + 10 ));
                std::uint32_t const _l3 = *(reinterpret_cast<unsigned char*>( buffer.data() + 11 ));
                header_length = (_l3 << 24) + (_l2 << 16) + (_l1 << 8) + _l0;
                header = std::string{ buffer.data() + 12, buffer.data() + 12 + header_length };
            }
            if constexpr (debug_mode)
                std::cout << "Extracted npy file header:\n" << header << std::endl;

            // fortran format or not
            bool const row_major = ( header.find("T") != std::string::npos ) ? false : true;
            if constexpr (debug_mode)
                std::cout << "The stored matrix is row major: " << row_major << std::endl;

            //extract row and column
            std::size_t const shape_pos = header.find("'shape': (");
            std::size_t const row_pos = shape_pos + 10; //start of row
            std::size_t const row_pos_end = header.find( ",", row_pos ); //end of row
            std::string const row_string = header.substr( row_pos, row_pos_end - row_pos );
            std::size_t const row = std::stoul( row_string );
            std::size_t const col_pos = row_pos_end + 1; //start of col
            std::size_t const col_pos_end = header.find( ")", col_pos ); //end of col
            std::string const col_string = header.substr( col_pos, col_pos_end - col_pos );
            std::size_t const col = std::stoul( col_string );
            if constexpr (debug_mode)
                std::cout << "extracted row: " << row << " and col: " << col << std::endl;

            //resize matrix
            zen.resize( row, col );
            if (!row_major)
                zen.reshape( col, row );

            //copy binary value
            std::size_t const data_offset = (version==1) ? (10 + header_length) : (12 +header_length);
            std::copy_n( reinterpret_cast<value_type*>(buffer.data()+data_offset), row*col, zen.data() );

            return true;
        }
    };//struct crtp_load_npy
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_minus_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;

        zen_type& operator-=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen.elementwise_apply( [&rhs]( value_type& v) { v -= rhs; } );
            return zen;
        }

        zen_type& operator-=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            auto v = zen.data();
            auto x = rhs.data();
            auto const& elementwise_minus = [v, x]( size_type offset )
            {
                v[offset] -= x[offset];
            };
            matrix_details::parallel( elementwise_minus, 0UL, zen.size(), 0UL );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_multiply_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::range_type range_type;
        zen_type& operator*=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            zen.elementwise_apply( [&rhs]( value_type& v ) { v *= rhs; } );

            return zen;
        }
        zen_type& direct_multiply( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            better_assert( zen.col() == other.row() && "direct_multiply: dimesion not match!" );
            zen_type tmp( zen.row(), other.col() );

            auto const& func = [&]( size_type i )
            {
                for ( size_type j = 0; j != tmp.col(); ++j )
                    tmp[i][j] = std::inner_product( zen.row_begin( i ), zen.row_end( i ), other.col_begin( j ), value_type( 0 ) );
            };
            matrix_details::parallel( func, 0UL, tmp.row(), 0UL );
            zen.swap( tmp );
            return zen;
        }
        zen_type& rr1( const zen_type& other ) noexcept
        {
            zen_type& zen            = static_cast< zen_type& >( *this );
            const zen_type& new_this = zen && value_type( 0 );
            const zen_type& new_ans  = new_this * other;
            zen.clone( new_ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& rr2( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            const zen_type new_this( zen, range_type( 0, zen.row() - 1 ), range_type( 0, zen.col() ) );
            const zen_type last_row( zen, range_type( zen.row() - 1, zen.row() ), range_type( 0, zen.col() ) );
            const zen_type& new_ans      = new_this * other;
            const zen_type& last_row_ans = last_row * other;
            const zen_type& ans          = new_ans && last_row_ans;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& cc1( const zen_type& other ) noexcept
        {
            zen_type& zen             = static_cast< zen_type& >( *this );
            const zen_type& new_this  = zen || value_type( 0 );
            const zen_type& new_other = other && value_type( 0 );
            const zen_type& ans       = new_this * new_other;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& cc2( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            const zen_type new_this( zen, range_type( 0, zen.row() ), range_type( 0, zen.col() - 1 ) );
            const zen_type last_col( zen, range_type( 0, zen.row() ), range_type( zen.col() - 1, zen.col() ) );
            const zen_type new_other( other, range_type( 0, other.row() - 1 ), range_type( 0, other.col() ) );
            const zen_type last_row( other, range_type( other.row() - 1, other.row() ), range_type( 0, other.col() ) );
            const zen_type& new_ans     = new_this * new_other;
            const zen_type& res_col_row = last_col * last_row;
            const zen_type& ans         = new_ans + res_col_row;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& oc1( const zen_type& other ) noexcept
        {
            zen_type& zen             = static_cast< zen_type& >( *this );
            const zen_type& new_other = other || value_type( 0 );
            const zen_type& new_ans   = zen * new_other;
            zen.clone( new_ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& oc2( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            const zen_type new_other( other, range_type( 0, other.row() ), range_type( 0, other.col() - 1 ) );
            const zen_type last_col( other, range_type( 0, other.row() ), range_type( other.col() - 1, other.col() ) );
            const zen_type& new_ans      = zen * new_other;
            const zen_type& last_col_ans = zen * last_col;
            const zen_type& ans          = new_ans || last_col_ans;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& strassen_multiply( const zen_type& other ) noexcept
        {
            zen_type& zen        = static_cast< zen_type& >( *this );
            const size_type R_2  = zen.row() >> 1;
            const size_type C_2  = zen.col() >> 1;
            const size_type OR_2 = C_2;
            const size_type OC_2 = other.col() >> 1;
            const zen_type a_00( zen, range_type( 0, R_2 ), range_type( 0, C_2 ) );
            const zen_type a_01( zen, range_type( 0, R_2 ), range_type( C_2, zen.col() ) );
            const zen_type a_10( zen, range_type( R_2, zen.row() ), range_type( 0, C_2 ) );
            const zen_type a_11( zen, range_type( R_2, zen.row() ), range_type( C_2, zen.col() ) );
            const zen_type b_00( other, range_type( 0, OR_2 ), range_type( 0, OC_2 ) );
            const zen_type b_01( other, range_type( 0, OR_2 ), range_type( OC_2, other.col() ) );
            const zen_type b_10( other, range_type( OR_2, other.row() ), range_type( 0, OC_2 ) );
            const zen_type b_11( other, range_type( OR_2, other.row() ), range_type( OC_2, other.col() ) );
            const zen_type& Q_0  = ( a_00 + a_11 ) * ( b_00 + b_11 );
            const zen_type& Q_1  = ( a_10 + a_11 ) * b_00;
            const zen_type& Q_2  = a_00 * ( b_01 - b_11 );
            const zen_type& Q_3  = a_11 * ( -b_00 + b_10 );
            const zen_type& Q_4  = ( a_00 + a_01 ) * b_11;
            const zen_type& Q_5  = ( -a_00 + a_10 ) * ( b_00 + b_01 );
            const zen_type& Q_6  = ( a_01 - a_11 ) * ( b_10 + b_11 );
            const zen_type& c_00 = Q_0 + Q_3 - Q_4 + Q_6;
            const zen_type& c_10 = Q_1 + Q_3;
            const zen_type& c_01 = Q_2 + Q_4;
            const zen_type& c_11 = Q_0 - Q_1 + Q_2 + Q_5;
            const zen_type& ans  = ( c_00 || c_01 ) && ( c_10 || c_11 );
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& operator*=( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            better_assert( zen.col() == other.row() && "operator *= :: the matrix dims not match!" );

            if constexpr( parallel_mode )
                return direct_multiply( other );

            static const size_type threshold = 17;
            const size_type max_dims         = std::max( std::max( zen.row(), zen.col() ), other.col() );
            const size_type min_dims         = std::min( std::min( zen.row(), zen.col() ), other.col() );

            if ( ( max_dims < threshold ) || ( min_dims == 1 ) )
            {
                return direct_multiply( other );
            }

            const size_type R  = zen.row();
            const size_type C  = zen.col();
            const size_type OC = other.col();

            if ( R & 1 )
            {
                if ( R & 2 )
                {
                    return rr1( other );
                }

                return rr2( other );
            }

            if ( C & 1 )
            {
                if ( C & 2 )
                {
                    return cc1( other );
                }

                return cc2( other );
            }

            if ( OC & 1 )
            {
                if ( OC & 2 )
                {
                    return oc1( other );
                }

                return oc2( other );
            }

            return strassen_multiply( other );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_plus_equal_operator
    {
        typedef Matrix                                  zen_type;
        typedef crtp_typedef< Type, Allocator >         type_proxy_type;
        typedef typename type_proxy_type::value_type    value_type;
        typedef typename type_proxy_type::size_type     size_type;
        zen_type& operator+=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            zen.elementwise_apply( [&rhs]( auto& v ){ v += rhs; } );

            return zen;
        }
        zen_type& operator+=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            auto x = zen.data();
            auto y = rhs.data();
            auto const& elementwise_add = [x, y]( size_type offset )
            {
                x[offset] += y[offset];
            };
            matrix_details::parallel( elementwise_add, 0UL, zen.size(), 0UL );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_prefix_minus
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        const zen_type operator-() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            zen_type ans{ zen };
            //std::transform( ans.begin(), ans.end(), ans.begin(), []( value_type x ) { return -x; } );
            auto x = ans.data();
            auto const& minus_function = [x]( size_type offset )
            {
                x[offset] = -x[offset];
            };
            matrix_details::parallel( minus_function, 0UL, ans.size(), 0UL );
            return ans;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_prefix_plus
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        const zen_type operator+() const noexcept
        {
            return static_cast< zen_type const& >( *this );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_reshape
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        zen_type& reshape( const size_type new_row, const size_type new_col ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            better_assert( new_row * new_col == zen.row() * zen.col() && "error: size before and after reshape does not agree, use resize() instead!" );
            zen.row_ = new_row;
            zen.col_ = new_col;
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_resize
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& resize( const size_type new_row, const size_type new_col ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            if ( zen.size() == new_row * new_col )
            {
                zen.reshape( new_row, new_col );
                return zen;
            }

            zen_type ans{ zen.get_allocator(), new_row, new_col };
            zen.swap( ans );
            return zen;
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_row_col_size
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        size_type row() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.row_;
        }
        size_type col() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.col_;
        }
        size_type size() const noexcept
        {
            return row()*col();
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_row_col_size_view
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        size_type row() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.row_dim_.second - zen.row_dim_.first;
        }
        size_type col() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.col_dim_.second - zen.col_dim_.first;
        }
        size_type size() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.row()*zen.col();
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_row_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        typedef typename type_proxy_type::reverse_row_type reverse_row_type;
        typedef typename type_proxy_type::const_reverse_row_type const_reverse_row_type;

        row_type row_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return row_type{ zen.begin() + index * zen.col() };
        }
        row_type row_end( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.row_begin( index ) + zen.col();
        }
        const_row_type row_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_row_type{ zen.begin() + index * zen.col() };
        }
        const_row_type row_end( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.row_begin( index ) + zen.col();
        }
        const_row_type row_cbegin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_row_type( zen.begin() + index * zen.col() );
        }
        const_row_type row_cend( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.row_begin( index ) + zen.col();
        }
        reverse_row_type row_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_row_type( row_end( index ) );
        }
        reverse_row_type row_rend( const size_type index = 0 ) noexcept
        {
            return reverse_row_type( row_begin( index ) );
        }
        const_reverse_row_type row_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_end( index ) );
        }
        const_reverse_row_type row_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_begin( index ) );
        }
        const_reverse_row_type row_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_end( index ) );
        }
        const_reverse_row_type row_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_begin( index ) );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_row_iterator_view : crtp_row_iterator<Matrix, Type, Allocator >
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        typedef typename type_proxy_type::reverse_row_type reverse_row_type;
        typedef typename type_proxy_type::const_reverse_row_type const_reverse_row_type;
/*
        auto row_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            size_type const row_offset = zen.row_dim_.first;
            size_type const col_offset = zen.col_dim_.first;
            return zen.matrix_.row_begin(row_offset+index) + col_offset;
        }
  */
        auto row_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type const row_offset = zen.row_dim_.first;
            size_type const col_offset = zen.col_dim_.first;
            return zen.matrix_.row_cbegin(row_offset+index) + col_offset;
        }

        auto row_cbegin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type const row_offset = zen.row_dim_.first;
            size_type const col_offset = zen.col_dim_.first;
            return zen.matrix_.row_cbegin(row_offset+index) + col_offset;
        }

    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_txt
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        bool save_as_txt( std::string const& file_name ) const noexcept
        {
            return save_as_txt( file_name.c_str() );
        }
        bool save_as_txt( char const * const file_name ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );

            if ( !matrix_details::create_directory_if_not_present( file_name ) )
                better_assert( !"save_as_txt", " failed to create parent directory: ", " with the target file name ", file_name );

            std::ofstream ofs( file_name );
            better_assert( ofs, " with the target file name is ", file_name );

            if ( !ofs ) return false;

            ofs.precision( 16 );
            ofs << zen;
            ofs.close();
            return true;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_binary
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        bool save_as_binary( std::string const& file_name ) const
        {
            return save_as_binary( file_name.c_str() );
        }
        bool save_as_binary( char const* const file_name ) const
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );

            if ( !matrix_details::create_directory_if_not_present( file_name ) )
                better_assert( !"save_as_binary", " failed to create directory: ", " with the target file name ", file_name );

            std::ofstream ofs( file_name, std::ios::out | std::ios::binary );
            better_assert( ofs, " save_as_binary failed to create file: ", " with the target file name ", file_name );

            if ( !ofs ) return false;

            auto const r = zen.row();
            ofs.write( reinterpret_cast< char const* >( std::addressof( r ) ), sizeof( r ) );
            auto const c = zen.col();
            ofs.write( reinterpret_cast< char const* >( std::addressof( c ) ), sizeof( c ) );
            ofs.write( reinterpret_cast< char const* >( zen.data() ), sizeof( Type ) * zen.size() );

            better_assert( ofs.good(), " save_as_binary failed to write: ", " with the target file name ", file_name );
            if ( !ofs.good() ) return false;

            ofs.close();
            return true;
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_plot
    {
        typedef Matrix zen_type;
        bool plot( std::string const& file_name, std::string const& color_map=std::string{"parula"} ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.save_as_bmp( file_name, color_map );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_bmp
    {
        typedef Matrix zen_type;
        bool save_as_bmp( const std::string& file_name, std::string const& color_map = std::string{ "parula" } ) const
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row() && "save_as_bmp: matrix row cannot be zero" );
            better_assert( zen.col() && "save_as_bmp: matrix column cannot be zero" );

            using matrix_details::bmp_details::color_maps;
            std::string const& map_name       = ( color_maps.find( color_map ) == color_maps.end() ) ? std::string{ "default" } : color_map;
            auto&& selected_map               = ( *( color_maps.find( map_name ) ) ).second;

            auto const [the_row, the_col] = zen.shape();
            matrix<std::uint8_t, std::allocator<std::uint8_t>> channel_r{ the_row, the_col };
            matrix<std::uint8_t, std::allocator<std::uint8_t>> channel_g{ the_row, the_col };
            matrix<std::uint8_t, std::allocator<std::uint8_t>> channel_b{ the_row, the_col };

            //auto const& [mx, mn] = std::make_tuple( zen.max(), zen.min() );
            auto const& [mn, mx] = zen.minmax();

            auto&& make_colormap = [&, mx=mx, mn=mn, the_row=the_row, the_col=the_col]( auto row_index )
            {
                for ( auto c : matrix_details::range( the_col ) )
                {
                    auto const[ r_, g_, b_ ] = selected_map( (zen[the_row-row_index-1][c]-mn)/(mx-mn+1.0e-10) );
                    channel_r[row_index][c] = r_;
                    channel_g[row_index][c] = g_;
                    channel_b[row_index][c] = b_;
                }
            };
            matrix_details::parallel( make_colormap, 0UL, the_row, 0UL );

            auto const& encoding = matrix_details::encode_bmp_stream( channel_r, channel_g, channel_b );

            if ( encoding )
            {
                std::string new_file_name{ file_name };
                std::string const extension{ ".bmp" };
                if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                    new_file_name += extension;

                if ( !matrix_details::create_directory_if_not_present( new_file_name ) )
                    better_assert( !"save_as_bmp", " failed to create directory with the target file name is ", new_file_name );

                std::ofstream stream( new_file_name.c_str(), std::ios_base::out | std::ios_base::binary );
                better_assert( stream, " failed to open file with the target file name is ", new_file_name );
                if ( !stream ) return false;

                stream.write( reinterpret_cast<char const*>((*encoding).data()), (*encoding).size() );
                return true;
            }
            return false;
        }

        bool save_as_bmp( char const* const file_name ) const
        {
            return save_as_bmp( std::string{ file_name } );
        }
    }; //crtp_save_as_bmp

    template < typename Matrix, typename Type, typename Allocator >
    using crtp_save_as_bmp_view = crtp_save_as_bmp<Matrix, Type, Allocator>;

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_pgm
    {
        typedef Matrix zen_type;
        bool save_as_pgm( const std::string& file_name ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            std::string new_file_name{ file_name };
            std::string const extension{ ".pgm" };

            if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;

            if ( !matrix_details::create_directory_if_not_present( new_file_name ) )
                better_assert( !"save_as_pgm", " failed to create directory with the target file name is ", new_file_name );

            std::ofstream stream( new_file_name.c_str() );
            better_assert( stream, " save_as_pgm failed to open file with the target file name is ", new_file_name );
            if ( !stream ) return false;

            {
                stream << "P2\n";
                stream << zen.col() << " " << zen.row() << "\n";
                stream << "255\n";
                stream << "# Generated Portable GrayMap image for path [" << file_name << "]\n";
            }

            double const max_val      = static_cast< double >( *std::max_element( zen.begin(), zen.end() ) );
            double const min_val      = static_cast< double >( *std::min_element( zen.begin(), zen.end() ) );
            double const divider      = max_val - min_val + 1.0e-10;

            //for ( std::uint_least64_t r = 0; r < zen.row(); r++ )
            for ( auto const r : matrix_details::range( zen.row() ) )
            {
                //for ( std::uint_least64_t c = 0; c < zen.col(); c++ )
                for ( auto const c : matrix_details::range( zen.col() ) )
                {
                    unsigned long const rgb =  static_cast<unsigned long>( 256.0 * ( zen[r][c] - min_val ) / divider );
                    stream << std::min( rgb, 255UL ) << " ";
                }
                stream << "\n";
            }

            stream.close();
            return true;
        }
        bool save_as_pgm( char const* const file_name ) const
        {
            return save_as_pgm( std::string{ file_name } );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_shrink_to_size
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;

        //reshape matrix to a new row and new col
        //  if new row or col are larger than the original, padding with zero
        //  otherwise, drop these elements
        zen_type& shrink_to_size( const size_type new_row, const size_type new_col ) noexcept
        {
            better_assert( new_row && new_col );
            zen_type& zen = static_cast< zen_type& >( *this );

            if ( new_row == zen.row() && new_col == zen.col() )
                return zen;

            zen_type other{ zen.get_allocator(), new_row, new_col };
            std::fill( other.begin(), other.end(), value_type{} );
            size_type const the_rows_to_copy = std::min( zen.row(), new_row );
            size_type const the_cols_to_copy = std::min( zen.col(), new_col );

            for ( size_type r = 0; r != the_rows_to_copy; ++r )
                std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_rows_to_copy, other.row_begin( r ) );

            zen.swap( other );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_stream_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        friend std::ostream& operator<<( std::ostream& lhs, zen_type const& rhs )
        {
            lhs.precision( 18 );

            for ( size_type i = 0; i < rhs.row(); ++i )
            {
                std::copy( rhs.row_begin( i ), rhs.row_end( i ), std::ostream_iterator< value_type >( lhs, "\t" ) );
                lhs << "\n";
            }

            return lhs;
        }
        friend std::istream& operator>>( std::istream& is, zen_type& rhs )
        {
            std::vector< std::string > row_element;
            std::string string_line;

            while ( std::getline( is, string_line, '\n' ) )
                row_element.push_back( string_line );

            size_type const row = row_element.size();
            size_type const col = std::count_if( row_element[0].begin(), row_element[0].end(), []( char ch ) { return '\t' == ch; } );

            if ( row == 0 || col == 0 )
            {
                is.setstate( std::ios::failbit );
                return is;
            }

            rhs.resize( row, col );

            for ( size_type r = 0; r != row; ++r )
            {
                std::istringstream the_row( row_element[r] );
                std::copy( std::istream_iterator< value_type >( the_row ), std::istream_iterator< value_type >(), rhs.row_begin( r ) );
            }

            return is;
        }
    };
    template < typename Matrix, typename T, typename A >
    struct crtp_swap
    {
        typedef Matrix zen_type;
        void swap( zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::swap( zen.row_, other.row_ );
            std::swap( zen.col_, other.col_ );
            std::swap( zen.dat_, other.dat_ );
            std::swap( zen.allocator_, other.allocator_ );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_tr
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        const value_type tr() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return std::accumulate( zen.diag_begin(), zen.diag_end(), value_type() );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_transpose
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        const zen_type transpose() const noexcept
        {
            const zen_type& zen = static_cast< zen_type const& >( *this );
            zen_type ans( zen.get_allocator(), zen.col(), zen.row() );

            for ( size_type i = 0; i < zen.col(); ++i )
                std::copy( zen.col_begin( i ), zen.col_end( i ), ans.row_begin( i ) );

            return ans;
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_minmax
    {
        typedef Type value_type;
        typedef Matrix zen_type;

        template< typename LessThanCompare >
        auto minmax( LessThanCompare comp ) const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            value_type min_val = std::numeric_limits<value_type>::max();
            value_type max_val = std::numeric_limits<value_type>::min();
            auto const [the_row, the_col] = zen.shape();
            for ( auto r : matrix_details::range(the_row) )
                for ( auto c : matrix_details::range(the_col) )
                {
                    min_val = comp( zen[r][c], min_val ) ? zen[r][c] : min_val;
                    max_val = comp( max_val, zen[r][c] ) ? zen[r][c] : max_val;
                }

            return std::make_tuple( min_val, max_val );
        }

        auto minmax() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return zen.minmax( []( value_type const& x, value_type const& y ) noexcept { return x < y; } );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_min_max
    {
        typedef Type value_type;
        typedef Matrix zen_type;

        template< typename LessThanCompare >
        value_type min( LessThanCompare comp ) const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return matrix_details::reduce( zen.begin(), zen.end(), std::numeric_limits<value_type>::max(), [&comp]( value_type const& a, value_type const& b ){ return comp(a, b) ? a : b; } );
        }

        template< typename LessThanCompare >
        value_type max( LessThanCompare comp ) const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return matrix_details::reduce( zen.begin(), zen.end(), std::numeric_limits<value_type>::min(), [&comp]( value_type const& a, value_type const& b ){ return comp(a, b) ? b : a; } );
        }

        value_type min() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return zen.min( []( value_type const& x, value_type const& y ) noexcept { return x < y; } );
        }

        value_type max() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return zen.max( []( value_type const& x, value_type const& y ) noexcept { return x < y; } );
        }

    };

    template < typename Matrix, typename Type, typename Allocator >
    using crtp_minmax_view = crtp_minmax<Matrix, Type, Allocator>;

    template < typename Type, class Allocator >
    struct matrix_view :
        crtp_save_as_bmp_view<matrix_view<Type, Allocator>, Type, Allocator>,
        crtp_shape_view<matrix_view<Type, Allocator>, Type, Allocator>,
        crtp_bracket_operator_view<matrix_view<Type, Allocator>, Type, Allocator>,
        crtp_minmax_view<matrix_view<Type, Allocator>, Type, Allocator>,
        crtp_row_col_size_view<matrix_view<Type, Allocator>, Type, Allocator>,
        crtp_row_iterator_view<matrix_view<Type, Allocator>, Type, Allocator>,
        crtp_col_iterator_view<matrix_view<Type, Allocator>, Type, Allocator>
    {
        typedef crtp_typedef< Type, Allocator >         type_proxy_type;
        typedef typename type_proxy_type::size_type     size_type;
        matrix_view( matrix<Type, Allocator> const& mat,
                     std::pair<size_type, size_type> const& row_dim,
                     std::pair<size_type, size_type> const& col_dim ) noexcept;

        matrix<Type, Allocator> const&                  matrix_;
        std::pair<size_type, size_type>                 row_dim_;
        std::pair<size_type, size_type>                 col_dim_;
    };//struct matrix_view

    template < typename Type, class Allocator = std::allocator<Type> >
    struct matrix : crtp_anti_diag_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_apply< matrix< Type, Allocator >, Type, Allocator >
        , crtp_bracket_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_clear< matrix< Type, Allocator >, Type, Allocator >
        , crtp_clone< matrix< Type, Allocator >, Type, Allocator >
        , crtp_col_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_copy< matrix< Type, Allocator >, Type, Allocator >
        , crtp_data< matrix< Type, Allocator >, Type, Allocator >
        , crtp_det< matrix< Type, Allocator >, Type, Allocator >
        , crtp_diag_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_direct_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_divide_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_get_allocator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_inverse< matrix< Type, Allocator >, Type, Allocator >
        , crtp_item< matrix< Type, Allocator >, Type, Allocator >
        , crtp_load_binary< matrix< Type, Allocator >, Type, Allocator >
        , crtp_load_npy< matrix< Type, Allocator >, Type, Allocator >
        , crtp_load_txt< matrix< Type, Allocator >, Type, Allocator >
        , crtp_plot< matrix< Type, Allocator >, Type, Allocator >
        , crtp_min_max< matrix< Type, Allocator >, Type, Allocator >
        , crtp_minmax< matrix< Type, Allocator >, Type, Allocator >
        , crtp_minus_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_multiply_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_plus_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_prefix_minus< matrix< Type, Allocator >, Type, Allocator >
        , crtp_prefix_plus< matrix< Type, Allocator >, Type, Allocator >
        , crtp_reshape< matrix< Type, Allocator >, Type, Allocator >
        , crtp_resize< matrix< Type, Allocator >, Type, Allocator >
        , crtp_row_col_size< matrix< Type, Allocator >, Type, Allocator >
        , crtp_row_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_binary< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_bmp< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_pgm< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_txt< matrix< Type, Allocator >, Type, Allocator >
        , crtp_shape< matrix< Type, Allocator >, Type, Allocator >
        , crtp_shrink_to_size< matrix< Type, Allocator >, Type, Allocator >
        , crtp_stream_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_swap< matrix< Type, Allocator >, Type, Allocator >
        , crtp_transpose< matrix< Type, Allocator >, Type, Allocator >
    {
        typedef matrix self_type;
        typedef crtp_typedef< Type, Allocator >                         type_proxy_type;
        typedef typename type_proxy_type::value_type                    value_type;
        typedef typename type_proxy_type::size_type                     size_type;
        typedef typename type_proxy_type::difference_type               difference_type;
        typedef typename type_proxy_type::pointer                       pointer;
        typedef typename type_proxy_type::allocator_type                allocator_type;
        typedef typename type_proxy_type::range_type                    range_type;

        size_type                                                       row_;
        size_type                                                       col_;
        pointer                                                         dat_;
        allocator_type                                                  allocator_;

        ~matrix()
        {
            (*this).clear();
        }

        matrix( size_type const row, size_type const col, std::initializer_list<value_type> const& value_list ) noexcept : matrix{}
        {
            (*this).resize( row, col );
            size_type elements_to_copy = std::min( (*this).size(), value_list.size() );
            std::copy( value_list.begin(), value_list.begin()+elements_to_copy, (*this).begin() );
        }

        matrix( self_type&& other ) noexcept : matrix{}
        {
            (*this).swap( other );
        }

        self_type& operator = ( self_type&& other )
        {
            (*this).clear();
            (*this).swap( other );
            return *this;
        }

        matrix( const self_type& other ) noexcept : row_{ other.row_ }, col_{ other.col_ }, dat_{ nullptr }, allocator_{ other.allocator_ }
        {
            if ( row_*col_ != 0 )
            {
                dat_ = allocator_.allocate( row_*col_ );
                std::copy_n( other.dat_, row_*col_, dat_ );
            }
        }

        template < typename T, typename A >
        matrix( matrix< T, A> const& other ) noexcept : row_{ other.row_ }, col_{ other.col_ }, dat_{ nullptr }, allocator_{ other.allocator_ }
        {
            if ( row_*col_ != 0 )
            {
                dat_ = allocator_.allocate( row_*col_ );
                std::copy_n( other.dat_, row_*col_, dat_ );
            }
        }

        explicit matrix( const size_type r = 0, const size_type c = 0, value_type const& v = value_type{} ) noexcept : row_{ r } , col_{ c }, dat_{ nullptr }
        {
            if ( r*c != 0 )
            {
                dat_ = allocator_.allocate( r*c );
                std::fill_n( dat_, r*c, v );
            }
        }

        explicit matrix( allocator_type a, const size_type r = 0, const size_type c = 0, value_type const& v = value_type{} ) noexcept : row_{ r } , col_{ c }, dat_{ nullptr }, allocator_{ a }
        {
            if ( r*c != 0 )
            {
                dat_ = allocator_.allocate( r*c );
                std::fill_n( dat_, r*c, v );
            }
        }

        template< typename T, typename A >
        matrix( matrix<T,A> const& other, std::initializer_list<size_type> rr, std::initializer_list<size_type> rc ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            better_assert( rr.size() == 2 && "row dims not match!" );
            better_assert( rc.size() == 2 && "col dims not match!" );
            auto [rr0, rr1] = std::make_pair( *(rr.begin()), *(rr.begin()+1) );
            auto [rc0, rc1] = std::make_pair( *(rc.begin()), *(rc.begin()+1) );
            (*this).clone( other, rr0, rr1, rc0, rc1 );
        }

        matrix( matrix const& other, std::initializer_list<size_type> rr, std::initializer_list<size_type> rc ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            better_assert( rr.size() == 2 && "row dims not match!" );
            better_assert( rc.size() == 2 && "col dims not match!" );
            auto [rr0, rr1] = std::make_pair( *(rr.begin()), *(rr.begin()+1) );
            auto [rc0, rc1] = std::make_pair( *(rc.begin()), *(rc.begin()+1) );
            (*this).clone( other, rr0, rr1, rc0, rc1 );
        }

        template < typename T, typename A >
        matrix( const matrix< T, A >& other, const range_type& rr, const range_type& rc ) noexcept : matrix{ other, {rr.first, rr.second}, {rc.first, rc.second} } {}

        matrix( matrix const& other, range_type const& rr, range_type const& rc ) noexcept : matrix{ other, {rr.first, rr.second}, {rc.first, rc.second} } {}

        template < typename T, typename A >
        matrix( const matrix< T, A >& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : matrix{ other, {r0, r1}, {c0, c1} } {}

        matrix( self_type const& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : matrix{ other, {r0, r1}, {c0, c1} } {}

        self_type& operator = ( const self_type& rhs ) noexcept
        {
            ( *this ).copy( rhs );
            return *this;
        }

        template < typename T, typename A >
        self_type& operator = ( const matrix< T, A >& rhs ) noexcept
        {
            ( *this ).copy( rhs );
            return *this;
        }

        self_type& operator = ( const value_type& v ) noexcept
        {
            std::fill( ( *this ).diag_begin(), ( *this ).diag_end(), v ); //TODO:should move to crtp_xxx
            return *this;
        }

        matrix( matrix_view<Type, Allocator> const& view ) noexcept;

        template< typename T >
        auto const astype() const noexcept
        {
            matrix<T, typename std::allocator_traits<Allocator>:: template rebind_alloc<T> > ans{ (*this).get_allocator(), (*this).row(), (*this).col() };
            std::copy( (*this).begin(), (*this).end(), ans.begin() ); //TODO: should move to crtp_xxx
            return ans;
        }

    };//struct matrix

    namespace matrix_details
    {

        namespace map_impl_private
        {
            template< typename T, typename A >
            auto map_impl( matrix<T, A> const& mat ) noexcept
            {
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), ans.begin(), [&]( auto const& v, value_type& a ){ a = func(v); } );
                    return ans;
                };
            }

            template< typename T, typename A, typename T2, typename A2 >
            auto map_impl( matrix<T, A> const& mat, matrix<T2, A2> const& nat ) noexcept
            {
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T, T2> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), nat.begin(), ans.begin(), [&]( auto const& u, auto const& v, value_type& a ){ a = func(u, v); } );
                    return ans;
                };
            }

            template< typename T, typename A, typename T2, typename A2, typename T3, typename A3 >
            auto map_impl( matrix<T, A> const& mat, matrix<T2, A2> const& nat, matrix<T3, A3> const& lat ) noexcept
            {
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T, T2, T3> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), nat.begin(), lat.begin(), ans.begin(), [&]( auto const& u, auto const& v, auto const& w, value_type& a ){ a = func(u, v, w); } );
                    return ans;
                };
            }

            template< typename T, typename A, typename T2 >
            auto map_impl( matrix<T, A> const& mat, T2 const& v ) noexcept
            {
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T, T2> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), ans.begin(), [&]( auto const& u, value_type& a ){ a = func(u, v); } );
                    return ans;
                };
            }

            template< typename T, typename A, typename T2 >
            auto map_impl( T2 const& u, matrix<T, A> const& mat ) noexcept
            {
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T2, T> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), ans.begin(), [&]( auto const& v, value_type& a ){ a = func(u, v); } );
                    return ans;
                };
            }
        }

        template< typename Func >
        auto map( Func const& func ) noexcept
        {
            return [&]( auto const& ... mat ) noexcept
            {
                return map_impl_private::map_impl( mat ... )( func );
            };
        }

        namespace reduce_impl_private
        {
            template< typename T, typename A >
            auto reduce_impl( matrix<T, A> const& mat ) noexcept
            {
                return [&]( auto const& func, auto const& init ) noexcept
                {
                    auto const& stride_func = [&]( auto start_itor, auto stride, auto end_itor ) noexcept
                    {
                        T init_ = init;
                        for ( auto itor = start_itor; itor < end_itor; itor += stride )
                            init_ = func( init_, *itor );
                        return init_;
                    };

                    std::uint_least64_t parallel_size = std::thread::hardware_concurrency();

                    //direct reduce
                    if ( parallel_size<= 1 || mat.size() < 32 )
                        return stride_func( mat.begin(), 1, mat.end() );

                    //parallel reduce to cache
                    std::vector<T> result_cache( parallel_size );
                    auto const& paralle_func = [&]( std::uint_least64_t idx ) noexcept
                    {
                        result_cache[idx] = stride_func( mat.begin()+idx, parallel_size, mat.end() );
                    };
                    matrix_details::parallel( paralle_func, 0UL, parallel_size, 0UL );

                    //final reduce
                    return stride_func( result_cache.begin(), 1, result_cache.end() );
                };
            }
        }//reduce_impl_private

        template< typename Func, typename Type >
        auto reduce( Func const& func, Type init ) noexcept
        {
            return [&]( auto const& mat ) noexcept
            {
                return reduce_impl_private::reduce_impl( mat )( func, init );
            };
        }

    }

    template < typename Type, class Allocator >
    matrix_view<Type, Allocator>::matrix_view( matrix<Type, Allocator> const& mat, std::pair<size_type, size_type> const& row_dim, std::pair<size_type, size_type> const& col_dim ) noexcept: matrix_{ mat }
    {
        row_dim_.first = (row_dim.first >= matrix_.row() || row_dim.first >= row_dim.second) ? 0 : row_dim.first;
        col_dim_.first = (col_dim.first >= matrix_.col() || col_dim.first >= col_dim.second) ? 0 : col_dim.first;
        row_dim_.second = (row_dim.second <= matrix_.row()) ? row_dim.second : matrix_.row();
        col_dim_.second = (col_dim.second <= matrix_.col()) ? col_dim.second : matrix_.col();
    }

        //matrix<Type, Allocator> const&                  matrix_;
        //std::pair<size_type, size_type>                 row_dim_;
        //std::pair<size_type, size_type>                 col_dim_;
    template < typename Type, class Allocator >
    matrix<Type, Allocator>::matrix( matrix_view<Type, Allocator> const& view ) noexcept
    {
        (*this).resize( view.row_dim_.second-view.row_dim_.first, view.col_dim_.second-view.col_dim_.first );
        for ( auto const row : matrix_details::range((*this).row() ) )//copy row by row
                std::copy( view.matrix_.row_begin(row+view.row_dim_.first), view.matrix_.row_end(row+view.row_dim_.second), (*this).row_begin(row) );
    }

    template< typename Type, typename Allocator, typename Integer_Type >
    matrix_view<Type, Allocator> const
    make_view( matrix<Type, Allocator> const& matrix_, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) noexcept
    {
        typedef crtp_typedef< Type, Allocator >         type_proxy_type;
        typedef typename type_proxy_type::size_type     size_type;

        better_assert( row_dim.size() == 2 && "Error: row parameters for a matrix view should be 2!" );
        better_assert( col_dim.size() == 2 && "Error: col parameters for a matrix view should be 2!" );
        return matrix_view<Type, Allocator>
        {
            matrix_,
            std::make_pair( static_cast<size_type>(*(row_dim.begin())), static_cast<size_type>(*(row_dim.begin()+1)) ),
            std::make_pair( static_cast<size_type>(*(col_dim.begin())), static_cast<size_type>(*(col_dim.begin()+1)) )
        };
    }

    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator+( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans{ lhs };
        ans += rhs;
        return ans;
    }

    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator-( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans{ lhs };
        ans -= rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator*( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans( lhs );
        ans *= rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator/( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans( lhs );
        ans /= rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator<( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        better_assert( lhs.row() == rhs.row() );
        better_assert( lhs.col() == rhs.col() );
        return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator==( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        better_assert( lhs.row() == rhs.row() );
        better_assert( lhs.col() == rhs.col() );
        return std::equal( lhs.begin(), lhs.end(), rhs.begin() );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator>( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        return !( ( lhs < rhs ) || ( lhs == rhs ) );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator>=( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        return !( lhs < rhs );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator<=( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        return !( lhs > rhs );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator||( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        if ( lhs.row() == 0 )
            return rhs;

        if ( rhs.row() == 0 )
            return lhs;

        better_assert( lhs.row() == rhs.row() );
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type ::size_type size_type;
        const size_type row = lhs.row();
        const size_type col = lhs.col() + rhs.col();
        matrix_type ans( row, col );

        for ( size_type i = 0; i < row; ++i )
        {
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );
            std::copy( rhs.row_begin( i ), rhs.row_end( i ), ans.row_begin( i ) + lhs.col() );
        }

        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator&&( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        if ( lhs.col() == 0 )
            return rhs;

        if ( rhs.col() == 0 )
            return lhs;

        better_assert( lhs.col() == rhs.col() );
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type ::size_type size_type;
        const size_type row = lhs.row() + rhs.row();
        const size_type col = lhs.col();
        matrix_type ans( row, col );

        for ( size_type i = 0; i < col; ++i )
        {
            std::copy( lhs.col_begin( i ), lhs.col_end( i ), ans.col_begin( i ) );
            std::copy( rhs.col_begin( i ), rhs.col_end( i ), ans.col_begin( i ) + lhs.row() );
        }

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const T_* const rhs )
    {
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs, T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const T_* lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs, lhs + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const std::valarray< T_ >& rhs )
    {
        better_assert( lhs.col() == rhs.size() );
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), std::begin( rhs ), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const std::valarray< T_ >& lhs, const matrix< T, A >& rhs )
    {
        better_assert( rhs.row() == lhs.size() );
        matrix< T, A > ans( 1, lhs.row() );

        for ( std::uint_least64_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( std::begin( lhs ), std::begin( lhs ) + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const std::vector< T_ >& rhs )
    {
        better_assert( lhs.col() == rhs.size() );
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs.begin(), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const std::vector< T_ >& lhs, const matrix< T, A >& rhs )
    {
        better_assert( rhs.col() == lhs.size() );
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs.begin(), lhs.end(), rhs.col_begin( i ), T() );

        return ans;
    }

    //
    // zeros: creating matrix of all zeros.
    //
    template< typename T = double >
    auto const zeros( std::uint_least64_t r, std::uint_least64_t c )
    {
        return matrix<T>{ r, c };
    }

    template< typename T, typename A >
    auto const zeros( A const& alloc, std::uint_least64_t r, std::uint_least64_t c )
    {
        return matrix<T, A>{ alloc, r, c };
    }

    template< typename T = double >
    auto const zeros( std::uint_least64_t n )
    {
        return zeros( n, n );
    }

    template < typename T1, typename A1, typename T2, typename A2 >
    matrix< T1, A1 > const blkdiag( const matrix< T1, A1 >& m1, const matrix< T2, A2 >& m2 )
    {
        return ( m1 || zeros( m1, m1.row(), m2.col() ) ) && ( zeros( m1, m2.row(), m1.col() ) || m2 );
    }
    template < typename T1, typename A1, typename T2, typename A2, typename... Matrices >
    matrix< T1, A1 > const blkdiag( const matrix< T1, A1 >& m1, const matrix< T2, A2 >& m2, const Matrices& ... matrices )
    {
        return blkdiag( blkdiag( m1, m2 ), matrices... );
    }
    template < typename T, typename A, typename... Matrices >
    matrix< T, A > const blk_diag( const matrix< T, A >& m, const Matrices& ... matrices )
    {
        return blkdiag( m, matrices... );
    }
    template < typename T, typename A, typename... Matrices >
    matrix< T, A > const block_diag( const matrix< T, A >& m, const Matrices& ... matrices )
    {
        return blkdiag( m, matrices... );
    }

    template < typename T, typename A >
    matrix< std::complex< T >, A> const ctranspose( const matrix< std::complex< T >, A>& m )
    {
        return conj( m.transpose() );
    }
    template < typename T, typename A >
    T const det( const matrix< T, A >& m )
    {
        return m.det();
    }
    template < typename T, typename A >
    matrix< T, A > const diag( const matrix< T, A >& m, const std::ptrdiff_t offset = 0 )
    {
        const std::uint_least64_t dim = std::min( m.row(), m.col() ) + ( offset > 0 ? offset : -offset );
        matrix< T, A > ans{ dim, dim };
        std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin( offset ) );
        return ans;
    }
    namespace diag_private
    {
        template < typename Itor >
        matrix< typename std::iterator_traits< Itor >::value_type > const
        impl_diag( Itor first, Itor last, const std::ptrdiff_t offset = 0 )
        {
            std::uint_least64_t dim = std::distance( first, last ) + ( offset > 0 ? offset : -offset );
            matrix< typename std::iterator_traits< Itor >::value_type > ans{ dim, dim };
            std::copy( first, last, ans.diag_begin( offset ) );
            return ans;
        }
    }
    template < typename T, typename A >
    matrix< T > const diag( const std::vector< T, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename A >
    matrix< T > const diag( const std::deque< T, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename C, typename A >
    matrix< T > const diag( const std::set< T, C, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename C, typename A >
    matrix< T > const diag( const std::multiset< T, C, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T >
    matrix< T > const diag( const std::valarray< T >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( std::begin( v ), std::end( v ), offset );
    }
    namespace make_diag_private
    {
        struct impl_make_diag
        {
            std::uint_least64_t pos;
            impl_make_diag( const std::uint_least64_t pos_ = 0 )
                : pos( pos_ )
            {
            }
            template < typename T, typename A, typename Arg, typename... Args >
            void operator()( matrix< T, A >& m, const Arg& arg, const Args& ... args ) const
            {
                m[pos][pos] = arg;
                impl_make_diag( pos + 1 )( m, args... );
            }
            template < typename T, typename A, typename Arg >
            void operator()( matrix< T, A >& m, const Arg& arg ) const
            {
                *( m.diag_rbegin() ) = arg;
            }
        };
    }
    template < typename T, typename... Tn >
    matrix< T > const make_diag( const T& v1, const Tn& ... vn )
    {
        const std::uint_least64_t n = 1 + sizeof...( vn );
        matrix< T > ans{ n, n };
        make_diag_private::impl_make_diag()( ans, v1, vn... );
        return ans;
    }
    template < typename T, typename A >
    void display( const matrix< T, A >& m )
    {
        std::cout << m << std::endl;
    }
    template < typename T, typename A >
    void disp( const matrix< T, A >& m )
    {
        display( m );
    }
    template < typename Matrix1, typename Matrix2 >
    typename Matrix1::value_type
    dot( const Matrix1& m1, const Matrix2& m2 )
    {
        better_assert( m1.row() == m2.row() );
        better_assert( m1.col() == m2.col() );
        return std::inner_product( m1.begin(), m1.end(), m2.begin(), typename Matrix1::value_type( 0 ) );
    }
    namespace eye_private
    {
        template < typename T >
        struct one_maker
        {
            T operator()() const
            {
                return T( 1 );
            }
        };
        template < typename T >
        struct one_maker< std::complex< T >>
        {
            std::complex< T > const operator()() const
            {
                return std::complex< T >( T( 1 ), T( 0 ) );
            }
        };
    };
    template < typename T, typename A    = std::allocator< T > >
    matrix< T, A > const eye( const std::uint_least64_t r, const std::uint_least64_t c )
    {
        matrix< T > ans{ r, c };
        std::fill( ans.diag_begin(), ans.diag_end(), eye_private::one_maker< T >()() );
        return ans;
    }
    template < typename T, typename A    = std::allocator< T > >
    matrix< T, A > const eye( const std::uint_least64_t n )
    {
        return eye< T, A >( n, n );
    }
    template < typename T, typename A    = std::allocator< T > >
    matrix< T, A > const eye( const matrix< T, A >& m )
    {
        return eye< T, A >( m.row(), m.col() );
    }
    template < typename T, typename A >
    matrix< T, A > const flipdim( const matrix< T, A >& m, const std::uint_least64_t dim )
    {
        matrix< T, A > ans{ m };

        if ( 1 == dim )
        {
            std::uint_least64_t index_upper = 0;
            std::uint_least64_t index_lower = m.row() - 1;

            while ( index_lower > index_upper )
            {
                std::swap_ranges( ans.row_begin( index_lower ), ans.row_end( index_lower ), ans.row_begin( index_upper ) );
                --index_lower;
                ++index_upper;
            }

            return ans;
        }

        if ( 2 == dim )
        {
            std::uint_least64_t index_left  = 0;
            std::uint_least64_t index_right = m.col() - 1;

            while ( index_right > index_left )
            {
                std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.row_begin( index_right ) );
                --index_right;
                ++index_left;
            }

            return ans;
        }

        better_assert( !"the second argument of flipdim should be '1' or '2'" );
        return ans;
    }
    template < typename T, typename A >
    matrix< T, A > const fliplr( const matrix< T, A >& m )
    {
        return flipdim( m, 1 );
    }
    template < typename T, typename A >
    matrix< T, A > const flipud( const matrix< T, A >& m )
    {
        return flipdim( m, 2 );
    }
    template < typename T,
               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const hilb( const std::uint_least64_t n )
    {
        matrix< T, A > ans( n, n );

        for ( std::uint_least64_t i = 0; i < n; ++i )
            for ( std::uint_least64_t j = i; j < n; ++j )
            {
                ans[i][j] = T( 1 ) / ( i + j + 1 );
                ans[j][i] = ans[i][j];
            }

        return ans;
    }
    template < typename T,
               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const hilbert( const std::uint_least64_t n )
    {
        return hilb< T, A >( n );
    }
    template < typename Matrix >
    Matrix const hilb( const std::uint_least64_t n, const Matrix& )
    {
        typedef typename Matrix::value_type value_type;
        Matrix ans( n, n );

        for ( std::uint_least64_t i = 0; i < n; ++i )
            for ( std::uint_least64_t j = i; j < n; ++j )
            {
                ans[i][j] = value_type( 1 ) / ( i + j + 1 );
                ans[j][i] = ans[i][j];
            }

        return ans;
    }
    template < typename Matrix >
    Matrix const hilbert( const std::uint_least64_t n, const Matrix& m )
    {
        return hilb( n, m );
    }
    template < typename Matrix >
    Matrix const inverse( const Matrix& m )
    {
        return m.inverse();
    }
    template < typename Matrix >
    Matrix const inv( const Matrix& m )
    {
        return m.inverse();
    }
    template < typename T, typename A >
    bool is_column( const matrix< T, A >& m )
    {
        return m.col() == 1;
    }
    template < typename T, typename A >
    bool is_column_matrix( const matrix< T, A >& m )
    {
        return is_column( m );
    }
    template < typename T, typename A >
    bool iscolumn( const matrix< T, A >& m )
    {
        return is_column( m );
    }
    template < typename T, typename A >
    bool is_empty( const matrix< T, A >& m )
    {
        return m.size() == 0;
    }
    template < typename T, typename A >
    bool is_empty_matrix( const matrix< T, A >& m )
    {
        return is_empty( m );
    }
    template < typename T, typename A >
    bool isempty( const matrix< T, A >& m )
    {
        return is_empty( m );
    }
    template < typename T, typename A >
    bool is_equal( const matrix< T, A >& m1, const matrix< T, A > m2 )
    {
        return m1 == m2;
    }
    template < typename M1, typename M2, typename... Mn >
    bool is_equal( const M1& m1, const M2& m2, const Mn& ... mn )
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }
    template < typename T, typename A >
    bool isequal( const matrix< T, A >& m1, const matrix< T, A > m2 )
    {
        return is_equal( m1, m2 );
    }
    template < typename M1, typename M2, typename... Mn >
    bool isequal( const M1& m1, const M2& m2, const Mn& ... mn )
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }
    template < typename T, typename A >
    matrix< bool > is_inf( const matrix< T, A >& m )
    {
        matrix< bool > ans( m.row(), m.col() );
        matrix_details::for_each( m.begin(), m.end(), ans.begin(), []( const T & v, bool & a )
        {
            a = std::isinf( v ) ? true : false;
        } );
        return ans;
    }
    template < typename T, typename A >
    matrix< bool > isinf( const matrix< T, A >& m )
    {
        return is_inf( m );
    }
    template < typename T, typename A >
    matrix< bool > is_nan( const matrix< T, A >& m )
    {
        matrix< bool > ans( m.row(), m.col() );
        matrix_details::for_each( m.begin(), m.end(), ans.begin(), []( const T & v, bool & a )
        {
            a = std::isnan( v ) ? true : false;
        } );
        return ans;
    }
    template < typename T, typename A >
    matrix< bool > isnan( const matrix< T, A >& m )
    {
        return is_nan( m );
    }
    template < typename T, typename A, typename F >
    bool is_orthogonal( const matrix< T, A >& m, F f )
    {
        if ( m.row() != m.col() )
            return false;

        auto mm = m.transpose() * m;
        matrix_details::for_each( mm.diag_begin(), mm.diag_end(), []( T & v )
        {
            v -= T( 1 );
        } );
        return std::all_of( mm.begin(), mm.end(), f );
    }
    template < typename T, typename A >
    bool is_orthogonal( const matrix< T, A >& m )
    {
        return is_orthogonal( m, []( const T v )
        {
            return v == T( 0 );
        } );
    }
    template < typename T, typename A >
    bool is_positive_definite( const matrix< T, A >& m )
    {
        typedef matrix< T, A > matrix_type;
        typedef typename matrix_type::range_type range_type;

        if ( m.row() != m.col() )
            return false;

        for ( std::uint_least64_t i = 1; i != m.row(); ++i )
        {
            const matrix_type a{ m, range_type{ 0, i }, range_type{ 0, i } };

            if ( a.det() <= T( 0 ) )
                return false;
        }

        return true;
    }
    template < typename T, typename A >
    bool is_row( const matrix< T, A >& m )
    {
        return m.row() == 1;
    }
    template < typename T, typename A >
    bool is_row_matrix( const matrix< T, A >& m )
    {
        return is_row( m );
    }
    template < typename T, typename A >
    bool isrow( const matrix< T, A >& m )
    {
        return is_row( m );
    }
    template < typename T, typename A, typename F >
    bool is_symmetric( const matrix< T, A >& m, F f )
    {
        if ( m.row() != m.col() )
            return false;

        for ( std::uint_least64_t i = 1; i != m.row(); ++i )
            if ( !std::equal( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), m.lower_diag_cbegin( i ), f ) )
                return false;

        return true;
    }
    template < typename T, typename A >
    bool is_symmetric( const matrix< T, A >& m )
    {
        return is_symmetric( m, []( const T v1, const T v2 )
        {
            return v1 == v2;
        } );
    }
    inline matrix< std::uint_least64_t > const magic( const std::uint_least64_t n ) noexcept
    {
        matrix< std::uint_least64_t > ans{ n, n };

        if ( 2 == n ) return ans; // no magic for n = 2

        if ( 3 == n )
            return matrix< std::uint_least64_t >{ 3, 3, { 8, 1, 6, 3, 5, 7, 4, 9, 2 } };

        if ( 4 == n )
            return matrix< std::uint_least64_t >{ 4, 4, { 16, 3, 2, 13, 5, 10, 11, 8, 9, 6, 7, 12, 4, 15, 14, 1} };

        // odd case
        if ( n & 1 )
        {
            for ( std::uint_least64_t i = 0; i < n; ++i )
                for ( std::uint_least64_t j = 0; j < n; ++j )
                    //ans[( ( n - 1 ) / 2 + i - j + n ) % n][( 3 * n - 1 + j - 2 * i ) % n] = i * n + j + 1;
                    ans[n - (( 3 * n - 1 + j - 2 * i ) % n) - 1][n - (( ( n - 1 ) / 2 + i - j + n ) % n) - 1] = i * n + j + 1;

            return ans;
        }

        // singly even LUX
        if ( n & 2 )
        {
            auto const half = n >> 1;
            auto const& m_half = magic( half );
            // L
            for ( auto r : matrix_details::range( (half+1) >> 1 ) )
                for ( auto c : matrix_details::range( half ) )
                {
                    auto const val = m_half[r][c];
                    ans[r<<1][c<<1]     = val << 2;         ans[r<<1][(c<<1)+1]     = (val << 2) - 3;
                    ans[(r<<1)+1][c<<1] = (val << 2) - 2;   ans[(r<<1)+1][(c<<1)+1] = (val << 2) - 1;
                }

            // U
            for ( auto c : matrix_details::range(half) )
            {
                auto const r = (half+1) >> 1;
                auto const val = m_half[r][c];
                ans[r<<1][c<<1]     = (val << 2) - 3;    ans[r<<1][(c<<1)+1]     = (val << 2);
                ans[(r<<1)+1][c<<1] = (val << 2) - 2;    ans[(r<<1)+1][(c<<1)+1] = (val << 2) - 1;
            }

            // swap central block
            if (1)
            {
                {
                    auto const [r,c] = std::make_tuple( (half-1)>>1, (half-1)>>1 );
                    auto const val = m_half[r][c];
                    ans[r<<1][c<<1]     = (val << 2) - 3;    ans[r<<1][(c<<1)+1]     = (val << 2);
                    ans[(r<<1)+1][c<<1] = (val << 2) - 2;    ans[(r<<1)+1][(c<<1)+1] = (val << 2) - 1;
                }
                {
                    auto const [r,c] = std::make_tuple( (half+1)>>1, (half+1)>>1 );
                    auto const val = m_half[r][c];
                    ans[r<<1][c<<1]     = (val << 2);    ans[r<<1][(c<<1)+1]     = (val << 2) - 3;
                    ans[(r<<1)+1][c<<1] = (val << 2) - 2;    ans[(r<<1)+1][(c<<1)+1] = (val << 2) - 1;
                }
            }
            // X
            for ( auto r : matrix_details::range( (half+3) >> 1, half ) )
                for ( auto c : matrix_details::range( half ) )
                {
                    auto const& val = m_half[r][c];
                    ans[r<<1][c<<1]     = (val << 2) - 3;   ans[r<<1][(c<<1)+1]     = (val << 2);
                    ans[(r<<1)+1][c<<1] = (val << 2) - 1;   ans[(r<<1)+1][(c<<1)+1] = (val << 2) - 2;
                }

            return ans;
        }

        // doubly even <X>
        std::iota( ans.begin(), ans.end(), 1 );
        std::reverse( ans.diag_begin(), ans.diag_end() );
        std::reverse( ans.anti_diag_begin(), ans.anti_diag_end() );
        std::swap_ranges( ans.lower_diag_begin( n >> 1 ), ans.lower_diag_end( n >> 1 ), ans.upper_diag_rbegin( n >> 1 ) );
        std::swap_ranges( ans.upper_anti_diag_begin( n >> 1 ), ans.upper_anti_diag_end( n >> 1 ), ans.lower_anti_diag_rbegin( n >> 1 ) );
        return ans;
    }
    template < typename T, typename A >
    T const
    max( const matrix< T, A >& m )
    {
        better_assert( m.size() );
        return *std::max_element( m.begin(), m.end() );
    }
    template < typename T, typename A >
    T const
    min( const matrix< T, A >& m )
    {
        better_assert( m.size() );
        return *std::min_element( m.begin(), m.end() );
    }
    template < typename T >
    matrix<T> arange( std::uint_least64_t start, const std::uint_least64_t stop, const std::uint_least64_t step = 1ULL )
    {
        matrix<T> ans{ 1, stop-start/step };
        for ( auto& v : ans )
        {
            v = start;
            start += step;
        }
        return ans;
    }
    template < typename T >
    matrix<T> arange( const std::uint_least64_t length )
    {
        matrix<T> ans{ 1, length };
        std::iota( ans.begin(), ans.end(), T{0} );
        return ans;
    }
    template < typename T >
    matrix<T> linspace( T start, T stop, const std::uint_least64_t num = 50ULL, bool end_point=true )
    {
        if ( 0 == num )
            return matrix<T>{};
        if ( 1 == num )
            return matrix<T>{1, 1, start};

        matrix<T> ans{ 1, num };
        T const step = end_point ? (stop-start)/(num-1) : (stop-start)/num;
        for ( auto& v : ans )
        {
            v = start;
            start += step;
        }
        return ans;
    }
    template < typename T, typename A >
    matrix< T, A > const ones_like( matrix<T, A> const& mat ) noexcept
    {
        return matrix<T, A>{ mat.get_allocator(), mat.row(), mat.col(), T{1} };
    }
    template < typename T >
    auto const ones( const std::uint_least64_t r, const std::uint_least64_t c ) noexcept
    {
        matrix< T > ans{ r, c, T{ 1 } };
        return ans;
    }
    template < typename T >
    auto const ones( const std::uint_least64_t n ) noexcept
    {
        return ones< T >( n, n );
    }
    template < typename T, typename A >
    matrix< T, A > const ones( A const& alloc, std::uint_least64_t r, std::uint_least64_t c ) noexcept
    {
        return { alloc, r , c, T{1} };
    }
    template < typename T, typename A >
    matrix< T, A > const ones( A const& alloc, std::uint_least64_t n ) noexcept
    {
        return { alloc, n , n, T{1} };
    }

    template < typename T >
    auto const empty( const std::uint_least64_t r, const std::uint_least64_t c ) noexcept
    {
        return matrix< T >{ r, c };
    }
    template < typename T, typename A >
    matrix< T, A > const empty( A const& alloc, std::uint_least64_t r, std::uint_least64_t c ) noexcept
    {
        return { alloc, r , c };
    }

    template < typename T, typename A_   = std::allocator<  T  >>
    std::uint_least64_t
    singular_value_decomposition( matrix<T,A_> const& A,
                                  matrix<T,A_>& u,
                                  matrix<T,A_>& w,
                                  matrix<T,A_>& v,
                                  std::uint_least64_t const max_its = 1000 )
    {
        typedef T value_type;
        const value_type zero{ 0 };
        const value_type one{ 1 };
        auto const [m, n] = A.shape();
        u                 = A;
        w.resize( n, n );
        v.resize( n, n );
        std::uint_least64_t i{ 0 }, l{ 0 };
        value_type c{ 0 }, f{ 0 }, h{ 0 };
        std::vector< value_type > arr( n );
        value_type g     = zero;
        value_type s     = zero;
        value_type scale = zero;
        value_type anorm = zero;

        for ( i = 0; i < n; ++i )
        {
            l      = i + 2;
            arr[i] = scale * g;
            g      = zero;
            s      = zero;
            scale  = zero;

            if ( i < m )
            {
                scale = std::accumulate( u.col_begin( i ) + i, u.col_end( i ), value_type( 0 ), []( value_type v1, value_type v2 ) { return v1 + std::abs( v2 ); } );

                if ( scale != zero )
                {
                    matrix_details::for_each( u.col_begin( i ) + i, u.col_end( i ), [scale]( value_type & v ) { v /= scale; } );
                    const value_type tmp_s = std::inner_product( u.col_begin( i ) + i, u.col_end( i ), u.col_begin( i ) + i, value_type( 0 ) );
                    g                      = ( u[i][i] >= zero ) ? -std::sqrt( tmp_s ) : std::sqrt( tmp_s );
                    const value_type tmp_h = u[i][i] * g - tmp_s;
                    u[i][i] -= g;

                    for ( std::uint_least64_t j = l - 1; j < n; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.col_begin( i ) + i, u.col_end( i ), u.col_begin( j ) + i, value_type( 0 ) );
                        std::transform( u.col_begin( j ) + i, u.col_end( j ), u.col_begin( i ) + i, u.col_begin( j ) + i, [tmp_ss, tmp_h]( value_type v1, value_type v2 ) { return v1 + tmp_ss * v2 / tmp_h; } );
                    }

                    matrix_details::for_each( u.col_begin( i ) + i, u.col_end( i ), [scale]( value_type & v ) { v *= scale; } );
                }
            }

            w[i][i] = scale * g;
            g       = zero;
            s       = zero;
            scale   = zero;

            if ( i + 1 <= m && i != n )
            {
                scale = std::accumulate( u.row_begin( i ) + l - 1, u.row_end( i ), value_type( 0 ), []( value_type v1, value_type v2 ) { return v1 + std::abs( v2 ); } );

                if ( scale != zero )
                {
                    matrix_details::for_each( u.row_begin( i ) + l - 1, u.row_end( i ), [scale]( value_type & v ) { v /= scale; } );
                    auto const tmp_s = std::inner_product( u.row_begin( i ) + l - 1, u.row_end( i ), u.row_begin( i ) + l - 1, value_type( 0 ) );
                    g                = ( u[i][l - 1] >= zero ) ? -std::sqrt( tmp_s ) : std::sqrt( tmp_s );
                    auto const tmp_h = u[i][l - 1] * g - tmp_s;
                    u[i][l - 1] -= g;
                    std::transform( u.row_begin( i ) + l - 1, u.row_end( i ), arr.begin() + l - 1, [tmp_h]( value_type v ) { return v / tmp_h; } );

                    for ( std::uint_least64_t j = l - 1; j < m; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.row_begin( j ) + l - 1, u.row_end( j ), u.row_begin( i ) + l - 1, value_type( 0 ) );
                        std::transform( u.row_begin( j ) + l - 1, u.row_end( j ), arr.begin() + l - 1, u.row_begin( j ) + l - 1, [tmp_ss]( value_type v1, value_type v2 ) { return v1 + tmp_ss * v2; } );
                    }

                    matrix_details::for_each( u.row_begin( i ) + l - 1, u.row_end( i ), [scale]( value_type & v )
                    {
                        v *= scale;
                    } );
                }
            }

            anorm = std::max( anorm, ( std::fabs( w[i][i] ) + std::fabs( arr[i] ) ) );
        }

        for ( i = n - 1;; --i )
        {
            if ( i < n - 1 )
            {
                if ( g != zero )
                {
                    auto const tmp_uil = u[i][l];
                    std::transform( u.row_begin( i ) + l, u.row_end( i ), v.col_begin( i ) + l, [g, tmp_uil]( value_type val ) { return val / ( tmp_uil * g ); } );

                    for ( std::uint_least64_t j = l; j < n; j++ )
                    {
                        const auto tmp_s = std::inner_product( u.row_begin( i ) + l, u.row_end( i ), v.col_begin( j ) + l, value_type( 0 ) );
                        std::transform( v.col_begin( j ) + l, v.col_end( j ), v.col_begin( i ) + l, v.col_begin( j ) + l, [tmp_s]( value_type v1, value_type v2 ) { return v1 + v2 * tmp_s; } );
                    }
                }

                std::fill( v.row_begin( i ) + l, v.row_end( i ), zero );
                std::fill( v.col_begin( i ) + l, v.col_end( i ), zero );
            }

            v[i][i] = one;
            g       = arr[i];
            l       = i;

            if ( !i )
                break;
        }

        for ( i = std::min( m, n ) - 1;; --i )
        {
            auto const tmp_l = i + 1;
            auto const tmp_g = w[i][i];
            std::fill( u.row_begin( i ) + tmp_l, u.row_end( i ), zero );

            if ( tmp_g != zero )
            {
                for ( std::uint_least64_t j = tmp_l; j < n; j++ )
                {
                    auto const tmp_s = std::inner_product( u.col_begin( i ) + tmp_l, u.col_end( i ), u.col_begin( j ) + tmp_l, value_type( 0 ) );
                    auto const tmp_f = tmp_s / ( u[i][i] * tmp_g );
                    std::transform( u.col_begin( j ) + i, u.col_end( j ), u.col_begin( i ) + i, u.col_begin( j ) + i, [tmp_f]( value_type v1, value_type v2 ) { return v1 + tmp_f * v2; } );
                }

                matrix_details::for_each( u.col_begin( i ) + i, u.col_end( i ), [tmp_g]( value_type & v ) { v /= tmp_g; } );
            }
            else
                std::fill( u.col_begin( i ) + i, u.col_end( i ), zero );

            ++u[i][i];

            if ( !i )
                break;
        }

        for ( std::uint_least64_t k = n - 1;; --k )
        {
            for ( std::uint_least64_t its = 0; its < max_its; its++ )
            {
                bool flag          = true;
                std::uint_least64_t tmp_nm = 0;

                for ( l = k;; l-- )
                {
                    tmp_nm = l - 1;

                    if ( std::fabs( arr[l] ) + anorm == anorm )
                    {
                        flag = false;
                        break;
                    }

                    if ( std::fabs( w[l - 1][l - 1] ) + anorm == anorm )
                    {
                        break;
                    }

                    if ( l == 0 )
                        break;
                }

                if ( flag )
                {
                    c = zero;
                    s = one;

                    for ( i = l - 1; i < k + 1; ++i )
                    {
                        f      = s * arr[i];
                        arr[i] = c * arr[i];

                        if ( std::fabs( f ) + anorm == anorm )
                        {
                            break;
                        }

                        g       = w[i][i];
                        h       = std::hypot( f, g );
                        w[i][i] = h;
                        h       = one / h;
                        c       = g * h;
                        s       = -f * h;

                        for ( std::uint_least64_t j = 0; j < m; ++j )
                        {
                            value_type y = u[j][tmp_nm];
                            value_type z = u[j][i];
                            u[j][tmp_nm] = y * c + z * s;
                            u[j][i]      = z * c - y * s;
                        }
                    }
                }

                value_type z = w[k][i];

                if ( l == k )
                {
                    if ( z < zero )
                    {
                        w[k][k] = -z;
                        matrix_details::for_each( v.col_begin( k ), v.col_end( k ), []( value_type & v ) { v = -v; } );
                    }

                    break;
                }

                if ( ( its + 1 ) == max_its )
                    return 1;

                value_type x = w[l][l];
                value_type y = w[k - 1][k - 1];
                g            = arr[k - 1];
                h            = arr[k];
                f            = ( ( y - z ) * ( y + z ) + ( g - h ) * ( g + h ) ) / ( 2.0 * h * y );
                g            = std::hypot( f, one );
                g            = ( f >= zero ) ? g : -g;
                f            = ( ( x - z ) * ( x + z ) + h * ( ( y / ( f + g ) ) ) - h ) / x;
                c = s = one;

                for ( std::uint_least64_t j = l; j <= k - 1; j++ )
                {
                    g      = arr[j + 1];
                    y      = w[j + 1][j + 1];
                    h      = s * g;
                    g      = c * g;
                    z      = std::hypot( f, h );
                    arr[j] = z;
                    c      = f / z;
                    s      = h / z;
                    f      = x * c + g * s;
                    g      = g * c - x * s;
                    h      = y * s;
                    y *= c;
                    matrix_details::for_each( v.col_begin( j ), v.col_end( j ), v.col_begin( j + 1 ), [c, s]( value_type & v1, value_type & v2 )
                    {
                        const auto vv1( v1 );
                        const auto vv2( v2 );
                        v1 = vv1 * c + vv2 * s;
                        v2 = vv2 * c - vv1 * s;
                    } );
                    w[j][j] = std::hypot( f, h );

                    if ( value_type( 0 ) != w[j][j] )
                    {
                        c = f / w[j][j];
                        s = h / w[j][j];
                    }

                    matrix_details::for_each( u.col_begin( j ), u.col_end( j ), u.col_begin( j + 1 ), [c, s]( value_type & v1, value_type & v2 )
                    {
                        const auto vv1( v1 );
                        const auto vv2( v2 );
                        v1 = vv1 * c + vv2 * s;
                        v2 = vv2 * c - vv1 * s;
                    } );
                    f = c * g + s * y;
                    x = c * y - s * g;
                }

                arr[l]  = zero;
                arr[k]  = f;
                w[k][k] = x;
                i       = k;
            }

            if ( !k )
                break;
        }

        return 0;
    }

    template< typename T, typename A >
    std::optional< std::tuple<matrix<T, A>, matrix<T, A>, matrix<T, A>> >
    singular_value_decomposition( matrix<T,A> const& a ) noexcept
    {
        auto const [row, col] = a.shape();
        auto const max_iteration = std::max( std::uint_least64_t{100}, std::max( row, col ) );
        if ( matrix<T, A> u, w, v; singular_value_decomposition( a, u, w, v, max_iteration ) ) // fail
            return {};
        else // success
            return std::forward_as_tuple( u, w, v );
    }

    template< typename T, typename A >
    auto svd( matrix<T,A> const& a ) noexcept
    {
        return singular_value_decomposition( a );
    }

    template < typename T, typename A >
    matrix<T,A> const svd_inverse( matrix<T, A> const& a )
    {
        matrix<T, A> u;
        matrix<T, A> w;
        matrix<T, A> v;
        singular_value_decomposition( a, u, v, w );
        matrix_details::for_each( v.begin(), v.end(), []( auto & val ) { if ( std::abs( val ) > 1.0e-10 ) val = 1.0 / val; });
        return w * v.transpose() * u.transpose();
    }
    template < typename Matrix >
    Matrix const pinverse( const Matrix& m )
    {
        Matrix u, w, v;
        singular_value_decomposition( m, u, w, v );
        return v * w * u.transpose();
    }
    template < typename Matrix >
    Matrix const pinv( const Matrix& m )
    {
        return pinverse( m );
    }

    //generating a matrix uniformly in (0, 1)
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const rand( const std::uint_least64_t r, const std::uint_least64_t c, unsigned int seed = 0 ) noexcept
    {
        matrix< T, A > ans{ r, c };
        if ( 0 == seed )
            std::srand( static_cast< unsigned int >( static_cast< std::uint_least64_t >( std::time( nullptr ) ) + reinterpret_cast< std::uint_least64_t >( &ans ) ) );
        else
            std::srand( seed );

        auto const& generator = []() noexcept
        {
            return ( static_cast<T>( std::rand() ) + 1 ) / ( static_cast<T>( RAND_MAX ) + 2 ); // make sure in open bounds range (0, 1)
        };
        std::generate( ans.begin(), ans.end(), generator );
        return ans;
    }
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const rand( const std::uint_least64_t n )
    {
        return rand< T, A >( n, n );
    }

    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const random( const std::uint_least64_t r, const std::uint_least64_t c )
    {
        return rand< T, A >( r, c );
    }
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const random( const std::uint_least64_t n )
    {
        return rand< T, A >( n );
    }
    template < typename T, typename A >
    matrix< T, A > const rand_like( matrix<T, A> const& mat ) noexcept
    {
        auto const[row, col] = mat.shape();
        return random<T, A>( row, col );
    }
    template < typename T, typename A >
    matrix< T, A > const random_like( matrix<T, A> const& mat ) noexcept
    {
        return rand_like<T,A>(mat);
    }
    template < typename T, typename A > //pytorch style
    matrix< T, A > const randn_like( matrix<T, A> const& mat ) noexcept
    {
        return rand_like<T,A>(mat);
    }
    template < typename T, typename A >
    const matrix< T, A >
    repmat( const matrix< T, A >& m, const std::uint_least64_t r, const std::uint_least64_t c )
    {
        better_assert( r );
        better_assert( c );

        if ( 1 == r && 1 == c )
            return m;

        if ( 1 == r )
            return repmat( m, 1, c - 1 ) || m;

        if ( 1 == c )
            return repmat( m, r - 1, 1 ) && m;

        return repmat( repmat( m, 1, c ), r, 1 );
    }
    template < typename Itor1,
               typename Itor2,
               typename A = std::allocator< typename std::remove_const< typename std::remove_reference< typename std::iterator_traits< Itor1 >::value_type >::result_type >::result_type >>
    matrix< typename std::iterator_traits< Itor1 >::value_type, A > const toeplitz( Itor1 i1_, Itor1 _i1, Itor2 i2_, Itor2 _i2 )
    {
        std::uint_least64_t r = std::distance( i1_, _i1 );
        std::uint_least64_t c = std::distance( i2_, _i2 );
        matrix< typename std::iterator_traits< Itor1 >::value_type, A > m( r, c );

        for ( std::uint_least64_t i = 0; i != r; ++i )
            std::fill( m.lower_diag_begin( i ), m.lower_diag_end( i ), *( i1_ + i ) );

        for ( std::uint_least64_t i = 1; i != c; ++i )
            std::fill( m.upper_diag_begin( i ), m.upper_diag_end( i ), *( i2_ + i ) );

        return m;
    }
    template < typename Itor,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< typename std::iterator_traits< Itor >::value_type >::result_type >::result_type >>
    matrix< typename std::iterator_traits< Itor >::value_type, A > const toeplitz( Itor i_, Itor _i )
    {
        return toeplitz( i_, _i, i_, _i );
    }
    template < typename Matrix >
    typename Matrix::value_type tr( const Matrix& m )
    {
        return m.tr();
    }
    template < typename Matrix >
    Matrix const transpose( const Matrix& m )
    {
        return m.transpose();
    }
    template < typename T, typename A >
    matrix< T, A > const tril( const matrix< T, A >& m )
    {
        matrix< T, A > ans{ m.row(), m.col() };

        for ( std::uint_least64_t i = 0; i != m.col(); ++i )
            std::copy( m.lower_diag_cbegin( i ), m.lower_diag_cend( i ), ans.lower_diag_begin( i ) );

        return ans;
    }
    template < typename T, typename A >
    matrix< T, A > const triu( const matrix< T, A >& m )
    {
        matrix< T, A > ans{ m.row(), m.col() };

        for ( std::uint_least64_t i = 0; i != m.col(); ++i )
            std::copy( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), ans.upper_diag_begin( i ) );

        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator+( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return rhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator+( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        return rhs + lhs;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator-( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return x - rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator-( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        matrix< std::complex< T >, A> ans( rhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [lhs]( std::complex< T > const & x )
        {
            return -lhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator*( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return x * rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator*( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        return rhs * lhs;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator/( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return x / rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator/( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        return lhs * rhs.inverse();
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator+( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return rhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator+( const T& lhs, const matrix< T, A >& rhs )
    {
        return rhs + lhs;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator-( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x - rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator-( const T& lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( rhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [lhs]( T x )
        {
            return -lhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x * rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator*( const T& lhs, const matrix< T, A >& rhs )
    {
        return rhs * lhs;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator/( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x / rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator/( const T& lhs, const matrix< T, A >& rhs )
    {
        return lhs * rhs.inverse();
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator||( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs.row(), lhs.col() + 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.col_begin( lhs.col() ), ans.col_end( lhs.col() ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator||( const T& lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( rhs.row(), rhs.col() + 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) + 1 );

        std::fill( ans.col_begin( 0 ), ans.col_end( 0 ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator&&( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs.row() + 1, lhs.col() );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.row_begin( lhs.row() ), ans.row_end( lhs.row() ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator&&( const T& lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( rhs.row() + 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i + 1 ) );

        std::fill( ans.row_begin( 0 ), ans.row_end( 0 ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator^( const matrix< T, A >& lhs, std::uint_least64_t n )
    {
        better_assert( lhs.row() == lhs.col() );
        auto const r = lhs.row();

        if ( 0 == n )
            return eye< T >( r, r );

        if ( 1 == n )
            return lhs;

        if ( n & 1 )
            return lhs ^ ( n - 1 ) * lhs;

        auto const& lhs_2 = lhs ^ ( n >> 1 );
        return lhs_2 * lhs_2;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int backward_substitution( const matrix< T1, A1 >& A,
                               matrix< T2, A2 >& x,
                               const matrix< T3, A3 >& b )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        better_assert( A.row() == A.col() );
        better_assert( A.row() == b.row() );
        better_assert( b.col() == 1 );
        size_type const n = A.row();
        x.resize( n, 1 );
        std::fill( x.begin(), x.end(), value_type( 0 ) );

        for ( size_type i = 0; i != n; ++i )
        {
            size_type const r = n - 1 - i;
            value_type sum    = std::inner_product( x.rbegin(), x.rbegin() + i, A.row_rbegin( r ), value_type( 0 ) );
            x[r][0]           = ( b[r][0] - sum ) / A[r][r];

            if ( std::isinf( x[r][0] ) || std::isnan( x[r][0] ) )
                return 1;
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int biconjugate_gradient_stabilized_method( const matrix< T1, A1 >& A,
            matrix< T2, A2 >& x,
            const matrix< T3, A3 >& b,
            const std::uint_least64_t max_loops = 100,
            const T1 eps                = 1.0e-10 )
    {
        typedef T1 value_type;
        better_assert( A.row() == A.col() );
        better_assert( A.row() == b.row() );
        better_assert( b.col() == 1 );
        auto const n = A.row();

        if ( ( n != x.row() ) || ( 1 != x.col() ) || ( 0 == std::count_if( x.begin(), x.end(), []( T2 const v )
    {
        return v != T2( 0 );
        } ) ) )
        x = b;
        auto r                = b - A * x;
        auto const r_         = r;
        auto p                = r;
        auto s                = p;
        auto ap               = p;
        auto as               = p;
        auto new_r            = r;
        auto rem              = r;
        auto const EPS        = eps * n;
        value_type const zero = value_type( 0 );

        if ( dot( r, r ) < EPS )
            return 0;

        for ( std::uint_least64_t loops = 0; loops != max_loops; ++loops )
        {
            ap               = A * p;
            auto const alpha = dot( r, r_ ) / dot( ap, r_ );

            if ( zero == alpha )
                return 1;

            if ( std::isinf( alpha ) || std::isnan( alpha ) )
                return 1;

            s                = r - alpha * ap;
            as               = A * s;
            auto const omega = dot( as, s ) / dot( as, as );

            if ( std::isinf( omega ) || std::isnan( omega ) )
                return 1;

            x += alpha * p + omega * s;
            new_r           = s - omega * as;
            auto const beta = dot( new_r, r_ ) * alpha / dot( r, r_ ) / omega;

            if ( std::isinf( beta ) || std::isnan( beta ) )
                return 1;

            r   = new_r;
            p   = r + beta * ( p - omega * ap );
            rem = A * x - b;

            if ( dot( rem, rem ) <= EPS )
                return 0;
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int bicgstab( const matrix< T1, A1 >& A,
                  matrix< T2, A2 >& x,
                  const matrix< T3, A3 >& b,
                  const std::uint_least64_t max_loops = 100,
                  const T1 eps                = 1.0e-10 )
    {
        return biconjugate_gradient_stablized_method( A, x, b, max_loops, eps );
    }
    template < typename Matrix1, typename Matrix2 >
    void cholesky_decomposition( const Matrix1& m, Matrix2& a )
    {
        typedef typename Matrix1::value_type value_type;
        better_assert( m.row() == m.col() );
        a                   = m;
        const std::uint_least64_t n = m.row();

        for ( std::uint_least64_t i = 0; i < n; ++i )
            for ( std::uint_least64_t j = i; j < n; ++j )
            {
                const value_type sum = a[i][j] - std::inner_product( a.row_begin( i ), a.row_begin( i ) + i, a.row_begin( j ), value_type( 0 ) );
                a[j][i]              = ( i == j ) ? std::sqrt( sum ) : ( sum / a[i][i] );
            }

        for ( std::uint_least64_t i = 1; i < n; ++i )
            std::fill( a.upper_diag_begin( i ), a.upper_diag_end( i ), value_type() );
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int conjugate_gradient_squared( const matrix< T1, A1 >& A,
                                    matrix< T2, A2 >& x,
                                    const matrix< T3, A3 >& b,
                                    const std::uint_least64_t max_loops = 100,
                                    const T1 eps                = 1.0e-10 )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        better_assert( A.row() == A.col() );
        better_assert( A.row() == b.row() );
        better_assert( b.col() == 1 );
        size_type const n = A.row();
        x.resize( n, 1 );

        if ( dot( x, x ) == value_type() )
            x = b;

        matrix_type const r_ = b - A * x;
        matrix_type r        = r_;
        matrix_type p        = r_;
        matrix_type u        = r_;
        matrix_type ap       = r_;
        matrix_type q        = r_;
        matrix_type new_r    = r_;
        matrix_type uq       = r_;
        matrix_type rem      = r_;
        value_type const EPS = n * eps * eps;

        if ( dot( r_, r_ ) < EPS )
            return 0;

        for ( std::uint_least64_t i = 0; i != max_loops; ++i )
        {
            ap                     = A * p;
            value_type const alpha = dot( r, r_ ) / dot( ap, r_ );

            if ( std::isinf( alpha ) || std::isnan( alpha ) )
            {
                return 1;
            }

            q  = u - alpha * ap;
            uq = u + q;
            x += alpha * uq;
            rem = A * x - b;

            if ( dot( rem, rem ) < EPS )
                return 0;

            new_r                 = r - alpha * A * uq;
            value_type const beta = dot( new_r, r_ ) / dot( r, r_ );

            if ( std::isinf( beta ) || std::isnan( beta ) )
            {
                return 1;
            }

            r = new_r;
            u = r + beta * q;
            p = u + beta * ( q + beta * p );
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int cgs( const matrix< T1, A1 >& A,
             matrix< T2, A2 >& x,
             const matrix< T3, A3 >& b,
             const std::uint_least64_t max_loops = 100,
             const T1 eps                = 1.0e-10 )
    {
        return conjugate_gradient_squared( A, x, b, max_loops, eps );
    }
    template < typename Matrix1, typename Matrix2, typename Matrix3 >
    void householder( const Matrix1& A, Matrix2& Q, Matrix3& D )
    {
        typedef Matrix1 matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        better_assert( A.row() == A.col() );
        size_type const n     = A.row();
        value_type const zero = value_type( 0 );
        value_type const one  = value_type( 1 );
        value_type const two  = value_type( 2 );
        Matrix1 const I       = eye( n, A );
        Q                     = eye< value_type >( n );
        D                     = A;
        Matrix1 x( n, 1 );
        Matrix1 P( n, n );

        if ( n < 3 )
        {
            return;
        }

        for ( size_type i = 0; i != n - 1; ++i )
        {
            std::fill( x.begin(), x.begin() + i + 1, zero );
            std::copy( D.col_begin( i ) + i + 1, D.col_end( i ), x.begin() + i + 1 );
            value_type const delta = std::sqrt( std::inner_product( x.begin(), x.end(), x.begin(), zero ) );

            if ( zero == delta )
                continue;

            if ( x[i + 1][0] > zero )
                x[i + 1][0] += delta;
            else
                x[i + 1][0] -= delta;

            value_type const H = std::inner_product( x.begin(), x.end(), x.begin(), zero ) / two;
            P                  = I - ( x * x.transpose() ) / H;

            if ( zero == H )
                continue;

            matrix_type const p = D * x / H;
            value_type const k  = std::inner_product( x.begin(), x.end(), p.begin(), zero ) / ( H + H );
            matrix_type const q = p - k * x;
            D -= q * x.transpose() + x * q.transpose();
            Q -= Q * x * x.transpose() / H;
        }
    }
    namespace eigen_jacobi_private
    {
        template < typename Matrix >
        typename Matrix::value_type norm( const Matrix& A )
        {
            typedef typename Matrix::value_type value_type;
            auto A_ = abs( A );
            std::fill( A_.diag_begin(), A_.diag_end(), value_type( 0 ) );
            auto const max_elem = *( std::max_element( A_.cbegin(), A_.cend() ) );

            if ( value_type( 0 ) == max_elem )
                return value_type( 0 );

            A_ /= max_elem;
            auto const sum = std::inner_product( A_.cbegin(), A_.cend(), A_.cbegin(), value_type( 0 ) );
            return std::sqrt( sum ) * max_elem;
        }
        template < typename Matrix1, typename Matrix2 >
        void rotate( Matrix1& A, Matrix2& V, const std::uint_least64_t p, const std::uint_least64_t q )
        {
            typedef typename Matrix1::value_type value_type;
            auto const one   = value_type( 1 );
            auto const n     = A.row();
            auto const theta = ( A[q][q] - A[p][p] ) / ( A[p][q] + A[p][q] );
            auto const t     = std::copysign( one / ( std::abs( theta ) + std::hypot( theta, one ) ), theta );
            auto const c     = one / std::hypot( t, one );
            auto const s     = t * c;

            for ( std::uint_least64_t i = 0; i != n; ++i )
            {
                auto const vip = V[i][p] * c - V[i][q] * s;
                auto const viq = V[i][q] * c + V[i][p] * s;
                V[i][p]        = vip;
                V[i][q]        = viq;
                auto const api = c * A[p][i] - s * A[q][i];
                auto const aqi = c * A[q][i] + s * A[p][i];
                A[p][i]        = api;
                A[q][i]        = aqi;
            }

            for ( std::uint_least64_t i = 0; i != n; ++i )
            {
                auto const aip = A[i][p] * c - A[i][q] * s;
                auto const aiq = A[i][q] * c + A[i][p] * s;
                A[i][p]        = aip;
                A[i][q]        = aiq;
            }
        }
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_jacobi( A, V, Lambda.diag_begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        better_assert( A.row() == A.col() );
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type( 1 );
        auto const zero = value_type( 0 );
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );

        for ( size_type i = 0; i != size_type( -1 ); ++i )
        {
            size_type p            = 0;
            size_type q            = 1;
            value_type current_max = std::abs( a[p][q] );

            for ( size_type ip = 0; ip != n; ++ip )
                for ( size_type iq = ip + 1; iq != n; ++iq )
                {
                    auto const tmp = std::abs( a[ip][iq] );

                    if ( current_max > tmp )
                        continue;

                    current_max = tmp;
                    p           = ip;
                    q           = iq;
                }

            if ( current_max < eps )
            {
                std::copy( a.diag_begin(), a.diag_end(), o );
                return i;
            }

            eigen_jacobi_private::rotate( a, V, p, q );
        }

        return size_type( -1 );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, std::uint_least64_t max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        auto const compare_func = [eps]( const value_type lhs, const value_type rhs )
        {
            return std::abs( lhs - rhs ) < eps;
        };
        better_assert( A.row() == A.col() );
        size_type i     = 0;
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type( 1 );
        auto const zero = value_type( 0 );
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );

        for ( ; i != max_rot; ++i )
        {
            if ( !( i & 7 ) && eigen_jacobi_private::norm( a ) == zero )
            {
                break;
            }

            for ( size_type p = 0; p != n; ++p )
                for ( size_type q = p + 1; q != n; ++q )
                    eigen_jacobi_private::rotate( a, V, p, q );
        }

        std::copy( a.diag_begin(), a.diag_end(), o );
        return i * n * n;
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, std::uint_least64_t const max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, std::uint_least64_t const max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, std::uint_least64_t const max_rot = 80, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return cyclic_eigen_jacobi( A, V, Lambda.diag_begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_real_symmetric( A, V, Lambda.diag_begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        better_assert( A.row() == A.col() );
        Matrix1 D( A );
        Matrix1 Q( A );
        householder( A, Q, D );
        eigen_jacobi( D, V, o, eps );
        V = Q * V;
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-20 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-20 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T, typename A_, typename T_ = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-20 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_hermitian_impl( A, V, Lambda.diag_begin(), eps );
    }
    template < typename T1, typename A1, typename T2, typename A2, typename Otor, typename T = double >
    void eigen_hermitian_impl( const matrix< std::complex< T1 >, A1 >& A, matrix< std::complex< T2 >, A2 >& V, Otor o, const T eps = T( 1.0e-20 ) )
    {
        better_assert( A.row() == A.col() );
        std::uint_least64_t const n = A.row();
        auto const A_       = real( A );
        auto const B_       = imag( A );
        auto const AA       = ( A_ || ( -B_ ) ) && ( B_ || A_ );
        matrix< T1, A1 > VV( n + n, n + n );
        matrix< T1, A1 > LL( n + n, n + n );
        eigen_real_symmetric( AA, VV, LL, eps );
        std::vector< T1 > vec( n + n );
        std::copy( LL.diag_begin(), LL.diag_end(), vec.begin() );
        std::sort( vec.begin(), vec.end() );
        V.resize( n, n );

        for ( std::uint_least64_t i = 0; i != n; ++i )
        {
            std::uint_least64_t const offset = std::distance( LL.diag_begin(), std::find( LL.diag_begin(), LL.diag_end(), vec[i + i] ) );
            better_assert( offset < n + n );
            matrix_details::for_each( V.col_begin( i ), V.col_end( i ), VV.col_begin( offset ), []( std::complex< T2 >& c, T1 const r )
            {
                c.real( r );
            } );
            matrix_details::for_each( V.col_begin( i ), V.col_end( i ), VV.col_begin( offset ) + n, []( std::complex< T2 >& c, T1 const i )
            {
                c.imag( i );
            } );
            *o++ = vec[i + i];
        }
    }
    template < typename T, typename A_, typename O >
    T eigen_power_iteration( const matrix< T, A_ >& A, O output, const T eps = T( 1.0e-5 ) )
    {
        better_assert( A.row() == A.col() );
        matrix< T, A_ > b( A.col(), 1 );
        std::copy( A.diag_cbegin(), A.diag_cend(), b.begin() );

        for ( ;; )
        {
            auto const old_b = b;
            b                = A * b;
            auto const u     = std::inner_product( b.begin(), b.end(), b.begin(), T( 0 ) );
            auto const norm  = std::sqrt( u );
            b /= norm;
            auto const U  = std::inner_product( b.begin(), b.end(), b.begin(), T( 0 ) );
            auto const V  = std::inner_product( old_b.begin(), old_b.end(), old_b.begin(), T( 0 ) );
            auto const UV = std::inner_product( b.begin(), b.end(), old_b.begin(), T( 0 ) );

            if ( UV * UV > U * V * ( T( 1 ) - eps ) )
            {
                std::copy( b.begin(), b.end(), output );
                return norm;
            }
        }

        better_assert( !"eigen_power_iteration:: should never reach here!" );
        return T( 0 );
    }
    template < typename T, typename A_ >
    T eigen_power_iteration( const matrix< T, A_ >& A, const T eps = T( 1.0e-5 ) )
    {
        matrix< T, A_ > b( A.col(), 1 );
        return eigen_power_iteration( A, b.begin(), eps );
    }
    template < typename T, typename A_, typename O >
    T eigen_power_iteration( const matrix< std::complex< T >, A_ >& A, O output, const T eps = T( 1.0e-5 ) )
    {
        better_assert( A.row() == A.col() );
        matrix< std::complex< T >, A_ > b( A.col(), 1 );
        matrix< std::complex< T >, A_ > b_( A.col(), 1 );
        std::copy( A.diag_cbegin(), A.diag_cend(), b.begin() );
        matrix< std::complex< T >, A_ > Am( A );
        matrix_details::for_each( Am.begin(), Am.end(), []( std::complex< T >& c )
        {
            c = std::conj( c );
        } );

        for ( ;; )
        {
            auto const old_b = b;
            b                = Am * b;
            std::transform( b.begin(), b.end(), b_.begin(), []( std::complex< T > v )
            {
                return std::conj( v );
            } );
            auto const u_   = std::inner_product( b.begin(), b.end(), b_.begin(), std::complex< T >( 0, 0 ) );
            auto const u    = real( u_ );
            auto const norm = std::sqrt( u );
            b /= norm;
            std::transform( b.begin(), b.end(), b_.begin(), []( std::complex< T > v )
            {
                return std::conj( v );
            } );
            auto const U_ = std::inner_product( b.begin(), b.end(), b_.begin(), std::complex< T >( 0, 0 ) );
            auto const U  = real( U_ );
            std::transform( old_b.begin(), old_b.end(), b_.begin(), []( std::complex< T > v )
            {
                return std::conj( v );
            } );
            auto const V_  = std::inner_product( old_b.begin(), old_b.end(), b.begin(), std::complex< T >( 0, 0 ) );
            auto const V   = real( V_ );
            auto const UV_ = std::inner_product( b.begin(), b.end(), b_.begin(), std::complex< T >( 0, 0 ) );
            auto const UV  = real( UV_ );

            if ( UV * UV > U * V * ( T( 1 ) - eps ) )
            {
                std::copy( b.begin(), b.end(), output );
                return norm;
            }
        }

        better_assert( !"eigen_power_iteration:: should never reach here!" );
        return T( 0 );
    }
    template < typename T, typename A_ >
    T eigen_power_iteration( const matrix< std::complex< T >, A_ >& A, const T eps = T( 1.0e-5 ) )
    {
        matrix< std::complex< T >, A_ > b( A.col(), 1 );
        return eigen_power_iteration( A, b.begin(), eps );
    }
    /*
    template < typename Matrix >
    typename Matrix::value_type
    norm( const Matrix& A )
    {
        typedef typename Matrix::value_type value_type;
        std::vector< value_type > m( A.row() );

        for ( std::uint_least64_t i = 0; i != A.row(); ++i )
            m[i] = std::accumulate( A.row_cbegin( i ), A.row_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }

    template < typename Matrix >
    typename Matrix::value_type
    norm( const Matrix& A, const std::uint_least64_t n )
    {
        typedef typename Matrix::value_type value_type;

        if ( 1 == n )
        {
            std::vector< value_type > m( A.col() );

            for ( std::uint_least64_t i = 0; i != A.col(); ++i )
                m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), value_type( 0 ), []( value_type u, value_type v )
            {
                return u + std::abs( v );
            } );
            return *( std::max_element( m.begin(), m.end() ) );
        }

        if ( 2 == n )
        {
            return std::sqrt( eigen_power_iteration( A ) );
        }

        better_assert( !"norm:: other norm algorithm has not been implemented!" );
        return value_type( 0 );
    }
    */
    template < typename Matrix >
    typename Matrix::value_type
    norm_1( const Matrix& A )
    {
        typedef typename Matrix::value_type value_type;
        std::vector< value_type > m( A.col() );

        for ( std::uint_least64_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }
    template < typename T, typename A_ >
    T norm_1( const matrix< std::complex< T >, A_ >& A )
    {
        std::vector< T > m( A.col() );

        for ( std::uint_least64_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), T( 0 ), []( const T u, const std::complex< T >& v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }
    template < typename Matrix >
    typename Matrix::value_type
    norm_2( const Matrix& A )
    {
        return std::sqrt( eigen_power_iteration( A ) );
    }
    namespace expm_private
    {
        template < typename T >
        struct fix_complex_value_type
        {
            typedef T value_type;
        };
        template < typename T >
        struct fix_complex_value_type< std::complex< T >>
        {
            typedef typename fix_complex_value_type< T >::value_type value_type;
        };
    }
    template < typename T, typename A_ >
    const matrix< T, A_ >
    expm( const matrix< T, A_ >& A )
    {
        typedef matrix< T, A_ > matrix_type;
        typedef typename matrix_type::value_type value_type_;
        typedef typename expm_private::fix_complex_value_type< value_type_ >::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        better_assert( A.row() == A.col() );
        static const value_type theta[] = { 0.000000000000000e+000,
                                            3.650024139523051e-008,
                                            5.317232856892575e-004,
                                            1.495585217958292e-002,
                                            8.536352760102745e-002,
                                            2.539398330063230e-001,
                                            5.414660951208968e-001,
                                            9.504178996162932e-001,
                                            1.473163964234804e+000,
                                            2.097847961257068e+000,
                                            2.811644121620263e+000,
                                            3.602330066265032e+000,
                                            4.458935413036850e+000,
                                            5.371920351148152e+000
                                          };
        value_type const norm_A         = norm_1( A );
        value_type const ratio          = norm_A / theta[13];
        size_type const s               = ratio < value_type( 1 ) ? 0 : static_cast< size_type >( std::ceil( std::log2( ratio ) ) );
        value_type const s__2           = s ? value_type( 1 << s ) : value_type( 1 );
        matrix_type const& _A           = A / s__2;
        size_type const n               = _A.row();
        static value_type const c[]     = { 0.000000000000000,
                                            6.4764752532480000e+16,
                                            3.2382376266240000e+16,
                                            7.771770303897600e+15,
                                            1.187353796428800e+15,
                                            1.29060195264000e+14,
                                            1.0559470521600e+13,
                                            6.70442572800e+11,
                                            3.3522128640e+10,
                                            1.323241920e+9,
                                            4.0840800e+7,
                                            9.60960e+5,
                                            1.6380e+4,
                                            1.82e+2,
                                            1
                                          };
        matrix_type const& _A2          = _A * _A;
        matrix_type const& _A4          = _A2 * _A2;
        matrix_type const& _A6          = _A2 * _A4;
        matrix_type const& U            = _A * ( _A6 * ( c[14] * _A6 + c[12] * _A4 + c[10] * _A2 ) + c[8] * _A6 + c[6] * _A4 + c[4] * _A2 + c[2] * eye< value_type >( n, n ) );
        matrix_type const& V            = _A6 * ( c[13] * _A6 + c[11] * _A4 + c[9] * _A2 ) + c[7] * _A6 + c[5] * _A4 + c[3] * _A2 + c[1] * eye< value_type >( n, n );
        matrix_type const& VU           = V + U;
        matrix_type const& UV           = V - U;
        matrix_type F                   = VU / UV;

        for ( size_type i = 0; i != s; ++i )
            F *= F;

        return F;
    }
    namespace fft_private
    {
        template < typename T >
        struct add_complex
        {
            typedef std::complex< T > result_type;
        };
        template < typename T >
        struct add_complex< std::complex< T >>
        {
            typedef std::complex< T > result_type;
        };
    }
    template < typename T >
    auto fft( matrix< T > const& x )
    {
        typedef typename fft_private::add_complex< T >::result_type complex_type;
        matrix< complex_type > X( x.row(), x.col() );
        auto make_omege = []( auto k, auto n, auto N )
        {
            double const pi    = 3.1415926535897932384626433;
            double const theta = -pi * 2.0 * k * n / static_cast< double >( N );
            return complex_type{ std::cos( theta ), std::sin( theta ) };
        };
        std::uint_least64_t const R = X.row();
        std::uint_least64_t const C = X.col();

        for ( std::uint_least64_t r = 0; r != R; ++r )
            for ( std::uint_least64_t c = 0; c != C; ++c )
            {
                complex_type X_rc{ 0.0, 0.0 };

                for ( std::uint_least64_t r_ = 0; r_ != R; ++r_ )
                {
                    complex_type tmp{ 0.0, 0.0 };

                    for ( std::uint_least64_t c_ = 0; c_ != C; ++c_ )
                        tmp += x[r][c] * make_omege( c, c_, C );

                    X_rc += tmp * make_omege( r, r_, R );
                }

                X[r][c] = X_rc;
            }

        return X;
    }
    template < typename T >
    auto fftshift( matrix< T > const& x )
    {
        auto X                          = fft( x );
        std::uint_least64_t const R           = X.row();
        std::uint_least64_t const C           = X.col();
        std::uint_least64_t const row_starter = ( R >> 1 ) + ( R & 1 );

        for ( std::uint_least64_t index = 0; row_starter + index < R; ++index )
            std::swap_ranges( X.row_begin( index ), X.row_end( index ), X.row_begin( row_starter + index ) );

        std::uint_least64_t const col_starter = ( C >> 1 ) + ( C & 1 );

        for ( std::uint_least64_t index = 0; col_starter + index < C; ++index )
            std::swap_ranges( X.col_begin( index ), X.col_end( index ), X.col_begin( col_starter + index ) );

        return X;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int forward_substitution( const matrix< T1, A1 >& A, matrix< T2, A2 >& x, const matrix< T3, A3 >& b )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        better_assert( A.row() == A.col() );
        better_assert( A.row() == b.row() );
        better_assert( b.col() == 1 );
        size_type const n = A.row();
        x.resize( n, 1 );
        std::fill( x.begin(), x.end(), value_type( 0 ) );

        for ( size_type i = 0; i != n; ++i )
        {
            value_type sum = std::inner_product( x.begin(), x.begin() + i, A.row_begin( i ), value_type( 0 ) );
            x[i][0]        = ( b[i][0] - sum ) / A[i][i];

            if ( std::isinf( x[i][0] ) || std::isnan( x[i][0] ) )
                return 1;
        }

        return 0;
    }

    template < typename T, typename A >
    std::optional<matrix<T,A>> gauss_jordan_elimination( matrix< T, A > const& m ) noexcept
    {
        auto const& [row, col] = m.shape();
        better_assert( row < col && "matrix row must be less than colum to execut a Gauss-Jordan Elimination" );

        auto a = m;

        for ( auto i : matrix_details::range(row) )
        {
            auto const p = std::distance( a.col_begin( i ), std::max_element( a.col_begin( i ) + i, a.col_end( i ), [](auto x, auto y){ return std::abs(x) < std::abs(y); } ) );

            if ( p != static_cast<std::ptrdiff_t>(i) )
                std::swap_ranges( a.row_begin( i ) + i, a.row_end( i ), a.row_begin( p ) + i );

            auto const factor = a[i][i];

            if ( std::abs(factor) < 1.0e-10) return {};

            matrix_details::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, [factor](auto& v){ v /= factor; } );

            for ( auto j : matrix_details::range(row) )
            {
                if ( i == j ) continue;

                auto const ratio = a[j][i];
                std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), [ratio](auto x, auto y){ return x - y*ratio; } );
            }
        }

        return a;
    }

    // matlab alias
    template < typename T, typename A >
    std::optional<matrix<T,A>> rref( matrix< T, A > const& m ) noexcept
    {
        return gauss_jordan_elimination( m );
    }

    namespace ifft_private
    {
        template < typename T >
        struct add_complex
        {
            typedef std::complex< T > result_type;
        };
        template < typename T >
        struct add_complex< std::complex< T >>
        {
            typedef std::complex< T > result_type;
        };
    }
    template < typename T >
    auto ifft( matrix< T > const& x )
    {
        typedef typename ifft_private::add_complex< T >::result_type complex_type;
        matrix< complex_type > X( x.row(), x.col() );
        auto make_omege = []( auto k, auto n, auto N )
        {
            double const pi    = 3.1415926535897932384626433;
            double const theta = pi * 2.0 * k * n / static_cast< double >( N );
            return complex_type{ std::cos( theta ), std::sin( theta ) };
        };
        std::uint_least64_t const R = X.row();
        std::uint_least64_t const C = X.col();

        for ( std::uint_least64_t r = 0; r != R; ++r )
            for ( std::uint_least64_t c = 0; c != C; ++c )
            {
                complex_type X_rc{ 0.0, 0.0 };

                for ( std::uint_least64_t r_ = 0; r_ != R; ++r_ )
                {
                    complex_type tmp{ 0.0, 0.0 };

                    for ( std::uint_least64_t c_ = 0; c_ != C; ++c_ )
                        tmp += x[r][c] * make_omege( c, c_, C );

                    X_rc += tmp * make_omege( r, r_, R );
                }

                X[r][c] = X_rc;
            }

        return X;
    }
    template < typename T >
    auto ifftshift( matrix< T > const& x )
    {
        auto X                          = ifft( x );
        std::uint_least64_t const R           = X.row();
        std::uint_least64_t const C           = X.col();
        std::uint_least64_t const row_starter = ( R >> 1 ) + ( R & 1 );

        for ( std::uint_least64_t index = 0; row_starter + index < R; ++index )
            std::swap_ranges( X.row_begin( index ), X.row_end( index ), X.row_begin( row_starter + index ) );

        std::uint_least64_t const col_starter = ( C >> 1 ) + ( C & 1 );

        for ( std::uint_least64_t index = 0; col_starter + index < C; ++index )
            std::swap_ranges( X.col_begin( index ), X.col_end( index ), X.col_begin( col_starter + index ) );

        return X;
    }

    template < typename Type, typename Allocator>
    int lu_decomposition( const matrix< Type, Allocator >& A, matrix< Type, Allocator >& L, matrix< Type, Allocator >& U )
    {
        typedef Type value_type;
        better_assert( A.row() == A.col() && "Square Matrix Requred!" );

        const std::uint_least64_t n = A.row();
        L.resize( n, n );
        std::fill( L.begin(), L.end(), value_type{0} );
        std::fill( L.diag_begin(), L.diag_end(), value_type( 1 ) );

        U.resize( n, n );
        std::fill( U.begin(), U.end(), value_type{0} );

        for ( std::uint_least64_t j = 0; j < n; ++j )
        {
            for ( std::uint_least64_t i = 0; i < j + 1; ++i )
            {
                U[i][j] = A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + i, U.col_begin( j ), value_type() );
            }

            for ( std::uint_least64_t i = j + 1; i < n; ++i )
            {
                L[i][j] = ( A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + j, U.col_begin( j ), value_type() ) ) / U[j][j];

                if ( std::isinf( L[i][j] ) || std::isnan( L[i][j] ) )
                    return 1;
            }
        }

        return 0;
    }

    template < typename Type, typename Allocator>
    std::optional<std::tuple<matrix<Type, Allocator>, matrix<Type, Allocator>>> lu_decomposition( const matrix< Type, Allocator >& A )
    {
        if ( matrix<Type, Allocator> L, U; lu_decomposition( A, L, U ) == 1 )
            return {};
        else
            return std::make_tuple( L, U );
    }

    template < typename Type, typename Allocator >
    int lu_solver( const matrix< Type, Allocator >& A, matrix< Type, Allocator >& x, const matrix< Type, Allocator >& b )
    {
        typedef matrix< Type, Allocator > matrix_type;
        better_assert( A.row() == A.col() );
        better_assert( A.row() == b.row() );
        better_assert( b.col() == 1 );
        matrix_type L, U;

        if ( lu_decomposition( A, L, U ) )
            return 1;

        matrix_type Y;

        if ( forward_substitution( L, Y, b ) )
            return 1;

        if ( backward_substitution( U, x, Y ) )
            return 1;

        return 0;
    }

    template < typename Type, typename Allocator >
    std::optional<matrix<Type, Allocator>> lu_solver( const matrix< Type, Allocator >& A, matrix< Type, Allocator > const& b )
    {
        if ( matrix<Type, Allocator> x; lu_solver(A, x, b) == 1 )
            return {};
        else
            return x;
    }

    template< typename Type, typename Allocator >
    matrix<Type, Allocator> const conv( matrix<Type, Allocator> const& A, matrix<Type, Allocator> const& B ) noexcept
    {
        if ( ( 0 == A.size() ) || ( 0 == B.size() ) )
            return matrix<Type, Allocator>{0, 0};

        if ( A.size() > B.size() )
            return conv( B, A );

        matrix<Type, Allocator> padded_B{ B.row()+2*A.row()-2, B.col()+2*A.col()-2 };
        for ( auto row : matrix_details::range(B.row()) )
            std::copy( B.row_begin(row), B.row_end(row), padded_B.row_begin(row+A.row()-1) );

        matrix<Type, Allocator> ans{ A.row()+B.row()-1, A.col()+B.col()-1 };

        auto const& product = []( matrix<Type, Allocator> const& a, matrix_view<Type, Allocator> const& b, auto row, auto const col ) noexcept
        {
            Type ans{0};
            for ( auto r : matrix_details::range(row) )
                for ( auto c : matrix_details::range(col) )
                    ans += a[r][c] * b[r][c];
            return ans;
        };

        auto const& func = [&]( std::uint_least64_t row )
        {
            for ( auto col : matrix_details::range(ans.row() ))
            {
                auto const& view = make_view( padded_B, {row, row+A.row()}, {col, col+A.col()} );
                ans[row][col] = product( A, view, A.row(), A.col() );
            }
        };

        matrix_details::parallel( func, 0UL, ans.row(), 0UL );

        return ans;
    }

    template< typename Type, typename Allocator >
    matrix<Type, Allocator> const conv( matrix<Type, Allocator> const& A, matrix<Type, Allocator> const& B, std::string const& mode ) noexcept
    {
        auto const& default_conv = conv( A, B );

        auto const [ra, ca] = A.shape();
        auto const [rb, cb] = B.shape();

        if ( mode == std::string{"same"} )
        {
            better_assert( rb > 1, " For a convolution in 'same' mode, the row of the second matrix is at least 1, but now has ", rb );
            better_assert( rb > 1, " For a convolution in 'same' mode, the column of the second matrix is at least 1, but now has ", cb );
            return { default_conv, { (rb-1)>>1, ra + ((rb-1)>>1) }, { (cb-1)>>1, ca + ((cb-1)>>1) } };
        }

        if ( mode == std::string{"valid"} )
        {
            better_assert( ra >= rb-1, " For a convolution in 'valid' mode, the row of first matrix is supposed to be at least larger than the second matrix's row by 1",
                           " but the first matrix is with row ", ra, " and the second matrix is with row ", rb );
            better_assert( ca >= cb-1, " For a convolution in 'valid' mode, the column of first matrix is supposed to be at least larger than the second matrix's column by 1",
                           " but the first matrix is with column ", ca, " and the second matrix is with column ", cb );
            return { default_conv, { rb-1, ra }, { cb-1, ca } };
        }

        return default_conv;
    }

    template< typename ... Args >
    auto conv2( Args const& ... args )
    {
        return conv( args... );
    }

    inline std::optional<std::array<matrix<std::uint8_t>,3>> load_bmp( std::string const& file_path )
    {
        std::ifstream ifs{file_path, std::ios::binary};
        if ( !ifs )
        {
            std::cerr << "load_bmp::Failed to open file " << file_path << "\n";
            return {}; // <- failed to load file
        }

        std::vector<std::uint8_t> const file_content{(std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>())};
        if( file_content.size() <= 54 )
        {
            std::cerr << "load_bmp::file " << file_path << " has contents less than 54 byte.\n";
            return {}; // <- failed for bad file
        }

        std::uint_least64_t const col = std::uint_least64_t{file_content[18]}         | (std::uint_least64_t{file_content[19]} << 8) |
                                        (std::uint_least64_t{file_content[20]} << 16) | (std::uint_least64_t{file_content[21]} << 24);
        std::uint_least64_t const row = std::uint_least64_t{file_content[22]}         | (std::uint_least64_t{file_content[23]} << 8) |
                                        (std::uint_least64_t{file_content[24]} << 16) | (std::uint_least64_t{file_content[25]} << 24);
        std::uint_least64_t const padding = ( 4 - ( ( col * 3 ) & 0x3 ) ) & 0x3;

        if ( 54+(3*col+padding)*row != file_content.size() )
        {
            std::cerr << "load_bmp::file " << file_path << " has insistant contents with its header.\n";
            std::cerr << "Additional Information: col-" << col << ", row-" << row << ", padding-" << padding << ", file size-" << file_content.size() << "\n";
            std::cerr << "Expected size-" << 54+(3*col+padding)*row << ", but the file has " << file_content.size() << " bytes\n";
            return {}; //<- error with the data size
        }

        std::array<matrix<std::uint8_t>,3> ans;
        for ( auto& mat : ans ) mat.resize( row, col );
        auto& [mat_r, mat_g, mat_b] = ans;

        auto pos_itor = file_content.begin()+54;
        for ( auto r : matrix_details::range( row ) )
        {
            for ( auto c : matrix_details::range( col ) )
            {
                mat_b[row-r-1][c] = *pos_itor++;
                mat_g[row-r-1][c] = *pos_itor++;
                mat_r[row-r-1][c] = *pos_itor++;
            }
            std::advance( pos_itor, padding );
        }

        return {ans};
    }

    template< typename T, typename A >
    auto pooling( matrix<T,A> const& mat, std::uint_least64_t dim_r, std::uint_least64_t dim_c, std::string const& pooling_action = "mean" )
    {
        if (dim_r == 0 || dim_c == 0) return matrix<T,A>{};

        if (dim_r==1 && dim_c==1) return mat;

        std::map< std::string, std::pair<T, std::function< T(T, T) > > > function_list =
        {
            std::make_pair
            (
                "mean",
                std::make_pair
                (
                    T{0},
                    [dim_r, dim_c]( T v1, T v2 ){ return v1 + v2 / ( static_cast<T>(dim_r*dim_c) ); }
                )
            ),
            std::make_pair//alias operation of 'mean'
            (
                "average",
                std::make_pair
                (
                    T{0},
                    [dim_r, dim_c]( T v1, T v2 ){ return v1 + v2 / ( static_cast<T>(dim_r*dim_c) ); }
                )
            ),
            std::make_pair
            (
                "max",
                std::make_pair
                (
                    std::numeric_limits<T>::min(),
                    []( T v1, T v2 ){ return std::max(v1, v2); }
                )
            ),
            std::make_pair
            (
                "min",
                std::make_pair
                (
                    std::numeric_limits<T>::max(),
                    []( T v1, T v2 ){ return std::min(v1, v2); }
                )
            )
        };

        auto iterator = function_list.find( pooling_action );
        better_assert( iterator != function_list.end(), "Error: Unknow pooling action [[", pooling_action, "]], only [mean], [max], [min] supported!" );

        auto init_value =(*iterator).second.first;
        auto const& the_function =(*iterator).second.second;
        auto const [row, col] = mat.shape();
        auto const [new_row, new_col] = std::make_pair( row/dim_r, col/dim_c );
        matrix< T, A > ans{ new_row, new_col, T{0} };

        auto const& make_pooling = [&ans, &mat, &the_function, new_col=new_col, dim_r, dim_c, init_value]( std::uint_least64_t r )
        {
            for ( auto c : matrix_details::range( new_col ) )
            {
                ans[r][c] = init_value;
                for ( auto rr : matrix_details::range( dim_r ) )
                    ans[r][c] = std::accumulate( mat.row_begin(r*dim_r+rr)+c*dim_c, mat.row_begin(r*dim_r+rr)+c*dim_c+dim_c, ans[r][c], the_function );
            }
        };

        matrix_details::parallel( make_pooling, 0UL, new_row, 0UL );

        return ans;
    }

    template< typename T, typename A >
    auto pooling( matrix<T,A> const& mat, std::uint_least64_t dim, std::string const& pooling_action = "mean"  )
    {
        return pooling( mat, dim, dim, pooling_action );
    }

    template< typename T, typename A >
    void save_as_bmp( std::string const& file_name, matrix<T,A> const& red_channel, matrix<T,A> const& green_channel, matrix<T,A> const& blue_channel )
    {
        better_assert( red_channel.row()==green_channel.row(), "Row not match for red and green matrix! The row for red is ", red_channel.row(), " but for green is ", green_channel.row() );
        better_assert( red_channel.row()==blue_channel.row(), "Row not match for red and blue matrix! The row for red is ", red_channel.row(), " but for blue is ", blue_channel.row() );
        better_assert( red_channel.col()==green_channel.col(), "Col not match for red and green matrix! The col for red is ", red_channel.col(), " but for green is ", green_channel.col() );
        better_assert( red_channel.col()==blue_channel.col(), "Col not match for red and blue matrix! The col for red is ", red_channel.col(), " but for blue is ", blue_channel.col() );

        auto const [row, col] = red_channel.shape();

        // scale normal matrices to uint8 matrices
        matrix<std::uint8_t> channel_r{ row, col };
        matrix<std::uint8_t> channel_g{ row, col };
        matrix<std::uint8_t> channel_b{ row, col };
        auto const& [red_mx, red_mn] = std::make_pair( *std::max_element(red_channel.begin(), red_channel.end() ), *std::min_element(red_channel.begin(), red_channel.end() ) );
        auto const& [green_mx, green_mn] = std::make_pair( *std::max_element(green_channel.begin(), green_channel.end() ), *std::min_element(green_channel.begin(), green_channel.end() ) );
        auto const& [blue_mx, blue_mn] = std::make_pair( *std::max_element(blue_channel.begin(), blue_channel.end() ), *std::min_element(blue_channel.begin(), blue_channel.end() ) );
        for ( auto r : matrix_details::range(row) )
            for ( auto c : matrix_details::range(col) )
            {
                channel_r[r][c] = static_cast<std::uint8_t>( 256.0 * (red_channel[r][c] - red_mn) / (red_mx-red_mn+0.1) );
                channel_g[r][c] = static_cast<std::uint8_t>( 256.0 * (green_channel[r][c] - green_mn) / (green_mx-green_mn+0.1) );
                channel_b[r][c] = static_cast<std::uint8_t>( 256.0 * (blue_channel[r][c] - blue_mn) / (blue_mx-blue_mn+0.1) );
            }

        // encode rgb to bitmap stream
        auto const& encoding = matrix_details::encode_bmp_stream( channel_r, channel_g, channel_b );
        better_assert( encoding, "Failed to convert the 3 matrix to bmp stream in save_as_bmp function, where the file_name is ", file_name );

        // write file stream
        std::string new_file_name{ file_name };
        std::string const extension{ ".bmp" };
        if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
            new_file_name += extension;
        std::ofstream stream( new_file_name.c_str(), std::ios_base::out | std::ios_base::binary );
        better_assert( stream, "Failed to create file ", new_file_name, " when executing save_as_bmp with file_name = ", file_name, " and row = ", row, " col = ", col );
        stream.write( reinterpret_cast<char const*>((*encoding).data()), (*encoding).size() );
    }

    template< typename T, typename A >
    void save_as_bmp( std::string const& file_name, matrix<T,A> const& mat, std::string const& colormap = std::string{"parula"} )
    {
        mat.save_as_bmp( file_name, colormap );
    }

    template< typename T >
    auto meshgrid( T const& x, T const& y ) noexcept
    {
        unsigned long const row = static_cast<unsigned long>( y );
        unsigned long const col = static_cast<unsigned long>( x );

        matrix<T> mat_x{ row, col, T{} };
        matrix<T> mat_y{ row, col, T{} };
        /*
        for ( auto r = 0UL; r != row; ++r )
            for ( auto c = 0UL; c != col; ++c )
            {
                mat_x[r][c] = static_cast<T>(r);
                mat_y[r][c] = static_cast<T>(c);
            }
        */
        auto const& parallel_func = [&]( unsigned long r )
        {
            for ( auto c = 0UL; c != col; ++c )
            {
                mat_x[r][c] = static_cast<T>(r);
                mat_y[r][c] = static_cast<T>(c);
            }
        };

        matrix_details::parallel( parallel_func, 0UL, row, 0UL );

        return std::make_pair( mat_y, mat_x );
    }

    static auto const& fma = matrix_details::map( []( auto const& val, auto const& wal, auto const& xal ){ return std::fma(val, wal, xal); } );

    static auto const& ldexp = matrix_details::map( []( auto const& val, auto const& wal ){ return std::ldexp(val, wal); } );

    static auto const& scalbn = matrix_details::map( []( auto const& val, auto const& wal ){ return std::scalbn(val, wal); } );

    static auto const& scalbln = matrix_details::map( []( auto const& val, auto const& wal ){ return std::scalbln(val, wal); } );

    static auto const& pow = matrix_details::map( []( auto const& val, auto const& wal ){ return std::pow(val, wal); } );

    static auto const& hypot = matrix_details::map( []( auto const& val, auto const& wal ){ return std::hypot(val, wal); } );

    static auto const& fmod = matrix_details::map( []( auto const& val, auto const& wal ){ return std::fmod(val, wal); } );

    static auto const& remainder = matrix_details::map( []( auto const& val, auto const& wal ){ return std::remainder(val, wal); } );

    static auto const& copysign = matrix_details::map( []( auto const& val, auto const& wal ){ return std::copysign(val, wal); } );

    static auto const& nextafter = matrix_details::map( []( auto const& val, auto const& wal ){ return std::nextafter(val, wal); } );

    static auto const& fdim = matrix_details::map( []( auto const& val, auto const& wal ){ return std::fdim(val, wal); } );

    static auto const& fmax = matrix_details::map( []( auto const& val, auto const& wal ){ return std::fmax(val, wal); } );

    static auto const& fmin = matrix_details::map( []( auto const& val, auto const& wal ){ return std::fmin(val, wal); } );

    static auto const& cos = matrix_details::map( []( auto const& val ){ return std::cos(val); } );

    static auto const& sin = matrix_details::map( []( auto const& val ){ return std::sin(val); } );

    static auto const& tan = matrix_details::map( []( auto const& val ){ return std::tan(val); } );

    static auto const& acos = matrix_details::map( []( auto const& val ){ return std::acos(val); } );

    static auto const& asin = matrix_details::map( []( auto const& val ){ return std::asin(val); } );

    static auto const& atan = matrix_details::map( []( auto const& val ){ return std::atan(val); } );

    static auto const& cosh = matrix_details::map( []( auto const& val ){ return std::cosh(val); } );

    static auto const& sinh = matrix_details::map( []( auto const& val ){ return std::sinh(val); } );

    static auto const& tanh = matrix_details::map( []( auto const& val ){ return std::tanh(val); } );

    static auto const& acosh = matrix_details::map( []( auto const& val ){ return std::acosh(val); } );

    static auto const& asinh = matrix_details::map( []( auto const& val ){ return std::asinh(val); } );

    static auto const& atanh = matrix_details::map( []( auto const& val ){ return std::atanh(val); } );

    static auto const& exp = matrix_details::map( []( auto const& val ){ return std::exp(val); } );

    static auto const& log = matrix_details::map( []( auto const& val ){ return std::log(val); } );

    static auto const& log10 = matrix_details::map( []( auto const& val ){ return std::log10(val); } );

    static auto const& exp2 = matrix_details::map( []( auto const& val ){ return std::exp2(val); } );

    static auto const& expm1 = matrix_details::map( []( auto const& val ){ return std::expm1(val); } );

    static auto const& log1p = matrix_details::map( []( auto const& val ){ return std::log1p(val); } );

    static auto const& log2 = matrix_details::map( []( auto const& val ){ return std::log2(val); } );

    static auto const& logb = matrix_details::map( []( auto const& val ){ return std::logb(val); } );

    static auto const& sqrt = matrix_details::map( []( auto const& val ){ return std::sqrt(val); } );

    static auto const& cbrt = matrix_details::map( []( auto const& val ){ return std::cbrt(val); } );

    static auto const& erf = matrix_details::map( []( auto const& val ){ return std::erf(val); } );

    static auto const& erfc = matrix_details::map( []( auto const& val ){ return std::erfc(val); } );

    static auto const& tgamma = matrix_details::map( []( auto const& val ){ return std::tgamma(val); } );

    static auto const& lgamma = matrix_details::map( []( auto const& val ){ return std::lgamma(val); } );

    static auto const& ceil = matrix_details::map( []( auto const& val ){ return std::ceil(val); } );

    static auto const& floor = matrix_details::map( []( auto const& val ){ return std::floor(val); } );

    static auto const& trunc = matrix_details::map( []( auto const& val ){ return std::trunc(val); } );

    static auto const& round = matrix_details::map( []( auto const& val ){ return std::round(val); } );

    static auto const& rint = matrix_details::map( []( auto const& val ){ return std::rint(val); } );

    static auto const& nearbyint = matrix_details::map( []( auto const& val ){ return std::nearbyint(val); } );

    static auto const& fabs = matrix_details::map( []( auto const& val ){ return std::fabs(val); } );

    static auto const& abs = matrix_details::map( []( auto const& val ){ return std::abs(val); } );

    static auto const& ilogb = matrix_details::map( []( auto const& val ){ return std::ilogb(val); } );

    static auto const& lround = matrix_details::map( []( auto const& val ){ return std::lround(val); } );

    static auto const& llround = matrix_details::map( []( auto const& val ){ return std::llround(val); } );

    static auto const& lrint = matrix_details::map( []( auto const& val ){ return std::lrint(val); } );

    static auto const& llrint = matrix_details::map( []( auto const& val ){ return std::llrint(val); } );

    // special
    static auto const& remquo = matrix_details::map( []( auto const& num, auto const& den ){ int quo; auto const ans = std::remquo(num, den, &quo); return std::make_tuple(ans, quo); } );

    //
    // Complex Functions
    //
    static auto const& real = matrix_details::map( []( auto const& val ){ return std::real(val); } );

    static auto const& imag = matrix_details::map( []( auto const& val ){ return std::imag(val); } );

    static auto const& arg = matrix_details::map( []( auto const& val ){ return std::arg(val); } );

    static auto const& norm = matrix_details::map( []( auto const& val ){ return std::norm(val); } );

    static auto const& conj = matrix_details::map( []( auto const& val ){ return std::conj(val); } );

    static auto const& polar = matrix_details::map( []( auto const& val ){ return std::polar(val); } );

    static auto const& proj = matrix_details::map( []( auto const& val ){ return std::proj(val); } );

    // already previously defined
    // abs
    // sin
    // cos
    // ...
    //

    // reduce functions
    //
    static auto const& sum = matrix_details::reduce( []( auto x, auto y ) noexcept { return x+y; }, 0.0 );

    static auto const& mean = []( auto const& mat ) noexcept { return sum(mat) / mat.size(); };

    static auto const& variance = []( auto const& mat ) noexcept { return sum( pow( mat - mean( mat ), 2.0 ) ) / mat.size(); };

    static auto const& standard_deviation = []( auto const& mat ) noexcept
    {
        if ( mat.size() <= 1 )
            return 0.0;
        return std::sqrt( sum( pow( mat-mean( mat ), 2.0 ) ) / ( mat.size() - 1 ) );
    };

    static auto const& clip = []( auto lower, auto upper ) noexcept
    {
        better_assert( lower < upper, "Error in clip(lower, upper): lower ", lower, " is supposed to be smaller than upper ", upper );
        return [=]( auto const& mat ) noexcept
        {
            auto ans{ mat };
            auto&& impl = [=]( auto & v ) noexcept
            {
                v = v < lower ? lower :
                    v > upper ? upper :
                                v;
            };
            ans.map( impl );
            return ans;
        };
    };

} //namespace feng

//#undef better_assert

#endif

