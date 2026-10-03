#ifndef FENG_MATRIX_HPP_INCLUDED_
#define FENG_MATRIX_HPP_INCLUDED_

static_assert( __cplusplus >= 202002L, "C++20 is a must for this library, please update your compiler, or enable corresponding option such as -std=c++2a" );

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <chrono>
#include <cctype>
#include <cmath>
#include <compare>
#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
#include <ranges>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <span>
#include <valarray>
#include <vector>
#include <version>
#if defined( __cpp_lib_mdspan )
#include <mdspan>
#endif
#if defined( __cpp_lib_expected )
#include <expected>
#endif

// S9-R3 (D-033): configuration macros. FENG_MATRIX_PARALLEL runs elementwise loops in parallel, FENG_MATRIX_OPENCV
// enables the cv::Mat interface, FENG_MATRIX_CHECKED_ITERATORS (D-020) bounds-checks view iterators. The old names
// PARALLEL and OPENCV are deprecated and still accepted: each maps to its FENG_MATRIX_ name here, and the library
// below tests only the new names.
#if defined( PARALLEL ) && !defined( FENG_MATRIX_PARALLEL )
#define FENG_MATRIX_PARALLEL
#endif
#if defined( OPENCV ) && !defined( FENG_MATRIX_OPENCV )
#define FENG_MATRIX_OPENCV
#endif

#ifdef FENG_MATRIX_OPENCV // Interfacing cv::Mat. Enable this feature by passing `-DFENG_MATRIX_OPENCV` to the compiler.
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#endif//FENG_MATRIX_OPENCV

namespace feng
{
    constexpr std::uint_least64_t matrix_version = 20240314ULL;

    #ifdef FENG_MATRIX_PARALLEL
    constexpr std::uint_least64_t parallel_mode = 1;
    #else
    constexpr std::uint_least64_t parallel_mode = 0;
    #endif

    #ifdef NDEBUG
    constexpr std::uint_least64_t debug_mode = 0;
    #else
    constexpr std::uint_least64_t debug_mode = 1;
    #endif

    #ifdef FENG_MATRIX_OPENCV
    constexpr std::uint_least64_t enable_cv_mat = 1;
    #else
    constexpr std::uint_least64_t enable_cv_mat = 0;
    #endif

    // S9-R3 (D-033): the configuration (parallel, checked iterators, OpenCV) names a namespace holding one inline
    // variable, kept even in a TU that uses nothing from the library. On ELF, unless FENG_MATRIX_NO_CONFIG_CHECK, an
    // extern "C" alias to that variable sits in the variable's COMDAT group: TUs with the same configuration fold
    // into one group, TUs with different ones each keep theirs and the link fails with a multiple definition of
    // feng_matrix_configuration_mismatch_between_translation_units. NDEBUG is not part of the configuration.
    #ifdef FENG_MATRIX_PARALLEL
    #define FENG_MATRIX_CONFIG_P_ 1
    #else
    #define FENG_MATRIX_CONFIG_P_ 0
    #endif
    #ifdef FENG_MATRIX_CHECKED_ITERATORS
    #define FENG_MATRIX_CONFIG_C_ 1
    #else
    #define FENG_MATRIX_CONFIG_C_ 0
    #endif
    #ifdef FENG_MATRIX_OPENCV
    #define FENG_MATRIX_CONFIG_O_ 1
    #else
    #define FENG_MATRIX_CONFIG_O_ 0
    #endif
    // matrix_config_p<P>_c<C>_o<O> has 22 characters, hence the Itanium name _ZN4feng22matrix_config_p<P>_c<C>_o<O>3tagE.
    #define FENG_MATRIX_CONFIG_NS2_( P, C, O ) matrix_config_p ## P ## _c ## C ## _o ## O
    #define FENG_MATRIX_CONFIG_NS_( P, C, O ) FENG_MATRIX_CONFIG_NS2_( P, C, O )
    #define FENG_MATRIX_CONFIG_MANGLED2_( P, C, O ) _ZN4feng22matrix_config_p ## P ## _c ## C ## _o ## O ## 3tagE
    #define FENG_MATRIX_CONFIG_MANGLED_( P, C, O ) FENG_MATRIX_CONFIG_MANGLED2_( P, C, O )
    #define FENG_MATRIX_CONFIG_STR2_( X ) #X
    #define FENG_MATRIX_CONFIG_STR_( X ) FENG_MATRIX_CONFIG_STR2_( X )

    namespace FENG_MATRIX_CONFIG_NS_( FENG_MATRIX_CONFIG_P_, FENG_MATRIX_CONFIG_C_, FENG_MATRIX_CONFIG_O_ )
    {
        [[gnu::used]] inline char tag = 0;
    }
}//namespace feng

#if defined( __ELF__ ) && !defined( FENG_MATRIX_NO_CONFIG_CHECK )
extern "C" char feng_matrix_configuration_mismatch_between_translation_units
    __attribute__(( alias( FENG_MATRIX_CONFIG_STR_( FENG_MATRIX_CONFIG_MANGLED_( FENG_MATRIX_CONFIG_P_, FENG_MATRIX_CONFIG_C_, FENG_MATRIX_CONFIG_O_ ) ) ) ));
#endif

#undef FENG_MATRIX_CONFIG_STR_
#undef FENG_MATRIX_CONFIG_STR2_
#undef FENG_MATRIX_CONFIG_MANGLED_
#undef FENG_MATRIX_CONFIG_MANGLED2_
#undef FENG_MATRIX_CONFIG_NS_
#undef FENG_MATRIX_CONFIG_NS2_
#undef FENG_MATRIX_CONFIG_O_
#undef FENG_MATRIX_CONFIG_C_
#undef FENG_MATRIX_CONFIG_P_

namespace feng
{


    namespace matrix_private
    {
        // S2-R1 (D-003, D-011): the one contract-violation path. Formats a single line, writes it to stderr with
        // one fwrite, flushes and aborts; used in every build mode, never throws.
        template< typename... Args >
        [[noreturn]] void contract_violation( char const* expr, char const* file, int line, Args const&... detail ) noexcept
        {
            std::ostringstream os;
            os.precision( 20 );
            os << "feng::matrix: contract violation: " << expr << " (" << file << ":" << line << ")";
            if constexpr( sizeof...( Args ) > 0 )
            {
                os << ' ';
                ( os << ... << detail );
            }
            os << '\n';
            std::string const msg = os.str();
            std::fwrite( msg.data(), 1, msg.size(), stderr );
            std::fflush( stderr );
            std::abort();
        }
    }

    #ifdef FENG_MATRIX_EXPECTS
    #undef FENG_MATRIX_EXPECTS
    #endif
    // FENG_MATRIX_EXPECTS( expr, detail... ): when `expr` is false, report it and abort, in debug and NDEBUG builds.
    #define FENG_MATRIX_EXPECTS(EXPRESSION, ... ) ((EXPRESSION) ? (void)0 : ::feng::matrix_private::contract_violation( #EXPRESSION, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__ ))

    #ifdef better_assert
    #undef better_assert
    #endif
    //
    // legacy name (D-002), forwards to FENG_MATRIX_EXPECTS, usage:
    //
    // better_assert( a > 0 );
    // better_assert( a > 0, "a is expected larger than 0, but now a = ", a ); // detail appended to the message
    //
    #define better_assert(EXPRESSION, ... ) FENG_MATRIX_EXPECTS( EXPRESSION __VA_OPT__(,) __VA_ARGS__ )

    namespace matrix_private
    {
        // S2-R2 (D-012): element count for a rows x cols allocation; aborts on rows*cols overflow, on a byte count
        // that overflows or exceeds PTRDIFF_MAX (F02), or above allocator_traits<Alloc>::max_size, before anything
        // is allocated.
        template< typename Alloc >
        std::size_t checked_count( Alloc const& alloc, std::size_t r, std::size_t c ) noexcept
        {
            typedef typename std::allocator_traits< Alloc >::value_type value_type;
            constexpr std::size_t size_max = std::numeric_limits< std::size_t >::max();
            FENG_MATRIX_EXPECTS( r == 0 || c <= size_max / r, "matrix size: rows*cols overflows, rows = ", r, ", cols = ", c );
            std::size_t const n = r * c;
            FENG_MATRIX_EXPECTS( n <= size_max / sizeof( value_type ), "matrix size: byte count overflows, rows = ", r, ", cols = ", c );
            FENG_MATRIX_EXPECTS( n * sizeof( value_type ) <= static_cast< std::size_t >( PTRDIFF_MAX ), "matrix size: byte count above PTRDIFF_MAX, rows = ", r, ", cols = ", c );
            FENG_MATRIX_EXPECTS( n <= static_cast< std::size_t >( std::allocator_traits< Alloc >::max_size( alloc ) ), "matrix size: rows*cols above the allocator's max_size, rows = ", r, ", cols = ", c );
            return n;
        }

        // S3-R5: checks a rows x cols allocation against alloc, then returns alloc for the owning container.
        template< typename Alloc >
        Alloc checked_allocator( Alloc alloc, std::size_t r, std::size_t c ) noexcept
        {
            checked_count( alloc, r, c );
            return alloc;
        }

        // S3-R1: the one access point to matrix's private storage and extents, used by the CRTP bases.
        struct storage_access
        {
            template< typename M >
            static auto& storage( M& m ) noexcept { return m.storage_; }
            template< typename M >
            static auto& rows( M& m ) noexcept { return m.row_; }
            template< typename M >
            static auto& cols( M& m ) noexcept { return m.col_; }
        };

        // S2-R2: a signed or unsigned dimension is usable when it is non-negative and fits std::size_t.
        template< std::integral I >
        constexpr bool dimension_fits( I v ) noexcept
        {
            if constexpr( std::is_signed_v< I > )
                if ( v < 0 ) return false;
            return static_cast< std::uintmax_t >( v ) <= std::numeric_limits< std::size_t >::max();
        }

        // S2-R2: clone bounds, checked before the source is read or anything is allocated.
        inline void check_clone_lists( std::size_t rows_listed, std::size_t cols_listed ) noexcept
        {
            FENG_MATRIX_EXPECTS( rows_listed == 2, "matrix clone: the row range needs exactly two values, got ", rows_listed );
            FENG_MATRIX_EXPECTS( cols_listed == 2, "matrix clone: the column range needs exactly two values, got ", cols_listed );
        }
        inline void check_clone_range( std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1, std::size_t rows, std::size_t cols ) noexcept
        {
            FENG_MATRIX_EXPECTS( r0 < r1 && r1 <= rows, "matrix clone: row range [", r0, ", ", r1, ") outside a source with ", rows, " rows" );
            FENG_MATRIX_EXPECTS( c0 < c1 && c1 <= cols, "matrix clone: column range [", c0, ", ", c1, ") outside a source with ", cols, " columns" );
        }

        // S2-R2: the one two-index check behind at( r, c ) and m( r, c ); aborts with `index` when out of range.
        inline void check_index( std::size_t r, std::size_t c, std::size_t rows, std::size_t cols ) noexcept
        {
            FENG_MATRIX_EXPECTS( r < rows && "Row index out of boundary!", "matrix index: (", r, ", ", c, ") on a ", rows, "x", cols, " matrix" );
            FENG_MATRIX_EXPECTS( c < cols && "Column index out of boundary!", "matrix index: (", r, ", ", c, ") on a ", rows, "x", cols, " matrix" );
        }

        // S3-R3 (D-018): true for std::complex specializations.
        template< typename T >
        inline constexpr bool is_std_complex_v = false;
        template< typename T >
        inline constexpr bool is_std_complex_v< std::complex< T > > = true;
    }

    //
    // begin of concept allocators
    //
    template< typename T, typename=void >
    struct has_value_type : std::false_type{};

    template< typename T >
    struct has_value_type<T, std::void_t<typename T::value_type>> : std::true_type{};

    template< typename T >
    inline constexpr bool has_value_type_v = has_value_type<T>::value;

    template< typename T, typename=void >
    struct has_allocate : std::false_type {};

    template< typename T >
    struct has_allocate<T, std::void_t<decltype(std::declval<T&>().allocate(1UL)) >> : std::true_type{};

    template< typename T >
    inline constexpr bool has_allocate_v = has_allocate<T>::value;

    template< typename T ,typename=void >
    struct has_deallocate : std::false_type{};

    template< typename T >
    struct has_deallocate<T, std::void_t<decltype(std::declval<T&>().deallocate(std::declval<typename T::value_type*>(), 1UL))>> : std::true_type {};

    template<typename T>
    inline constexpr bool has_deallocate_v = has_deallocate<T>::value;

    template< typename T>
    concept Allocator = has_value_type_v<T> && has_allocate_v<T> && has_deallocate_v<T>;


    //
    // end of concept allocators
    //


    // S3-R3 (PR-5, D-012): element types of matrix and matrix_view; bool is excluded (no contiguous T* storage).
    template < typename T >
    concept matrix_element = std::is_object_v< T > && !std::is_const_v< T > && !std::is_volatile_v< T > &&
                             !std::is_same_v< T, bool > &&
                             // D-018: the size constructors default-construct elements. std::complex is carved out
                             // because libstdc++ declares complex() without noexcept, though it only value-initializes two arithmetic members.
                             ( std::is_nothrow_default_constructible_v< T > || matrix_private::is_std_complex_v< T > ) &&
                             std::is_nothrow_copy_constructible_v< T > && std::is_nothrow_move_constructible_v< T > &&
                             std::is_nothrow_copy_assignable_v< T > && std::is_nothrow_move_assignable_v< T > &&
                             std::is_nothrow_destructible_v< T >;

    template < matrix_element Type, Allocator Alloc >
    struct matrix;

    template < matrix_element Type, Allocator Alloc >
    struct matrix_view;

    template < matrix_element Type, Allocator Alloc >
    struct mutable_matrix_view;

    namespace matrix_details
    {

        template< typename Integer_Type >
        struct integer_iterator
        {
            static_assert(std::is_integral_v<Integer_Type>, "Integral required");

            typedef Integer_Type                    value_type;
            typedef value_type&                     reference;
            typedef value_type*                     pointer;
            typedef reference                       const_reference; // same type as before: const on a reference is ignored
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

            self_type operator++(int) noexcept
            {
                self_type ans{*this};
                ++(*this);
                return ans;
            }

            friend difference_type operator - ( self_type const& lhs, self_type const& rhs ) noexcept
            {
                return lhs.value_ - rhs.value_;
            }

            friend self_type operator + ( self_type const& lhs, difference_type const& rhs ) noexcept
            {
                return self_type{ lhs.value_ + rhs };
            }

            friend self_type operator + ( difference_type const& lhs, self_type const& rhs ) noexcept
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


        template< std::weakly_incrementable W >
        constexpr auto range( W val_begin, W val_end ) noexcept
        {
            return std::ranges::iota_view( val_begin, val_end );
        }

        template< std::weakly_incrementable W >
        constexpr auto range( W val_end ) noexcept
        {
            return range( W{0}, val_end );
        }


        template<std::integral Integer_Type, typename Function>
        void repeat( Function function, Integer_Type n ) noexcept
        {
            while ( n-- )
                function();
        }

        // S6-R1 (F11, F12): the one partition behind parallel and both reductions. For n = last - first > 0 the
        // effective worker count is w = clamp( workers, 1, n ) (0 means serial); an empty or reversed range has
        // w = 0. Chunk k holds n / w + ( k < n % w ) indices starting at first + k * ( n / w ) + min( k, n % w ),
        // so the w chunks are contiguous, disjoint, non-empty and cover [first, last) in order.
        template< std::integral Integer_Type >
        constexpr std::size_t effective_workers( Integer_Type first, Integer_Type last, std::size_t workers ) noexcept
        {
            if ( !( first < last ) ) return 0;
            std::size_t const n = static_cast<std::size_t>( last - first );
            return std::clamp( workers, std::size_t{1}, n );
        }

        // [begin, end) of chunk k; an empty pair for an empty range or k >= effective_workers( first, last, workers ).
        template< std::integral Integer_Type >
        constexpr std::pair<Integer_Type, Integer_Type> chunk_bounds( Integer_Type first, Integer_Type last, std::size_t workers, std::size_t k ) noexcept
        {
            std::size_t const w = effective_workers( first, last, workers );
            if ( k >= w ) return { first < last ? last : first, first < last ? last : first };
            std::size_t const n = static_cast<std::size_t>( last - first );
            std::size_t const q = n / w;
            std::size_t const r = n % w;
            std::size_t const offset = k * q + std::min( k, r );
            Integer_Type const b = static_cast<Integer_Type>( first + static_cast<Integer_Type>( offset ) );
            Integer_Type const e = static_cast<Integer_Type>( b + static_cast<Integer_Type>( q + ( k < r ? 1 : 0 ) ) );
            return { b, e };
        }

        // Runs func( i ) once for every i in [first, last): chunk 0 on the calling thread, chunks 1..w-1 on
        // std::jthreads that are all joined before return. The injected count is honoured in every build. A
        // thread-creation failure terminates (noexcept, D-012); func must not throw.
        template< typename Function, std::integral Integer_Type >
        void parallel_workers( Function const& func, Integer_Type first, Integer_Type last, std::size_t workers ) noexcept
        {
            std::size_t const w = effective_workers( first, last, workers );
            if ( w == 0 ) return;
            auto const run_chunk = [&func, first, last, w]( std::size_t k ) noexcept
            {
                auto const [b, e] = chunk_bounds( first, last, w, k );
                for ( Integer_Type i = b; i != e; ++i )
                    func( i );
            };
            if ( w == 1 )
            {
                run_chunk( 0 );
                return;
            }
            std::vector<std::jthread> threads;
            threads.reserve( w - 1 );
            for ( std::size_t k = 1; k != w; ++k )
                threads.emplace_back( run_chunk, k );
            run_chunk( 0 );
            for ( auto& th : threads )
                th.join();
        }

        // 1 in serial builds or for n <= threshold, else hardware_concurrency (1 if it reports 0).
        inline std::size_t default_workers( std::size_t n, std::size_t threshold ) noexcept
        {
            if constexpr( parallel_mode == 0 )
            {
                (void)n; (void)threshold;
                return 1;
            }
            else
            {
                if ( n <= threshold ) return 1;
                unsigned int const cores = std::thread::hardware_concurrency();
                return cores == 0 ? std::size_t{1} : static_cast<std::size_t>( cores );
            }
        }

        // S10-R3 (D-008, kept optimization par-thresholds): work-based default worker counts. In parallel builds a
        // call doing `work` units runs on min( hardware_concurrency, work / grain ) workers, at least 1, so a call
        // with less than two grains of work stays on the calling thread; serial builds always return 1. Explicit
        // worker counts (parallel_workers, reduce_range, the trailing reduce argument) are not affected.
        // Grains, measured on the S10 host (Ryzen 9 7900X3D, 24 threads, g++ -O2 -pthread; lane numbers in
        // bench/results.md "par-thresholds"): starting and joining one std::jthread costs about 19 us, so a worker
        // pays off only once it takes over more than that much work. Per-call medians, 1 worker against the best
        // count, warm inputs: sqrt via for_each 2^15 doubles 54 us (1) vs 58 us (2), 2^17 211 us vs 111 us (4); sum
        // 2^16 38 us (1) vs 49 us (2), 2^18 151 us vs 90 us (3); GEMM 64^3 (2^18 multiply-adds) 51 us (1) vs 58 us
        // (3), 96^3 171 us vs 105 us (4), 1024x1024 by 1024x1 171 us vs 105 us (4). The elementwise grain comes from
        // the bench workload par/add/1000x1000 (a + b into a fresh result): grain 2^16 (15 workers) 0.75-0.89 ms,
        // 2^17 (7) 0.84-0.90 ms, 2^18 (3) 0.98-1.06 ms, 24 workers (pre-S10) 0.98-1.06 ms; for map/1000x1000 a
        // callback grain of 2^16 (15 workers) beat 2^13-2^15 (24).
        inline constexpr std::size_t elementwise_grain = std::size_t{ 1 } << 16; // elements: add, minus, negate, copy, clone
        inline constexpr std::size_t callback_grain    = std::size_t{ 1 } << 16; // elements: for_each, apply, colormap, pooling
        inline constexpr std::size_t reduce_grain      = std::size_t{ 1 } << 16; // elements folded: reduce, sum, min, max
        inline constexpr std::size_t gemm_grain        = std::size_t{ 1 } << 18; // multiply-adds M*K*N; workers <= M rows

        inline std::size_t work_workers( std::size_t work, std::size_t grain ) noexcept
        {
            if constexpr( parallel_mode == 0 )
            {
                (void)work; (void)grain;
                return 1;
            }
            else
            {
                std::size_t const want = work / std::max( grain, std::size_t{ 1 } );
                if ( want <= 1 ) return 1;
                unsigned int const cores = std::thread::hardware_concurrency();
                std::size_t const cap = cores == 0 ? std::size_t{ 1 } : static_cast<std::size_t>( cores );
                return std::min( want, cap );
            }
        }

        // a * b, saturated at SIZE_MAX (work amounts only).
        constexpr std::size_t saturating_work( std::size_t a, std::size_t b ) noexcept
        {
            if ( a != 0 && b > std::numeric_limits<std::size_t>::max() / a ) return std::numeric_limits<std::size_t>::max();
            return a * b;
        }

        // Runs func( i ) for i in [first, last) on work_workers( ( last - first ) * work_per_index, grain ) workers.
        template< typename Function, std::integral Integer_Type >
        void parallel_work( Function const& func, Integer_Type first, Integer_Type last, std::size_t work_per_index, std::size_t grain ) noexcept
        {
            std::size_t const n = first < last ? static_cast<std::size_t>( last - first ) : 0;
            parallel_workers( func, first, last, work_workers( saturating_work( n, work_per_index ), grain ) );
        }

        template< typename Function, std::integral Integer_Type >
        void parallel( Function const& func, Integer_Type dim_first, Integer_Type dim_last, unsigned long threshold = 1024 ) noexcept // 1d parallel
        {
            std::size_t const n = dim_first < dim_last ? static_cast<std::size_t>( dim_last - dim_first ) : 0;
            parallel_workers( func, dim_first, dim_last, default_workers( n, threshold ) );
        }

        template< typename Function, typename Integer_Type >
        void parallel( Function const& func, Integer_Type dim_last ) noexcept
        {
            parallel( func, Integer_Type{0}, dim_last );
        }

        namespace bmp_details
        {
            inline std::vector<std::uint8_t> generate_bmp_header( std::uint_least64_t const the_row, std::uint_least64_t const the_col ) noexcept
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

            inline std::uint8_t operator""_u8(unsigned long long value) noexcept
            {
                return static_cast<std::uint8_t>(value);
            }

            static std::function<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>(double)>
            make_transformation_function(  std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> const& color_1, double value_1,
                                           std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> const& color_2, double value_2 ) noexcept
            {
                auto const [r1, g1, b1] = color_1;
                auto const [r2, g2, b2] = color_2;

                return [r1=r1, g1=g1, b1=b1, r2=r2, g2=g2, b2=b2, value_1, value_2]( double x ) noexcept
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
            make_color_map( std::vector<double> const& values, std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>> const& colors ) noexcept
            {
                better_assert( (values.size() == colors.size()), "make_color_map::length of values and colors not match!" );
                better_assert( std::abs(*(values.begin())) < 1.0e-10 && "make_color_map::value should start from 0!" );
                better_assert( std::abs(*(values.rbegin())-1.0) < 1.0e-10 && "make_color_map::value should end at 1!" );

                return [=]( double x ) noexcept
                {
                    for ( auto index : matrix_details::range( values.size() - 1 ) )
                        if ( x <= values[index+1] )
                            return  make_transformation_function( colors[index], values[index], colors[index+1], values[index+1] )(x);
                    better_assert( !"make_color_map::should never reach here! The input value x is ", x, ", and the values.size() is ", values.size() );
                    return std::make_tuple(0_u8, 0_u8, 0_u8);
                };
            }
            static std::function<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>(double)>
            make_color_map( std::initializer_list<double> const& values, std::initializer_list<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>> const& colors ) noexcept
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
            matrix_details::parallel_work( fill_row, 0UL, the_row, saturating_work( the_col, 3 ), elementwise_grain );

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
            Function _for_each_n( Function f, std::uint_least64_t n, InputIterator1 begin1, InputIteratorn... beginn ) noexcept
            {
                auto const& func = [&]( std::uint_least64_t idx ) { f( *(begin1+idx), *(beginn+idx)... ); };
                parallel_work( func, std::uint_least64_t{ 0 }, n, 1, callback_grain );
                return f;
            }

            template < typename Function, typename InputIterator1, typename... InputIteratorn >
            Function _for_each( Function f, InputIterator1 begin1, InputIterator1 end1, InputIteratorn... beginn ) noexcept
            {
                return _for_each_n( f, std::distance( begin1, end1 ), begin1, beginn... );
            }

            struct dummy { };

            template < typename... Types_N >
            struct for_each_impl_with_dummy
            {
                typedef typename extract_type_backward< 1, Types_N... >::result_type return_type;

                template < typename Predict, typename... Types >
                Predict impl( Predict p, dummy, Types... types ) const noexcept
                {
                    return _for_each( p, types... );
                }

                template < typename S, typename... Types >
                return_type impl( S s, Types... types ) const noexcept
                {
                    return impl( types..., s );
                }
            };
        }

        template < typename... Types >
        typename for_each_impl_private::extract_type_backward< 1, Types... >::result_type
        for_each( Types... types ) noexcept
        {
            static_assert( sizeof...( types ) > 2, "f::for_each requires at least 3 arguments" );
            return for_each_impl_private::for_each_impl_with_dummy< Types... >().impl( types..., for_each_impl_private::dummy() );
        }

        // S6-R1 (F11, F12): the one reduction. at( i ) reads element i of [0, n). Each chunk of the shared partition
        // folds from its own first element; the result is init folded with the chunk results in order, so init
        // appears exactly once and any associative func gives std::accumulate's result. Only at( i ) for i < n is
        // read, and no count is ever divided by.
        template< typename Result, typename At, typename Function >
        Result reduce_range( At const& at, std::size_t n, Result init, Function const& func, std::size_t workers ) noexcept
        {
            std::size_t const w = effective_workers( std::size_t{0}, n, workers );
            Result ans = std::move( init );
            if ( w <= 1 )
            {
                for ( std::size_t i = 0; i != n; ++i )
                    ans = func( ans, at( i ) );
                return ans;
            }
            std::vector<Result> cache( w );
            auto const chunk_func = [&]( std::size_t k ) noexcept
            {
                auto const [b, e] = chunk_bounds( std::size_t{0}, n, w, k );
                Result acc = at( b );
                for ( std::size_t i = b + 1; i != e; ++i )
                    acc = func( acc, at( i ) );
                cache[k] = std::move( acc );
            };
            parallel_workers( chunk_func, std::size_t{0}, w, w );
            for ( std::size_t k = 0; k != w; ++k )
                ans = func( ans, cache[k] );
            return ans;
        }

        template< typename Iterator >
        std::size_t iterator_range_size( Iterator begin, Iterator end ) noexcept
        {
            auto const d = std::distance( begin, end );
            return d > 0 ? static_cast<std::size_t>( d ) : std::size_t{0};
        }

        // Iterator reduce over [begin, end) with an injected worker count; the overload below defaults it to
        // work_workers( n, reduce_grain ) (S10-R3; before S10 default_workers( n, 32 )).
        template<typename Iterator, typename Function >
        typename std::invoke_result<Function, typename std::iterator_traits<Iterator>::value_type, typename std::iterator_traits<Iterator>::value_type>::type
        reduce( Iterator begin, Iterator end, typename std::iterator_traits<Iterator>::value_type init, Function const& func, std::size_t workers ) noexcept
        {
            typedef typename std::iterator_traits<Iterator>::value_type value_type;
            typedef typename std::invoke_result<Function, value_type, value_type>::type result_type;
            std::size_t const n = iterator_range_size( begin, end );
            auto const at = [begin]( std::size_t i ) noexcept -> decltype( auto ) { return *std::next( begin, static_cast<typename std::iterator_traits<Iterator>::difference_type>( i ) ); };
            return reduce_range<result_type>( at, n, static_cast<result_type>( std::move( init ) ), func, workers );
        }

        template<typename Iterator, typename Function >
        typename std::invoke_result<Function, typename std::iterator_traits<Iterator>::value_type, typename std::iterator_traits<Iterator>::value_type>::type
        reduce( Iterator begin, Iterator end, typename std::iterator_traits<Iterator>::value_type init, Function const& func ) noexcept
        {
            std::size_t const n = matrix_details::iterator_range_size( begin, end );
            return reduce( begin, end, std::move( init ), func, work_workers( n, reduce_grain ) );
        }

        inline bool create_directory_if_not_present( std::string const& file_name ) noexcept
        {
            // S5-R4 (D-011, D-012): error_code overloads only; false on failure, never throws
            std::error_code ec;
            std::filesystem::path file_path{ file_name };
            auto directory = file_path.parent_path();
            if ( directory.empty() ) return true;
            if ( std::filesystem::is_directory( directory, ec ) ) return true;
            ec.clear();
            if ( std::filesystem::exists( directory, ec ) || ec ) return false; // a non-directory, or not checkable
            std::filesystem::create_directories( directory, ec );
            return !ec && std::filesystem::is_directory( directory, ec );
        }

        // S5-R4 (D-011): prints the one stderr line of a failed writer, "<who> -- <reason>: <path>", and returns false.
        inline bool write_failed( char const* who, char const* reason, std::string const& path ) noexcept
        {
            std::cerr << who << " -- " << reason << ": " << path << "\n";
            return false;
        }

        // S5-R4: flushes and closes a written stream; true only if every write, the flush and the close succeeded.
        inline bool checked_close( std::ofstream& ofs ) noexcept
        {
            ofs.flush();
            ofs.close();
            return !ofs.fail();
        }

        // S5-R4: the common tail of a stream writer: a directory, open, write or close failure prints one line naming
        // `who` and `path` and returns false. `body` writes into the open stream.
        template < typename Body >
        bool write_stream( char const* who, std::string const& path, std::ios_base::openmode mode, Body&& body ) noexcept
        {
            if ( !create_directory_if_not_present( path ) ) return write_failed( who, "failed to create the parent directory", path );
            std::ofstream ofs( path, mode );
            if ( !ofs ) return write_failed( who, "failed to open file", path );
            body( ofs );
            if ( !ofs ) return write_failed( who, "failed to write file", path );
            if ( !checked_close( ofs ) ) return write_failed( who, "failed to write or close file", path );
            return true;
        }

        // S5-R1/S5-R2: overflow-checked size arithmetic for every external-data size computation; true when the
        // result fits in `out` (D-012: no throwing, no wrap-around).
        template < std::integral T >
        constexpr bool checked_mul( T a, T b, T& out ) noexcept
        {
            return !__builtin_mul_overflow( a, b, &out );
        }

        template < std::integral T >
        constexpr bool checked_add( T a, T b, T& out ) noexcept
        {
            return !__builtin_add_overflow( a, b, &out );
        }

        // S5-R4 (D-011): reads the whole file into `bytes`; on a directory, an open or a read failure prints one stderr
        // line "<who> -- <reason>: <path>" and returns false, leaving `bytes` unchanged. Never aborts.
        inline bool read_file( char const* path, std::vector< std::uint8_t >& bytes, char const* who = "read_file" ) noexcept
        {
            if ( path == nullptr )
            {
                std::cerr << who << " -- no file name given\n";
                return false;
            }
            std::error_code ec;
            if ( std::filesystem::is_directory( path, ec ) )
            {
                std::cerr << who << " -- is a directory: " << path << "\n";
                return false;
            }
            std::ifstream ifs( path, std::ios::binary );
            if ( !ifs )
            {
                std::cerr << who << " -- failed to open file: " << path << "\n";
                return false;
            }
            std::vector< std::uint8_t > buffer;
            std::array< char, 65536 > chunk;
            while ( ifs )
            {
                ifs.read( chunk.data(), static_cast< std::streamsize >( chunk.size() ) );
                std::streamsize const got = ifs.gcount();
                if ( got > 0 )
                    buffer.insert( buffer.end(), reinterpret_cast< std::uint8_t const* >( chunk.data() ),
                                   reinterpret_cast< std::uint8_t const* >( chunk.data() ) + got );
            }
            if ( ifs.bad() || !ifs.eof() )
            {
                std::cerr << who << " -- failed to read file: " << path << "\n";
                return false;
            }
            bytes.swap( buffer );
            return true;
        }

        // S5-R1 (D-021): the NPY dtype code of an element type; kind 0 means the type has no NPY dtype.
        template < typename T >
        struct npy_dtype
        {
            static constexpr char kind = std::is_same_v< T, float > && sizeof( T ) == 4                          ? 'f'
                                       : std::is_same_v< T, double > && sizeof( T ) == 8                         ? 'f'
                                       : std::is_same_v< T, std::complex< float > > && sizeof( T ) == 8          ? 'c'
                                       : std::is_same_v< T, std::complex< double > > && sizeof( T ) == 16        ? 'c'
                                       : std::is_integral_v< T > && !std::is_same_v< T, bool > && std::is_signed_v< T >
                                           && ( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4 || sizeof( T ) == 8 ) ? 'i'
                                       : std::is_integral_v< T > && !std::is_same_v< T, bool > && std::is_unsigned_v< T >
                                           && ( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4 || sizeof( T ) == 8 ) ? 'u'
                                       : '\0';
            static constexpr std::size_t component = kind == 'c' ? sizeof( T ) / 2 : sizeof( T ); // bytes swapped as one unit
        };

        // S5-R1: a cursor over the NPY header text; every read checks the bound first.
        struct npy_header_cursor
        {
            char const* p;
            char const* end;

            bool at_end() const noexcept { return p == end; }
            char peek() const noexcept { return p == end ? '\0' : *p; }
            void skip_ws() noexcept
            {
                while ( p != end && ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' ) ) ++p;
            }
            bool eat( char c ) noexcept
            {
                if ( p == end || *p != c ) return false;
                ++p;
                return true;
            }
            // A Python string literal in ' or " quotes without escapes; [b, e) receives its contents.
            bool string_literal( char const*& b, char const*& e ) noexcept
            {
                if ( p == end || ( *p != '\'' && *p != '"' ) ) return false;
                char const quote = *p++;
                b = p;
                while ( p != end && *p != quote )
                {
                    if ( *p == '\\' || *p == '\n' ) return false;
                    ++p;
                }
                if ( p == end ) return false;
                e = p++;
                return true;
            }
            bool word( char const* w ) noexcept
            {
                std::size_t const n = std::strlen( w );
                if ( static_cast< std::size_t >( end - p ) < n || std::memcmp( p, w, n ) != 0 ) return false;
                char const* const after = p + n;
                if ( after != end && ( std::isalnum( static_cast< unsigned char >( *after ) ) || *after == '_' ) ) return false;
                p = after;
                return true;
            }
        };

        // S5-R1 (F07, D-021): parses a complete NPY v1.0/2.0/3.0 file image of `size` bytes into `out`, a matrix< T, A >.
        // Validates the magic, version, header length, the header dict, the dtype (exact match to T), the order and
        // the shape before any allocation; the element count times sizeof(T) must equal the payload length exactly.
        // Builds the result in a temporary with out's allocator and moves it into `out` only on success; on failure
        // returns false, leaves `out` unchanged and, when `why` is given, stores a static reason string there.
        template < typename T, typename Out >
        bool parse_npy( std::uint8_t const* bytes, std::size_t size, Out& out, char const** why = nullptr ) noexcept
        {
            static_assert( std::is_same_v< typename Out::value_type, T >, "parse_npy: Out must be a matrix of T" );
            auto fail = [why]( char const* reason ) noexcept
            {
                if ( why ) *why = reason;
                return false;
            };
            constexpr char kind = npy_dtype< T >::kind;
            if constexpr ( kind == '\0' )
            {
                (void)bytes; (void)size; (void)out;
                return fail( "the element type has no NPY dtype" );
            }
            else
            {
                static_assert( std::is_trivially_copyable_v< T > );
                if ( bytes == nullptr || size < 10 ) return fail( "file too short for the NPY preamble" );
                if ( std::memcmp( bytes, "\x93NUMPY", 6 ) != 0 ) return fail( "bad NPY magic" );
                std::uint8_t const major = bytes[6];
                std::uint8_t const minor = bytes[7];
                if ( ( major != 1 && major != 2 && major != 3 ) || minor != 0 ) return fail( "unsupported NPY version" );
                std::size_t header_begin = 10;
                std::size_t header_length = static_cast< std::size_t >( bytes[8] ) | ( static_cast< std::size_t >( bytes[9] ) << 8 );
                if ( major != 1 )
                {
                    if ( size < 12 ) return fail( "file too short for the NPY header length" );
                    header_begin = 12;
                    header_length |= ( static_cast< std::size_t >( bytes[10] ) << 16 ) | ( static_cast< std::size_t >( bytes[11] ) << 24 );
                }
                std::size_t data_offset = 0;
                if ( !checked_add( header_begin, header_length, data_offset ) || data_offset > size )
                    return fail( "NPY header extends past the end of the file" );

                // The header is a Python dict literal with exactly the keys descr, fortran_order and shape.
                npy_header_cursor cur{ reinterpret_cast< char const* >( bytes ) + header_begin, reinterpret_cast< char const* >( bytes ) + data_offset };
                bool have_descr = false, have_order = false, have_shape = false;
                bool fortran = false, foreign = false;
                std::size_t dims[2] = { 0, 0 };
                std::size_t rank = 0;
                cur.skip_ws();
                if ( !cur.eat( '{' ) ) return fail( "NPY header is not a dict literal" );
                cur.skip_ws();
                while ( !cur.eat( '}' ) )
                {
                    char const* kb = nullptr;
                    char const* ke = nullptr;
                    if ( !cur.string_literal( kb, ke ) ) return fail( "NPY header key is not a string" );
                    std::string_view const key{ kb, static_cast< std::size_t >( ke - kb ) };
                    cur.skip_ws();
                    if ( !cur.eat( ':' ) ) return fail( "NPY header is missing ':'" );
                    cur.skip_ws();
                    if ( key == "descr" )
                    {
                        if ( have_descr ) return fail( "NPY header repeats 'descr'" );
                        have_descr = true;
                        if ( cur.peek() == '[' ) return fail( "structured NPY dtype is not supported" );
                        char const* vb = nullptr;
                        char const* ve = nullptr;
                        if ( !cur.string_literal( vb, ve ) ) return fail( "NPY 'descr' is not a plain string" );
                        if ( ve - vb < 3 ) return fail( "unsupported NPY dtype" );
                        char const order = vb[0];
                        if ( order != '<' && order != '>' && order != '|' && order != '=' ) return fail( "bad NPY byte order" );
                        if ( vb[1] != kind ) return fail( "NPY dtype kind does not match the element type" );
                        if ( vb[2] < '0' || vb[2] > '9' ) return fail( "bad NPY dtype size" );
                        if ( vb[2] == '0' ) return fail( "bad NPY dtype size" ); // no zero size, no leading zeros ('<f08')
                        std::size_t item = 0;
                        auto const [ptr, ec] = std::from_chars( vb + 2, ve, item );
                        if ( ec != std::errc{} || ptr != ve ) return fail( "bad NPY dtype size" );
                        if ( item != sizeof( T ) ) return fail( "NPY dtype size does not match the element type" );
                        if ( order == '|' && sizeof( T ) != 1 ) return fail( "NPY byte order '|' on a multi-byte dtype" );
                        if constexpr ( sizeof( T ) > 1 )
                        {
                            if ( order == '<' ) foreign = std::endian::native != std::endian::little;
                            if ( order == '>' ) foreign = std::endian::native != std::endian::big;
                        }
                    }
                    else if ( key == "fortran_order" )
                    {
                        if ( have_order ) return fail( "NPY header repeats 'fortran_order'" );
                        have_order = true;
                        if ( cur.word( "True" ) ) fortran = true;
                        else if ( cur.word( "False" ) ) fortran = false;
                        else return fail( "NPY 'fortran_order' is not True or False" );
                    }
                    else if ( key == "shape" )
                    {
                        if ( have_shape ) return fail( "NPY header repeats 'shape'" );
                        have_shape = true;
                        if ( !cur.eat( '(' ) ) return fail( "NPY 'shape' is not a tuple" );
                        cur.skip_ws();
                        bool comma = false;
                        while ( !cur.eat( ')' ) )
                        {
                            char const c0 = cur.peek();
                            if ( c0 < '0' || c0 > '9' ) return fail( "NPY dimension is not a non-negative integer" );
                            std::size_t d = 0;
                            auto const [ptr, ec] = std::from_chars( cur.p, cur.end, d );
                            if ( ec != std::errc{} ) return fail( "NPY dimension is out of range" );
                            if ( c0 == '0' && ptr - cur.p > 1 ) return fail( "NPY dimension has a leading zero" ); // '(01, 2)'
                            cur.p = ptr;
                            if ( rank == 2 ) return fail( "NPY rank above 2 is not supported" );
                            dims[rank++] = d;
                            cur.skip_ws();
                            comma = cur.eat( ',' );
                            cur.skip_ws();
                            if ( !comma && cur.peek() != ')' ) return fail( "malformed NPY 'shape'" );
                        }
                        if ( rank == 0 ) return fail( "NPY rank 0 is not supported" );
                        if ( rank == 1 && !comma ) return fail( "malformed NPY 'shape'" );
                    }
                    else
                        return fail( "unknown key in the NPY header" );
                    cur.skip_ws();
                    if ( cur.eat( ',' ) )
                    {
                        cur.skip_ws();
                        continue;
                    }
                    if ( cur.peek() != '}' ) return fail( "malformed NPY header dict" );
                }
                cur.skip_ws();
                if ( !cur.at_end() ) return fail( "trailing characters after the NPY header dict" );
                if ( !have_descr || !have_order || !have_shape ) return fail( "NPY header is missing a key" );

                std::size_t const r = rank == 1 ? 1 : dims[0];
                std::size_t const c = rank == 1 ? dims[0] : dims[1];
                std::size_t count = 0;
                std::size_t payload = 0;
                if ( !checked_mul( r, c, count ) || !checked_mul( count, sizeof( T ), payload ) )
                    return fail( "NPY shape is too large" );
                if ( payload != size - data_offset ) return fail( "NPY payload length does not match the shape" );
                using size_type = typename Out::size_type;
                if ( r > std::numeric_limits< size_type >::max() || c > std::numeric_limits< size_type >::max() )
                    return fail( "NPY shape is too large" );

                Out tmp{ out.get_allocator(), static_cast< size_type >( r ), static_cast< size_type >( c ) };
                std::uint8_t const* const src = bytes + data_offset;
                T* const dst = tmp.data();
                constexpr std::size_t unit = npy_dtype< T >::component;
                for ( std::size_t k = 0; k != count; ++k )
                {
                    // Row-major element k is (i, j); a Fortran payload stores it at j*r + i.
                    std::size_t const from = fortran ? ( k % c ) * r + ( k / c ) : k;
                    std::uint8_t element[sizeof( T )];
                    std::memcpy( element, src + from * sizeof( T ), sizeof( T ) );
                    if ( foreign )
                        for ( std::size_t u = 0; u != sizeof( T ); u += unit )
                            std::reverse( element + u, element + u + unit );
                    std::memcpy( dst + k, element, sizeof( T ) );
                }
                out = std::move( tmp );
                return true;
            }
        }

        // S5-R2: the element types the native binary format (save_as_binary / load_binary) supports.
        template < typename T >
        inline constexpr bool is_binary_element_v = std::is_arithmetic_v< T > || std::is_same_v< T, std::complex< float > >
                                                 || std::is_same_v< T, std::complex< double > > || std::is_same_v< T, std::complex< long double > >;

        template < typename T >
        struct text_complex : std::false_type {};
        template < typename F >
        struct text_complex< std::complex< F > > : std::true_type { using component = F; };

        // S5-R2: the text token separators; '\n' ends a row.
        constexpr bool is_text_separator( char c ) noexcept
        {
            return c == ',' || c == ';' || c == ' ' || c == '\t' || c == '\r';
        }

        // S5-R2: calls on_token( b, e ) for each token of the line [b, e); a token starting with '(' runs to the
        // next ')' so the ',' of a complex pair is not a separator. False on an unclosed '(' or when on_token fails.
        template < typename F >
        bool for_each_text_token( char const* b, char const* e, F&& on_token ) noexcept
        {
            for ( ;; )
            {
                while ( b != e && is_text_separator( *b ) ) ++b;
                if ( b == e ) return true;
                char const* const t = b;
                if ( *b == '(' )
                {
                    while ( b != e && *b != ')' ) ++b;
                    if ( b == e ) return false;
                    ++b;
                }
                while ( b != e && !is_text_separator( *b ) ) ++b;
                if ( !on_token( t, b ) ) return false;
            }
        }

        enum class text_value_status { ok, syntax, range };

        // S5-R2: one real number or integer in [b, e), parsed completely by std::from_chars (one leading '+' allowed).
        template < typename T >
        text_value_status parse_text_real( char const* b, char const* e, T& v ) noexcept
        {
            if ( b != e && *b == '+' )
            {
                ++b;
                if ( b != e && ( *b == '+' || *b == '-' ) ) return text_value_status::syntax;
            }
            if ( b == e ) return text_value_status::syntax;
            std::from_chars_result res{};
            if constexpr ( std::is_integral_v< T > )
                res = std::from_chars( b, e, v, 10 );
            else
                res = std::from_chars( b, e, v, std::chars_format::general );
            if ( res.ec == std::errc::result_out_of_range ) return text_value_status::range;
            if ( res.ec != std::errc{} || res.ptr != e ) return text_value_status::syntax;
            return text_value_status::ok;
        }

        template < typename T >
        concept text_real = ( std::is_integral_v< T > && !std::is_same_v< T, bool > )
                         || ( std::is_floating_point_v< T > && requires( char const* p, T& x ) { std::from_chars( p, p, x, std::chars_format::general ); } );

        template < typename T >
        concept text_element = text_real< T > || ( text_complex< T >::value && text_real< typename text_complex< T >::component > );

        // S5-R2: one element token: a real or integer of type T, or for std::complex a bare real or "(re,im)".
        template < typename T >
        text_value_status parse_text_value( char const* b, char const* e, T& v ) noexcept
        {
            if constexpr ( text_complex< T >::value )
            {
                using F = typename text_complex< T >::component;
                F re{}, im{};
                if ( b == e || *b != '(' )
                {
                    auto const s = parse_text_real( b, e, re );
                    if ( s == text_value_status::ok ) v = T{ re, F{} };
                    return s;
                }
                if ( e - b < 2 || *( e - 1 ) != ')' ) return text_value_status::syntax;
                char const* const inner_b = b + 1;
                char const* const inner_e = e - 1;
                char const* const comma = std::find( inner_b, inner_e, ',' );
                if ( comma == inner_e ) return text_value_status::syntax;
                auto trim = []( char const*& x, char const*& y ) noexcept
                {
                    while ( x != y && ( *x == ' ' || *x == '\t' ) ) ++x;
                    while ( y != x && ( *( y - 1 ) == ' ' || *( y - 1 ) == '\t' ) ) --y;
                };
                char const* rb = inner_b; char const* re_ = comma;
                char const* ib = comma + 1; char const* ie = inner_e;
                trim( rb, re_ );
                trim( ib, ie );
                auto const s1 = parse_text_real( rb, re_, re );
                if ( s1 != text_value_status::ok ) return s1;
                auto const s2 = parse_text_real( ib, ie, im );
                if ( s2 != text_value_status::ok ) return s2;
                v = T{ re, im };
                return text_value_status::ok;
            }
            else
                return parse_text_real( b, e, v );
        }

        // S5-R2 (F08): parses text of `size` chars into `out`, a matrix< T, A >: each line holding a token is one row,
        // all rows hold the same token count >= 1, every token parses completely as T. A first pass validates the
        // structure, so the temporary (built with out's allocator) holds at most one element per input char; `out`
        // is assigned only on success. On failure returns false, leaves `out` unchanged and sets *why when given.
        template < typename T, typename Out >
        bool parse_text( char const* chars, std::size_t size, Out& out, char const** why = nullptr ) noexcept
        {
            static_assert( std::is_same_v< typename Out::value_type, T >, "parse_text: Out must be a matrix of T" );
            auto fail = [why]( char const* reason ) noexcept
            {
                if ( why ) *why = reason;
                return false;
            };
            if constexpr ( !text_element< T > )
            {
                (void)chars; (void)size; (void)out;
                return fail( "the element type cannot be read from text" );
            }
            else
            {
                if ( chars == nullptr || size == 0 ) return fail( "empty text" );
                char const* const end = chars + size;
                std::size_t rows = 0, cols = 0;
                for ( char const* line = chars; line != end; )
                {
                    char const* const eol = std::find( line, end, '\n' );
                    std::size_t n = 0;
                    if ( !for_each_text_token( line, eol, [&n]( char const*, char const* ) noexcept { ++n; return true; } ) )
                        return fail( "unclosed '(' in a row" );
                    if ( n != 0 )
                    {
                        if ( rows == 0 ) cols = n;
                        else if ( n != cols ) return fail( "rows hold different numbers of values" );
                        ++rows;
                    }
                    line = eol == end ? end : eol + 1;
                }
                if ( rows == 0 ) return fail( "no values in the text" );
                using size_type = typename Out::size_type;
                std::size_t count = 0;
                if ( !checked_mul( rows, cols, count ) || rows > std::numeric_limits< size_type >::max() || cols > std::numeric_limits< size_type >::max() )
                    return fail( "text matrix is too large" );

                Out tmp{ out.get_allocator(), static_cast< size_type >( rows ), static_cast< size_type >( cols ) };
                T* dst = tmp.data();
                text_value_status status = text_value_status::ok;
                for ( char const* line = chars; line != end; )
                {
                    char const* const eol = std::find( line, end, '\n' );
                    bool const ok = for_each_text_token( line, eol, [&dst, &status]( char const* b, char const* e ) noexcept
                    {
                        status = parse_text_value( b, e, *dst );
                        ++dst;
                        return status == text_value_status::ok;
                    } );
                    if ( !ok )
                        return fail( status == text_value_status::range ? "a value is out of range for the element type"
                                                                        : "a token does not parse completely as the element type" );
                    line = eol == end ? end : eol + 1;
                }
                out = std::move( tmp );
                return true;
            }
        }

        // S5-R2: writes m as text that parse_text reads back: tab-separated rows, one-byte integers as numbers,
        // floating-point values with max_digits10 significant digits.
        template < typename M >
        void write_text( std::ostream& os, M const& m ) noexcept
        {
            using T = typename M::value_type;
            if constexpr ( std::is_floating_point_v< T > )
                os.precision( std::numeric_limits< T >::max_digits10 );
            else if constexpr ( text_complex< T >::value )
                os.precision( std::numeric_limits< typename text_complex< T >::component >::max_digits10 );
            else
                os.precision( 18 );
            for ( typename M::size_type r = 0; r != m.row(); ++r )
            {
                for ( auto it = m.row_begin( r ); it != m.row_end( r ); ++it )
                {
                    if constexpr ( std::is_integral_v< T > && sizeof( T ) == 1 )
                        os << static_cast< int >( *it ) << '\t';
                    else
                        os << *it << '\t';
                }
                os << '\n';
            }
        }

        // S5-R2 (F08): parses a native binary image (two size_type counts, then the raw elements) into `out`, a
        // matrix< T, A >. The count product and byte size are overflow-checked and the payload length must equal
        // the declared size exactly; `out` is assigned only on success, otherwise *why is set when given.
        template < typename T, typename Out >
        bool parse_binary( std::uint8_t const* bytes, std::size_t size, Out& out, char const** why = nullptr ) noexcept
        {
            static_assert( std::is_same_v< typename Out::value_type, T >, "parse_binary: Out must be a matrix of T" );
            static_assert( is_binary_element_v< T >, "the native binary format supports arithmetic and std::complex<float|double|long double> elements only" );
            auto fail = [why]( char const* reason ) noexcept
            {
                if ( why ) *why = reason;
                return false;
            };
            using size_type = typename Out::size_type;
            constexpr std::size_t head = 2 * sizeof( size_type );
            if ( bytes == nullptr || size < head ) return fail( "file shorter than the two counts" );
            size_type r = 0, c = 0;
            std::memcpy( &r, bytes, sizeof( r ) );
            std::memcpy( &c, bytes + sizeof( r ), sizeof( c ) );
            size_type count = 0, payload = 0;
            if ( !checked_mul( r, c, count ) || !checked_mul( count, static_cast< size_type >( sizeof( T ) ), payload ) )
                return fail( "the element count or byte size overflows" );
            if ( payload != size - head ) return fail( "payload length does not match the counts" );
            Out tmp{ out.get_allocator(), r, c };
            if ( payload != 0 ) std::memcpy( static_cast< void* >( tmp.data() ), bytes + head, static_cast< std::size_t >( payload ) );
            out = std::move( tmp );
            return true;
        }

        // S6-R5 (D-023): a scalar operand is an arithmetic type or a std::complex specialization.
        template< typename S >
        concept matrix_scalar = std::is_arithmetic_v< S > || matrix_private::is_std_complex_v< S >;

        // The scalar as an operation on R elements sees it: R itself, or R's value type for complex R and a real
        // scalar, converted as by static_cast.
        template< typename R, typename S >
        constexpr auto scalar_as( S const& s ) noexcept
        {
            if constexpr ( std::same_as< R, S > )
                return s;
            else if constexpr ( matrix_private::is_std_complex_v< R > && !matrix_private::is_std_complex_v< S > )
                return static_cast< typename R::value_type >( s );
            else
                return static_cast< R >( s );
        }

        // A scalar of another type that a compound operator on T elements converts first (D-023: T is kept).
        template< typename S, typename T >
        concept compound_scalar = matrix_scalar< S > && matrix_scalar< T > && !std::same_as< S, T >;

        // A scalar or matrix element of type S converts to T: not complex into real.
        template< typename S, typename T >
        inline constexpr bool converts_to_element_v = !( matrix_private::is_std_complex_v< S > && !matrix_private::is_std_complex_v< T > );

    }//namespace matrix_details

    // S4-R1 (F05): column, diagonal and anti-diagonal iteration. Holds a base pointer, a stride, a logical index
    // and a count; a pointer base_ + i*stride_ is formed only in operator*, operator[] and operator->, for a
    // dereferenceable position. Arithmetic, equality, ordering and distance work on the index alone, so one past
    // the end is index == count and a stride of 0 is legal. Under FENG_MATRIX_CHECKED_ITERATORS (D-020) the
    // iterator also keeps its owner's [origin, origin+extent] and aborts on any position or address outside it.
    template < typename P >
    struct stride_iterator
    {
        typedef stride_iterator                                                     self_type;
        typedef typename std::iterator_traits< P >::value_type                      value_type;
        typedef typename std::iterator_traits< P >::reference                       reference;
        typedef typename std::iterator_traits< P >::difference_type                 difference_type;
        typedef P                                                                   pointer;
        typedef std::size_t                                                         size_type;
        typedef std::random_access_iterator_tag                                     iterator_category;
        typedef std::random_access_iterator_tag                                     iterator_concept;

    private:
        template < typename Q > friend struct stride_iterator;

        P                                                                           base_ = nullptr;
        difference_type                                                             stride_ = 1;
        difference_type                                                             index_ = 0;
        difference_type                                                             count_ = 0;
        #ifdef FENG_MATRIX_CHECKED_ITERATORS
        P                                                                           origin_ = nullptr;
        difference_type                                                             extent_ = 0;
        #endif

        void check_position() const noexcept
        {
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            FENG_MATRIX_EXPECTS( 0 <= index_ && index_ <= count_, "matrix iterator: position ", index_, " outside [0, ", count_, "]" );
            #endif
        }

        P address( difference_type i ) const noexcept
        {
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            FENG_MATRIX_EXPECTS( 0 <= i && i < count_, "matrix iterator: dereference at position ", i, " outside [0, ", count_, ")" );
            #endif
            return base_ + i * stride_;
        }

    public:
        stride_iterator() noexcept = default;

        // base: the element at index 0; count: the number of elements; index: the starting position in [0, count];
        // origin, extent: the owner's data() and size(), checked only under FENG_MATRIX_CHECKED_ITERATORS.
        stride_iterator( P base, difference_type stride, difference_type count, difference_type index, [[maybe_unused]] P origin, [[maybe_unused]] size_type extent ) noexcept
            : base_( base ), stride_( stride ), index_( index ), count_( count )
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            , origin_( origin ), extent_( static_cast< difference_type >( extent ) )
            #endif
        {
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            FENG_MATRIX_EXPECTS( 0 <= count_, "matrix iterator: negative count ", count_ );
            FENG_MATRIX_EXPECTS( extent <= static_cast< size_type >( PTRDIFF_MAX ), "matrix iterator: owner extent ", extent, " above PTRDIFF_MAX" );
            // integer offsets from the owner's origin, computed before any other pointer is formed
            std::uintptr_t const b = reinterpret_cast< std::uintptr_t >( base );
            std::uintptr_t const o = reinterpret_cast< std::uintptr_t >( origin );
            std::uintptr_t const bytes = b - o;
            std::uintptr_t const span = static_cast< std::uintptr_t >( extent_ ) * sizeof( value_type );
            FENG_MATRIX_EXPECTS( b >= o && bytes <= span && bytes % sizeof( value_type ) == 0, "matrix iterator: first address outside the owner's [data(), data()+size()]" );
            difference_type const first = static_cast< difference_type >( bytes / sizeof( value_type ) );
            if ( count_ > 0 )
            {
                FENG_MATRIX_EXPECTS( first < extent_, "matrix iterator: first element at offset ", first, " outside an owner of size ", extent_ );
                FENG_MATRIX_EXPECTS( stride_ == 0 || ( count_ - 1 ) <= ( stride_ > 0 ? ( extent_ - 1 - first ) / stride_ : first / -stride_ ),
                                     "matrix iterator: last element of ", count_, " with stride ", stride_, " from offset ", first, " outside an owner of size ", extent_ );
            }
            #endif
            check_position();
        }

        stride_iterator( const self_type& ) noexcept = default;
        stride_iterator( self_type&& ) noexcept = default;
        self_type& operator=( const self_type& ) noexcept = default;
        self_type& operator=( self_type&& ) noexcept = default;

        // stride_iterator<T*> converts to stride_iterator<T const*>
        template < typename Q >
        requires ( !std::is_same_v< Q, P > && std::is_convertible_v< Q, P > )
        stride_iterator( stride_iterator< Q > const& other ) noexcept
            : base_( other.base_ ), stride_( other.stride_ ), index_( other.index_ ), count_( other.count_ )
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            , origin_( other.origin_ ), extent_( other.extent_ )
            #endif
        { }

        self_type& operator++() noexcept
        {
            ++index_;
            check_position();
            return *this;
        }
        self_type operator++( int ) noexcept
        {
            self_type ans( *this );
            operator++();
            return ans;
        }
        self_type& operator--() noexcept
        {
            --index_;
            check_position();
            return *this;
        }
        self_type operator--( int ) noexcept
        {
            self_type ans( *this );
            operator--();
            return ans;
        }
        self_type& operator+=( const difference_type dt ) noexcept
        {
            index_ += dt;
            check_position();
            return *this;
        }
        self_type& operator-=( const difference_type dt ) noexcept
        {
            index_ -= dt;
            check_position();
            return *this;
        }
        friend self_type operator+( const self_type& lhs, const difference_type rhs ) noexcept
        {
            self_type ans( lhs );
            ans += rhs;
            return ans;
        }
        friend self_type operator+( const difference_type lhs, const self_type& rhs ) noexcept
        {
            return rhs + lhs;
        }
        friend self_type operator-( const self_type& lhs, const difference_type rhs ) noexcept
        {
            self_type ans( lhs );
            ans -= rhs;
            return ans;
        }
        friend difference_type operator-( const self_type& lhs, const self_type& rhs ) noexcept
        {
            return lhs.index_ - rhs.index_;
        }

        reference operator*() const noexcept
        {
            return *address( index_ );
        }
        reference operator[]( const difference_type dt ) const noexcept
        {
            return *address( index_ + dt );
        }
        pointer operator->() const noexcept
        {
            return address( index_ );
        }

        friend bool operator==( const self_type& lhs, const self_type& rhs ) noexcept
        {
            return lhs.index_ == rhs.index_;
        }
        friend std::strong_ordering operator<=>( const self_type& lhs, const self_type& rhs ) noexcept
        {
            return lhs.index_ <=> rhs.index_;
        }
    };

    // S4-R2 (F06, F05): row-major iteration over a rank-2 view. Holds the first viewed element, the parent's row
    // stride, the view's column count, a logical index and a count; the address
    // base_ + (i/cols_)*row_stride_ + i%cols_ is formed only in operator*, operator[] and operator->, for a
    // dereferenceable position. Arithmetic, equality, ordering and distance work on the index alone. Under
    // FENG_MATRIX_CHECKED_ITERATORS (D-020) it keeps its owner's [origin, origin+extent] and aborts on any position
    // or address outside it, as stride_iterator does.
    template < typename P >
    struct view_iterator
    {
        typedef view_iterator                                                       self_type;
        typedef typename std::iterator_traits< P >::value_type                      value_type;
        typedef typename std::iterator_traits< P >::reference                       reference;
        typedef typename std::iterator_traits< P >::difference_type                 difference_type;
        typedef P                                                                   pointer;
        typedef std::size_t                                                         size_type;
        typedef std::random_access_iterator_tag                                     iterator_category;
        typedef std::random_access_iterator_tag                                     iterator_concept;

    private:
        template < typename Q > friend struct view_iterator;

        P                                                                           base_ = nullptr;
        difference_type                                                             row_stride_ = 0;
        difference_type                                                             cols_ = 1;
        difference_type                                                             index_ = 0;
        difference_type                                                             count_ = 0;
        #ifdef FENG_MATRIX_CHECKED_ITERATORS
        P                                                                           origin_ = nullptr;
        difference_type                                                             extent_ = 0;
        #endif

        void check_position() const noexcept
        {
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            FENG_MATRIX_EXPECTS( 0 <= index_ && index_ <= count_, "matrix iterator: position ", index_, " outside [0, ", count_, "]" );
            #endif
        }

        P address( difference_type i ) const noexcept
        {
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            FENG_MATRIX_EXPECTS( 0 <= i && i < count_, "matrix iterator: dereference at position ", i, " outside [0, ", count_, ")" );
            #endif
            return base_ + ( ( i / cols_ ) * row_stride_ + i % cols_ );
        }

    public:
        view_iterator() noexcept = default;

        // base: the element at index 0; row_stride: the parent's columns; cols: the view's columns (> 0 when
        // count > 0); count: the number of elements; index: the starting position in [0, count]; origin, extent:
        // the owner's data() and size(), checked only under FENG_MATRIX_CHECKED_ITERATORS.
        view_iterator( P base, difference_type row_stride, difference_type cols, difference_type count, difference_type index, [[maybe_unused]] P origin, [[maybe_unused]] size_type extent ) noexcept
            : base_( base ), row_stride_( row_stride ), cols_( cols > 0 ? cols : 1 ), index_( index ), count_( count )
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            , origin_( origin ), extent_( static_cast< difference_type >( extent ) )
            #endif
        {
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            FENG_MATRIX_EXPECTS( 0 <= count_ && 0 <= row_stride_, "matrix iterator: negative count ", count_, " or row stride ", row_stride_ );
            FENG_MATRIX_EXPECTS( extent <= static_cast< size_type >( PTRDIFF_MAX ), "matrix iterator: owner extent ", extent, " above PTRDIFF_MAX" );
            // integer offsets from the owner's origin, computed before any other pointer is formed
            std::uintptr_t const b = reinterpret_cast< std::uintptr_t >( base );
            std::uintptr_t const o = reinterpret_cast< std::uintptr_t >( origin );
            std::uintptr_t const bytes = b - o;
            std::uintptr_t const span = static_cast< std::uintptr_t >( extent_ ) * sizeof( value_type );
            FENG_MATRIX_EXPECTS( b >= o && bytes <= span && bytes % sizeof( value_type ) == 0, "matrix iterator: first address outside the owner's [data(), data()+size()]" );
            difference_type const first = static_cast< difference_type >( bytes / sizeof( value_type ) );
            if ( count_ > 0 )
            {
                difference_type const last_row = ( count_ - 1 ) / cols_;
                difference_type const last_col = ( count_ - 1 ) % cols_;
                FENG_MATRIX_EXPECTS( first < extent_ && ( row_stride_ == 0 || last_row <= ( extent_ - 1 - first ) / row_stride_ ) &&
                                     last_row * row_stride_ + last_col <= extent_ - 1 - first,
                                     "matrix iterator: last element of ", count_, " in rows of ", cols_, " with row stride ", row_stride_, " from offset ", first, " outside an owner of size ", extent_ );
            }
            #endif
            check_position();
        }

        view_iterator( const self_type& ) noexcept = default;
        view_iterator( self_type&& ) noexcept = default;
        self_type& operator=( const self_type& ) noexcept = default;
        self_type& operator=( self_type&& ) noexcept = default;

        // view_iterator<T*> converts to view_iterator<T const*>
        template < typename Q >
        requires ( !std::is_same_v< Q, P > && std::is_convertible_v< Q, P > )
        view_iterator( view_iterator< Q > const& other ) noexcept
            : base_( other.base_ ), row_stride_( other.row_stride_ ), cols_( other.cols_ ), index_( other.index_ ), count_( other.count_ )
            #ifdef FENG_MATRIX_CHECKED_ITERATORS
            , origin_( other.origin_ ), extent_( other.extent_ )
            #endif
        { }

        self_type& operator++() noexcept { ++index_; check_position(); return *this; }
        self_type operator++( int ) noexcept { self_type ans( *this ); operator++(); return ans; }
        self_type& operator--() noexcept { --index_; check_position(); return *this; }
        self_type operator--( int ) noexcept { self_type ans( *this ); operator--(); return ans; }
        self_type& operator+=( const difference_type dt ) noexcept { index_ += dt; check_position(); return *this; }
        self_type& operator-=( const difference_type dt ) noexcept { index_ -= dt; check_position(); return *this; }
        friend self_type operator+( const self_type& lhs, const difference_type rhs ) noexcept { self_type ans( lhs ); ans += rhs; return ans; }
        friend self_type operator+( const difference_type lhs, const self_type& rhs ) noexcept { return rhs + lhs; }
        friend self_type operator-( const self_type& lhs, const difference_type rhs ) noexcept { self_type ans( lhs ); ans -= rhs; return ans; }
        friend difference_type operator-( const self_type& lhs, const self_type& rhs ) noexcept { return lhs.index_ - rhs.index_; }

        reference operator*() const noexcept { return *address( index_ ); }
        reference operator[]( const difference_type dt ) const noexcept { return *address( index_ + dt ); }
        pointer operator->() const noexcept { return address( index_ ); }

        friend bool operator==( const self_type& lhs, const self_type& rhs ) noexcept { return lhs.index_ == rhs.index_; }
        friend std::strong_ordering operator<=>( const self_type& lhs, const self_type& rhs ) noexcept { return lhs.index_ <=> rhs.index_; }
    };

    namespace matrix_private
    {
        // S4-R3 (F06): a view of an owner with `rows` rows and `cols` columns needs r0 <= r1 <= rows and
        // c0 <= c1 <= cols; an empty range is allowed, nothing is normalized.
        inline void check_view_range( std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1, std::size_t rows, std::size_t cols ) noexcept
        {
            FENG_MATRIX_EXPECTS( r0 <= r1 && r1 <= rows, "matrix view: row range [", r0, ", ", r1, ") outside an owner with ", rows, " rows" );
            FENG_MATRIX_EXPECTS( c0 <= c1 && c1 <= cols, "matrix view: column range [", c0, ", ", c1, ") outside an owner with ", cols, " columns" );
        }

        // S4-R3: a factory's range list must hold exactly two values.
        template < typename Integer_Type >
        std::pair< std::size_t, std::size_t > view_extent( std::initializer_list< Integer_Type > list, char const* what ) noexcept
        {
            FENG_MATRIX_EXPECTS( list.size() == 2, "matrix view: the ", what, " range needs exactly two values, got ", list.size() );
            return { static_cast< std::size_t >( *list.begin() ), static_cast< std::size_t >( *( list.begin() + 1 ) ) };
        }

        // S4-R1: a column index must name a column.
        inline void check_column( std::size_t c, std::size_t cols ) noexcept
        {
            FENG_MATRIX_EXPECTS( c < cols, "matrix iterator: column ", c, " outside a matrix with ", cols, " columns" );
        }

        // S4-R1: diagonal (and anti-diagonal) k exists for -rows < k < cols; diagonal 0 always exists (empty on
        // an empty matrix).
        inline void check_diagonal( std::ptrdiff_t k, std::size_t rows, std::size_t cols ) noexcept
        {
            bool const valid = k == 0 || ( k > 0 ? static_cast< std::size_t >( k ) < cols : static_cast< std::size_t >( -( k + 1 ) ) + 1 < rows );
            FENG_MATRIX_EXPECTS( valid, "matrix iterator: diagonal ", k, " outside a ", rows, "x", cols, " matrix" );
        }

        // the signed diagonal of an unsigned upper (k >= 0) or lower (k <= 0) index, saturated so that it stays invalid
        inline std::ptrdiff_t upper_diagonal( std::size_t index ) noexcept
        {
            return static_cast< std::ptrdiff_t >( std::min< std::size_t >( index, PTRDIFF_MAX ) );
        }
        inline std::ptrdiff_t lower_diagonal( std::size_t index ) noexcept
        {
            return -static_cast< std::ptrdiff_t >( std::min< std::size_t >( index, PTRDIFF_MAX ) );
        }

        // S4-R1: the [begin, end) pair of a strided range of `length` elements starting at integer offset `start`
        // of an owner [data, data+size); an empty range starts at offset 0, so no pointer past data+size is formed.
        template < typename It, typename Ptr >
        It stride_range( Ptr data, std::size_t size, std::size_t start, std::ptrdiff_t stride, std::size_t length, bool at_end ) noexcept
        {
            if ( length == 0 ) start = 0;
            std::ptrdiff_t const n = static_cast< std::ptrdiff_t >( length );
            return It( data + start, stride, n, at_end ? n : 0, data, size );
        }
    }

    template < typename Type, Allocator Alloc >
    struct crtp_typedef
    {
        typedef typename std::decay< Type >::type                       value_type;
        typedef value_type*                                             iterator;
        typedef const value_type*                                       const_iterator;
        typedef Alloc                                               allocator_type;
        typedef std::uint_least64_t                                     size_type;
        typedef std::ptrdiff_t                                          difference_type;
        typedef std::pair<size_type, size_type>                         range_type;
        typedef typename std::allocator_traits<Alloc>::pointer      pointer;
        typedef typename std::allocator_traits<Alloc>::const_pointer const_pointer;
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


    // interfacing opencv
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_opencv
    {
        typedef Matrix zen_type;
        typedef Type value_type;

        #ifdef FENG_MATRIX_OPENCV

        cv::Mat to_opencv( unsigned long channels = 1 ) const noexcept
        {
            better_assert( ((channels>=1) && (channels<=4)), "Expecting 1-4 channels." );

            auto const& zen = static_cast<zen_type const&>(*this);

            unsigned long rows = static_cast<unsigned long>( zen.col() / channels );
            unsigned long cols = zen.row();
            better_assert( rows * channels == zen.col(), "Error: cannot cast this matrix to opencv matrix with the selected channels, cannot divide it." );

            cv::Mat ans;

            if constexpr (std::is_same_v<value_type, std::uint8_t>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_8UC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_8UC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_8UC3 ):
                                      cv::Mat( rows, cols, CV_8UC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<std::uint8_t*>( ans.data ) );
            }
            else if constexpr (std::is_same_v<value_type, std::int8_t>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_8SC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_8SC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_8SC3 ):
                                      cv::Mat( rows, cols, CV_8SC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<std::int8_t*>( ans.data ) );
            }
            else if constexpr (std::is_same_v<value_type, std::uint16_t>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_16UC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_16UC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_16UC3 ):
                                      cv::Mat( rows, cols, CV_16UC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<std::uint16_t*>( ans.data ) );
            }
            else if constexpr (std::is_same_v<value_type, std::int16_t>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_16SC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_16SC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_16SC3 ):
                                      cv::Mat( rows, cols, CV_16SC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<std::int16_t*>( ans.data ) );
            }
            else if constexpr (std::is_same_v<value_type, std::int32_t>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_32SC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_32SC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_32SC3 ):
                                      cv::Mat( rows, cols, CV_32SC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<std::int32_t*>( ans.data ) );
            }
            else if constexpr (std::is_same_v<value_type, float>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_32FC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_32FC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_32FC3 ):
                                      cv::Mat( rows, cols, CV_32FC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<float*>( ans.data ) );
            }
            else if constexpr (std::is_same_v<value_type, double>)
            {
                ans = channels == 1 ? cv::Mat( rows, cols, CV_64FC1 ):
                      channels == 2 ? cv::Mat( rows, cols, CV_64FC2 ):
                      channels == 3 ? cv::Mat( rows, cols, CV_64FC3 ):
                                      cv::Mat( rows, cols, CV_64FC4 );
                std::copy( zen.begin(), zen.end(), reinterpret_cast<double*>( ans.data ) );
            }
            else
            {
                better_assert( false, "Cannot convert this type of matrix to a OpenCV matrix." );
            }

            return ans;
        }

        auto& from_opencv( cv::Mat image ) noexcept
        {
            if ( !image.isContiguous() ) // data stored in Mat is not always continuous in memory
                image = image.clone();

            unsigned long const rows = image.rows;
            unsigned long const cols = image.cols;
            unsigned long const channels = 1 + (image.type() >> CV_CN_SHIFT);
            unsigned char const depth = image.type() & CV_MAT_DEPTH_MASK;
            auto* img_data = image.data;

            if ( img_data == nullptr )
                return *this;

            auto& zen = static_cast<zen_type&>(*this);
            zen.resize( cols, rows * channels ); // different orders

            using matrix_details::for_each;
            switch (depth)
            {
                case CV_8U:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<std::uint8_t const*>(img_data), []( auto& v, std::uint8_t x ){ v = x; } );
                    break;
                case CV_8S:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<std::int8_t const*>(img_data), []( auto& v, std::int8_t x ){ v = x; } );
                    break;
                case CV_16U:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<std::uint16_t const*>(img_data), []( auto& v, std::uint16_t x ){ v = x; } );
                    break;
                case CV_16S:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<std::int16_t const*>(img_data), []( auto& v, std::int16_t x ){ v = x; } );
                    break;
                case CV_32S:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<std::int32_t const*>(img_data), []( auto& v, std::int32_t x ){ v = x; } );
                    break;
                case CV_32F:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<float const*>(img_data), []( auto& v, float x ){ v = x; } );
                    break;
                case CV_64F:
                    for_each( zen.begin(), zen.end(), reinterpret_cast<double const*>(img_data), []( auto& v, double x ){ v = x; } );
                    break;
                default:
                    better_assert( false, "Unknown image depth. Exit." );
            }

            return *this;
        }
        #endif

    }; // struct crtp_opencv




    template < typename Matrix, typename Type, Allocator Alloc >
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

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_shape
    {
        typedef Matrix zen_type;
        auto shape() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            return std::make_tuple( zen.row(), zen.col() );
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    using crtp_shape_view = crtp_shape<Matrix, Type, Alloc>;

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_get_allocator
    {
        typedef Matrix zen_type;
        auto get_allocator() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            return matrix_private::storage_access::storage( zen ).get_allocator();
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_anti_diag_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::difference_type difference_type;
        typedef typename type_proxy_type::anti_diag_type anti_diag_type;
        typedef typename type_proxy_type::const_anti_diag_type const_anti_diag_type;
        typedef typename type_proxy_type::reverse_anti_diag_type reverse_anti_diag_type;
        typedef typename type_proxy_type::const_reverse_anti_diag_type const_reverse_anti_diag_type;
        anti_diag_type upper_anti_diag_begin( const size_type index = 0 ) noexcept
        {
            return upper_anti_diag_range< anti_diag_type >( static_cast< zen_type& >( *this ), index, false );
        }
        anti_diag_type upper_anti_diag_end( const size_type index = 0 ) noexcept
        {
            return upper_anti_diag_range< anti_diag_type >( static_cast< zen_type& >( *this ), index, true );
        }
        const_anti_diag_type upper_anti_diag_begin( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_range< const_anti_diag_type >( static_cast< zen_type const& >( *this ), index, false );
        }
        const_anti_diag_type upper_anti_diag_end( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_range< const_anti_diag_type >( static_cast< zen_type const& >( *this ), index, true );
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
            return lower_anti_diag_range< anti_diag_type >( static_cast< zen_type& >( *this ), index, false );
        }
        anti_diag_type lower_anti_diag_end( const size_type index = 0 ) noexcept
        {
            return lower_anti_diag_range< anti_diag_type >( static_cast< zen_type& >( *this ), index, true );
        }
        const_anti_diag_type lower_anti_diag_begin( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_range< const_anti_diag_type >( static_cast< zen_type const& >( *this ), index, false );
        }
        const_anti_diag_type lower_anti_diag_end( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_range< const_anti_diag_type >( static_cast< zen_type const& >( *this ), index, true );
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
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_begin( static_cast< size_type >( index ) );
            return lower_anti_diag_begin( static_cast< size_type >( -index ) );
        }
        anti_diag_type anti_diag_end( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_end( static_cast< size_type >( index ) );
            return lower_anti_diag_end( static_cast< size_type >( -index ) );
        }
        const_anti_diag_type anti_diag_begin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_begin( static_cast< size_type >( index ) );
            return lower_anti_diag_begin( static_cast< size_type >( -index ) );
        }
        const_anti_diag_type anti_diag_end( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_end( static_cast< size_type >( index ) );
            return lower_anti_diag_end( static_cast< size_type >( -index ) );
        }
        const_anti_diag_type anti_diag_cbegin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_cbegin( static_cast< size_type >( index ) );
            return lower_anti_diag_cbegin( static_cast< size_type >( -index ) );
        }
        const_anti_diag_type anti_diag_cend( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_cend( static_cast< size_type >( index ) );
            return lower_anti_diag_cend( static_cast< size_type >( -index ) );
        }
        reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_rbegin( static_cast< size_type >( index ) );
            return lower_anti_diag_rbegin( static_cast< size_type >( -index ) );
        }
        reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_rend( static_cast< size_type >( index ) );
            return lower_anti_diag_rend( static_cast< size_type >( -index ) );
        }
        const_reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_rbegin( static_cast< size_type >( index ) );
            return lower_anti_diag_rbegin( static_cast< size_type >( -index ) );
        }
        const_reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_rend( static_cast< size_type >( index ) );
            return lower_anti_diag_rend( static_cast< size_type >( -index ) );
        }
        const_reverse_anti_diag_type anti_diag_crbegin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_crbegin( static_cast< size_type >( index ) );
            return lower_anti_diag_crbegin( static_cast< size_type >( -index ) );
        }
        const_reverse_anti_diag_type anti_diag_crend( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_anti_diag_crend( static_cast< size_type >( index ) );
            return lower_anti_diag_crend( static_cast< size_type >( -index ) );
        }
    private:
        // S4-R1: upper anti-diagonal k starts at (0, col()-1-k), lower anti-diagonal k at (k, col()-1); each step
        // moves one row down and one column left (stride col()-1, 0 for one column).
        template < typename It, typename Z >
        static It upper_anti_diag_range( Z& zen, size_type index, bool at_end ) noexcept
        {
            size_type const rows = zen.row(), cols = zen.col();
            matrix_private::check_diagonal( matrix_private::upper_diagonal( index ), rows, cols );
            size_type const length = index < cols ? std::min( cols - index, rows ) : 0;
            size_type const start = length ? cols - 1 - index : 0;
            return matrix_private::stride_range< It >( std::to_address( zen.data() ), zen.size(), start, static_cast< difference_type >( cols ) - 1, length, at_end );
        }
        template < typename It, typename Z >
        static It lower_anti_diag_range( Z& zen, size_type index, bool at_end ) noexcept
        {
            size_type const rows = zen.row(), cols = zen.col();
            matrix_private::check_diagonal( matrix_private::lower_diagonal( index ), rows, cols );
            size_type const length = index < rows ? std::min( rows - index, cols ) : 0;
            size_type const start = length ? index * cols + cols - 1 : 0;
            return matrix_private::stride_range< It >( std::to_address( zen.data() ), zen.size(), start, static_cast< difference_type >( cols ) - 1, length, at_end );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
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
            matrix_details::parallel_work( parallel_function, 0UL, zen.size(), 1, matrix_details::callback_grain );
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
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_bracket_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        row_type operator[]( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            FENG_MATRIX_EXPECTS( index < zen.row() && "Row index out of boundary!", "matrix index: row ", index, " with row() ", zen.row() );
            return zen.row_begin( index );
        }
        const_row_type operator[]( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            FENG_MATRIX_EXPECTS( index < zen.row() && "Row index out of boundary!", "matrix index: row ", index, " with row() ", zen.row() );
            return zen.row_cbegin( index );
        }
        // S2-R2: at( r, c ) and m( r, c ) share matrix_private::check_index and abort with `index` out of range.
        value_type const& at( size_type r, size_type c ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_index( r, c, zen.row(), zen.col() );
            return zen.row_cbegin( r )[c];
        }
        value_type& at( size_type r, size_type c ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_index( r, c, zen.row(), zen.col() );
            return zen.row_begin( r )[c];
        }
        value_type operator()( size_type r, size_type c ) const noexcept
        {
            return (*this).at( r, c );
        }
        value_type& operator()( size_type r, size_type c ) noexcept
        {
            return (*this).at( r, c );
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_clear
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        void clear() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            auto& storage = matrix_private::storage_access::storage( zen );
            std::remove_reference_t< decltype( storage ) > empty{ storage.get_allocator() };
            storage.swap( empty ); // equal allocators: releases the old buffer
            matrix_private::storage_access::rows( zen ) = 0;
            matrix_private::storage_access::cols( zen ) = 0;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_clone
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        template < typename Other_Matrix >
        zen_type& clone( const Other_Matrix& other, std::initializer_list<size_type> r, std::initializer_list<size_type> c ) noexcept
        {
            matrix_private::check_clone_lists( r.size(), c.size() );
            return clone( other, *r.begin(), *(r.begin()+1), *c.begin(), *(c.begin()+1) );
        }
        template < typename Other_Matrix >
        zen_type& clone( const Other_Matrix& other, size_type const r0, size_type const r1, size_type const c0, size_type const c1 ) noexcept
        {
            matrix_private::check_clone_range( r0, r1, c0, c1, other.row(), other.col() );

            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type tmp{ zen.get_allocator(), r1-r0, c1-c0 };

            auto && parallel_function = [&]( std::uint_least64_t r )
            {
                std::copy_n( other.row_begin(r+r0)+c0, tmp.col(), tmp.row_begin(r) );
            };

            matrix_details::parallel_work( parallel_function, 0UL, tmp.row(), tmp.col(), matrix_details::elementwise_grain );

            zen.swap( tmp );
            return zen;
        }

        [[nodiscard]] zen_type clone( std::initializer_list<size_type> r, std::initializer_list<size_type> c ) const noexcept
        {
            matrix_private::check_clone_lists( r.size(), c.size() );
            auto const [r0, r1] = std::tuple{ *r.begin(), *(r.begin()+1) };
            auto const [c0, c1] = std::tuple{ *c.begin(), *(c.begin()+1) };

            return clone( r0, r1, c0, c1 );
        }
        [[nodiscard]] zen_type clone( size_type const r0, size_type const r1, size_type const c0, size_type const c1 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            zen_type ans{ zen.get_allocator() };
            ans.clone( zen, r0, r1, c0, c1 );
            return ans;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_col_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::difference_type difference_type;
        typedef typename type_proxy_type::col_type col_type;
        typedef typename type_proxy_type::const_col_type const_col_type;
        typedef typename type_proxy_type::reverse_col_type reverse_col_type;
        typedef typename type_proxy_type::const_reverse_col_type const_reverse_col_type;
        col_type col_begin( const size_type index ) noexcept
        {
            return col_range< col_type >( static_cast< zen_type& >( *this ), index, false );
        }
        col_type col_end( const size_type index ) noexcept
        {
            return col_range< col_type >( static_cast< zen_type& >( *this ), index, true );
        }
        const_col_type col_begin( const size_type index ) const noexcept
        {
            return col_range< const_col_type >( static_cast< zen_type const& >( *this ), index, false );
        }
        const_col_type col_end( const size_type index ) const noexcept
        {
            return col_range< const_col_type >( static_cast< zen_type const& >( *this ), index, true );
        }
        const_col_type col_cbegin( const size_type index ) const noexcept
        {
            return col_begin( index );
        }
        const_col_type col_cend( const size_type index ) const noexcept
        {
            return col_end( index );
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
            return col_rbegin( index );
        }
        const_reverse_col_type col_crend( const size_type index = 0 ) const noexcept
        {
            return col_rend( index );
        }
    private:
        // S4-R1: column c starts at offset c with stride col() and row() elements.
        template < typename It, typename Z >
        static It col_range( Z& zen, size_type index, bool at_end ) noexcept
        {
            matrix_private::check_column( index, zen.col() );
            return matrix_private::stride_range< It >( std::to_address( zen.data() ), zen.size(), index, static_cast< difference_type >( zen.col() ), zen.row(), at_end );
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_copy
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        template < typename Other_Matrix >
        void copy( const Other_Matrix& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            // S2-R4: copying a matrix onto itself is a no-op that keeps the contents.
            if ( static_cast< void const* >( std::addressof( rhs ) ) == static_cast< void const* >( std::addressof( zen ) ) ) return;
            // S3 (F04): the destination keeps its own allocator.
            // S4-R2 (B-009): a view of the destination is copied through a snapshot taken before resizing.
            if constexpr ( requires { rhs.row_stride(); } && std::is_constructible_v< zen_type, Other_Matrix const& > )
            {
                if ( rhs.size() != 0 )
                {
                    void const* const first = static_cast< void const* >( std::addressof( rhs.at( 0, 0 ) ) );
                    void const* const lo = static_cast< void const* >( zen.data() );
                    void const* const hi = static_cast< void const* >( zen.data() + zen.size() );
                    std::less<> const before{};
                    if ( !before( first, lo ) && before( first, hi ) )
                    {
                        zen_type const snapshot{ rhs };
                        zen.copy( snapshot );
                        return;
                    }
                }
            }
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

            if ( other.row() == 0 || other.col() == 0 ) return;

            // S2-R4: when the source's element range overlaps the destination block, copy a snapshot of the source.
            {
                auto const* const src_first = static_cast< void const* >( std::addressof( *other.row_begin( 0 ) ) );
                auto const* const src_last  = static_cast< void const* >( std::addressof( *( other.row_begin( other.row() - 1 ) + ( other.col() - 1 ) ) ) + 1 );
                auto const* const dst_first = static_cast< void const* >( std::addressof( *( zen.row_begin( r0 ) + c0 ) ) );
                auto const* const dst_last  = static_cast< void const* >( std::addressof( *( zen.row_begin( r1 - 1 ) + ( c1 - 1 ) ) ) + 1 );
                std::less<> const before{};
                if ( before( src_first, dst_last ) && before( dst_first, src_last ) )
                {
                    zen_type snapshot( other.row(), other.col() );
                    for ( size_type row_index = 0; row_index != other.row(); ++row_index )
                        std::copy( other.row_begin( row_index ), other.row_end( row_index ), snapshot.row_begin( row_index ) );
                    zen.copy( snapshot, { r0, r1 }, { c0, c1 } );
                    return;
                }
            }

            auto const& copy_function = [&, r0=r0, c0=c0]( size_type const row_index )
            {
                std::copy( other.row_begin(row_index), other.row_end(row_index), zen.row_begin(r0+row_index)+c0 );
            };
            matrix_details::parallel_work( copy_function, 0UL, other.row(), other.col(), matrix_details::elementwise_grain );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_data
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::pointer pointer;
        typedef typename type_proxy_type::const_pointer const_pointer;
        pointer data() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return matrix_private::storage_access::storage( zen ).data();
        }
        const_pointer data() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return matrix_private::storage_access::storage( zen ).data();
        }
    };

    // ---- linear algebra: status, result and the one LU kernel (S7-R1, S7-R2, D-026, D-027) ----
    // Declared ahead of the crtp members so det() and inverse() can use them.
    enum class linalg_status { ok, singular, not_positive_definite, not_converged, nonfinite };

    template < typename V >
    struct linalg_result
    {
        V value;
        linalg_status status;
        constexpr bool ok() const noexcept { return status == linalg_status::ok; }
        constexpr explicit operator bool() const noexcept { return ok(); }
    };

    // S9-R2 (D-034): the element types linear algebra accepts: floating-point or std::complex. An integral matrix
    // fails this constraint; convert it with astype<double>() first.
    template < typename T >
    concept linalg_element = std::floating_point< T > || matrix_private::is_std_complex_v< T >;

    namespace matrix_details
    {
        template < typename T >
        struct linalg_real { using type = T; };
        template < typename T >
        struct linalg_real< std::complex< T > > { using type = T; };
        template < typename T >
        using linalg_real_t = typename linalg_real< T >::type;

        template < typename T >
        bool linalg_isfinite( T const& x ) noexcept
        {
            if constexpr ( matrix_private::is_std_complex_v< T > )
                return std::isfinite( x.real() ) && std::isfinite( x.imag() );
            else
                return std::isfinite( x );
        }

        struct lu_info
        {
            linalg_status status;
            std::size_t rank;
            int sign; // sign of the row permutation
        };

        // In-place partial-pivoting LU of the row-major n×n array `a`: on return the strict lower triangle holds L
        // (unit diagonal implied) and the upper triangle U, with P·A = L·U. `piv` (size n) gets row i of P·A =
        // row piv[i] of A. The pivot of column k is the first entry of largest |·| in rows k..n-1; a column that
        // is zero there is left as is (no elimination). rank counts |u_kk| > n·ε·max|U| (D-027).
        template < typename T >
        lu_info lu_in_place( T* a, std::size_t n, std::size_t* piv ) noexcept
        {
            using R = linalg_real_t< T >;
            for ( std::size_t i = 0; i != n; ++i ) piv[i] = i;
            for ( std::size_t i = 0; i != n * n; ++i )
                if ( !linalg_isfinite( a[i] ) )
                    return lu_info{ linalg_status::nonfinite, 0, 1 };
            int sign = 1;
            for ( std::size_t k = 0; k != n; ++k )
            {
                std::size_t p = k;
                R best = std::abs( a[k * n + k] );
                for ( std::size_t i = k + 1; i != n; ++i )
                    if ( R const v = std::abs( a[i * n + k] ); v > best ) { best = v; p = i; }
                if ( p != k )
                {
                    std::swap_ranges( a + k * n, a + k * n + n, a + p * n );
                    std::swap( piv[k], piv[p] );
                    sign = -sign;
                }
                if ( best == R{ 0 } ) continue; // zero column: nothing to eliminate
                T const pivot = a[k * n + k];
                for ( std::size_t i = k + 1; i != n; ++i )
                {
                    T* const ri = a + i * n;
                    T const l = ri[k] / pivot;
                    ri[k] = l;
                    if ( l == T{ 0 } ) continue;
                    T const* const rk = a + k * n;
                    for ( std::size_t j = k + 1; j != n; ++j ) ri[j] -= l * rk[j];
                }
            }
            for ( std::size_t i = 0; i != n * n; ++i ) // overflow during elimination
                if ( !linalg_isfinite( a[i] ) ) return lu_info{ linalg_status::nonfinite, 0, sign };
            R max_u{ 0 };
            for ( std::size_t i = 0; i != n; ++i )
                for ( std::size_t j = i; j != n; ++j )
                    max_u = std::max( max_u, R( std::abs( a[i * n + j] ) ) );
            R const tol = static_cast< R >( n ) * std::numeric_limits< R >::epsilon() * max_u;
            std::size_t rank = 0;
            for ( std::size_t k = 0; k != n; ++k )
                if ( std::abs( a[k * n + k] ) > tol ) ++rank;
            return lu_info{ rank < n ? linalg_status::singular : linalg_status::ok, rank, sign };
        }

        // det = sign(P)·∏u_kk; exactly 0 when rank < n, 1 for n = 0, NaN for a nonfinite input or factor
        template < typename T >
        T lu_det( T const* lu, std::size_t n, lu_info const& info ) noexcept
        {
            if ( info.status == linalg_status::nonfinite ) return T( std::numeric_limits< linalg_real_t< T > >::quiet_NaN() );
            if ( info.status != linalg_status::ok ) return T{ 0 };
            T d = T( info.sign );
            for ( std::size_t k = 0; k != n; ++k ) d *= lu[k * n + k];
            return d;
        }

        // Solves A·X = B from the factors of lu_in_place: b is the row-major n×k right-hand side, x (n×k) gets X.
        template < typename T >
        void lu_solve( T const* lu, std::size_t n, std::size_t const* piv, T const* b, std::size_t k, T* x ) noexcept
        {
            for ( std::size_t i = 0; i != n; ++i )
                std::copy( b + piv[i] * k, b + piv[i] * k + k, x + i * k );
            for ( std::size_t i = 0; i != n; ++i ) // L·Y = P·B, unit diagonal
                for ( std::size_t j = 0; j != i; ++j )
                    if ( T const l = lu[i * n + j]; l != T{ 0 } )
                        for ( std::size_t c = 0; c != k; ++c ) x[i * k + c] -= l * x[j * k + c];
            for ( std::size_t r = n; r-- != 0; ) // U·X = Y
            {
                for ( std::size_t j = r + 1; j != n; ++j )
                    if ( T const u = lu[r * n + j]; u != T{ 0 } )
                        for ( std::size_t c = 0; c != k; ++c ) x[r * k + c] -= u * x[j * k + c];
                T const d = lu[r * n + r];
                for ( std::size_t c = 0; c != k; ++c ) x[r * k + c] /= d;
            }
        }

        template < typename T >
        bool all_finite( T const* x, std::size_t n ) noexcept
        {
            for ( std::size_t i = 0; i != n; ++i )
                if ( !linalg_isfinite( x[i] ) ) return false;
            return true;
        }

        // inverse of the square matrix `m` into `out` (resized to n×n only on success); returns the status
        template < typename M >
        linalg_status lu_inverse( M const& m, M& out ) noexcept
        {
            using T = typename M::value_type;
            std::size_t const n = m.row();
            M lu( m );
            std::vector< std::size_t > piv( n );
            lu_info const info = lu_in_place( lu.data(), n, piv.data() );
            if ( info.status != linalg_status::ok ) return info.status;
            M id( n, n );
            std::fill( id.begin(), id.end(), T{ 0 } );
            for ( std::size_t i = 0; i != n; ++i ) id[i][i] = T{ 1 };
            M x( n, n );
            lu_solve( lu.data(), n, piv.data(), id.data(), n, x.data() );
            if ( !all_finite( x.data(), n * n ) ) return linalg_status::nonfinite;
            out = std::move( x );
            return linalg_status::ok;
        }
    }//namespace matrix_details

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_det
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::range_type range_type;
        // S7-R2: sign(P)·∏u_kk from the one LU kernel; exactly 0 when rank < n, 1 for 0×0 (D-027)
        [[nodiscard]] value_type det() const noexcept requires linalg_element< value_type >
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row()==zen.col(), "matrix::det: expecting a square matrix, but got ", zen.row(), "x", zen.col() );
            size_type const n = zen.row();
            zen_type lu( zen );
            std::vector< std::size_t > piv( n );
            auto const info = matrix_details::lu_in_place( lu.data(), n, piv.data() );
            return matrix_details::lu_det( lu.data(), n, info );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_diag_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
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
            return upper_diag_range< diag_type >( static_cast< zen_type& >( *this ), index, false );
        }
        diag_type upper_diag_end( const size_type index ) noexcept
        {
            return upper_diag_range< diag_type >( static_cast< zen_type& >( *this ), index, true );
        }
        const_diag_type upper_diag_begin( const size_type index ) const noexcept
        {
            return upper_diag_range< const_diag_type >( static_cast< zen_type const& >( *this ), index, false );
        }
        const_diag_type upper_diag_end( const size_type index ) const noexcept
        {
            return upper_diag_range< const_diag_type >( static_cast< zen_type const& >( *this ), index, true );
        }
        const_diag_type upper_diag_cbegin( const size_type index ) const noexcept
        {
            return upper_diag_begin( index );
        }
        const_diag_type upper_diag_cend( const size_type index ) const noexcept
        {
            return upper_diag_end( index );
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
            return upper_diag_rbegin( index );
        }
        const_reverse_upper_diag_type upper_diag_crend( const size_type index = 0 ) const noexcept
        {
            return upper_diag_rend( index );
        }
        diag_type lower_diag_begin( const size_type index ) noexcept
        {
            return lower_diag_range< diag_type >( static_cast< zen_type& >( *this ), index, false );
        }
        diag_type lower_diag_end( const size_type index ) noexcept
        {
            return lower_diag_range< diag_type >( static_cast< zen_type& >( *this ), index, true );
        }
        const_diag_type lower_diag_begin( const size_type index ) const noexcept
        {
            return lower_diag_range< const_diag_type >( static_cast< zen_type const& >( *this ), index, false );
        }
        const_diag_type lower_diag_end( const size_type index ) const noexcept
        {
            return lower_diag_range< const_diag_type >( static_cast< zen_type const& >( *this ), index, true );
        }
        const_diag_type lower_diag_cbegin( const size_type index ) const noexcept
        {
            return lower_diag_begin( index );
        }
        const_diag_type lower_diag_cend( const size_type index ) const noexcept
        {
            return lower_diag_end( index );
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
            return lower_diag_rbegin( index );
        }
        const_reverse_lower_diag_type lower_diag_crend( const size_type index = 0 ) const noexcept
        {
            return lower_diag_rend( index );
        }
        diag_type diag_begin( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_begin( static_cast< size_type >( index ) );
            return lower_diag_begin( static_cast< size_type >( -index ) );
        }
        diag_type diag_end( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_end( static_cast< size_type >( index ) );
            return lower_diag_end( static_cast< size_type >( -index ) );
        }
        const_diag_type diag_begin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_begin( static_cast< size_type >( index ) );
            return lower_diag_begin( static_cast< size_type >( -index ) );
        }
        const_diag_type diag_end( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_end( static_cast< size_type >( index ) );
            return lower_diag_end( static_cast< size_type >( -index ) );
        }
        const_diag_type diag_cbegin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_cbegin( static_cast< size_type >( index ) );
            return lower_diag_cbegin( static_cast< size_type >( -index ) );
        }
        const_diag_type diag_cend( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_cend( static_cast< size_type >( index ) );
            return lower_diag_cend( static_cast< size_type >( -index ) );
        }
        reverse_diag_type diag_rbegin( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_rbegin( static_cast< size_type >( index ) );
            return lower_diag_rbegin( static_cast< size_type >( -index ) );
        }
        reverse_diag_type diag_rend( const difference_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_rend( static_cast< size_type >( index ) );
            return lower_diag_rend( static_cast< size_type >( -index ) );
        }
        const_reverse_diag_type diag_rbegin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_rbegin( static_cast< size_type >( index ) );
            return lower_diag_rbegin( static_cast< size_type >( -index ) );
        }
        const_reverse_diag_type diag_rend( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_rend( static_cast< size_type >( index ) );
            return lower_diag_rend( static_cast< size_type >( -index ) );
        }
        const_reverse_diag_type diag_crbegin( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_crbegin( static_cast< size_type >( index ) );
            return lower_diag_crbegin( static_cast< size_type >( -index ) );
        }
        const_reverse_diag_type diag_crend( const difference_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            matrix_private::check_diagonal( index, zen.row(), zen.col() );
            if ( index > 0 ) return upper_diag_crend( static_cast< size_type >( index ) );
            return lower_diag_crend( static_cast< size_type >( -index ) );
        }
    private:
        // S4-R1: upper diagonal k starts at (0, k), lower diagonal k at (k, 0); stride col()+1.
        template < typename It, typename Z >
        static It upper_diag_range( Z& zen, size_type index, bool at_end ) noexcept
        {
            size_type const rows = zen.row(), cols = zen.col();
            matrix_private::check_diagonal( matrix_private::upper_diagonal( index ), rows, cols );
            size_type const length = index < cols ? std::min( cols - index, rows ) : 0;
            return matrix_private::stride_range< It >( std::to_address( zen.data() ), zen.size(), index, static_cast< difference_type >( cols ) + 1, length, at_end );
        }
        template < typename It, typename Z >
        static It lower_diag_range( Z& zen, size_type index, bool at_end ) noexcept
        {
            size_type const rows = zen.row(), cols = zen.col();
            matrix_private::check_diagonal( matrix_private::lower_diagonal( index ), rows, cols );
            size_type const length = index < rows ? std::min( rows - index, cols ) : 0;
            size_type const start = length ? index * cols : 0;
            return matrix_private::stride_range< It >( std::to_address( zen.data() ), zen.size(), start, static_cast< difference_type >( cols ) + 1, length, at_end );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_direct_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
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

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_divide_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        // S6-R5 (D-023): a scalar or matrix of another element type keeps T; the operand is converted as by
        // static_cast first (to T's value type for complex T and a real scalar); complex into real is rejected.
        template< typename S > requires matrix_details::compound_scalar< S, value_type >
        zen_type& operator/=( const S& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< S, value_type >, "feng::matrix operator/=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            auto const v = matrix_details::scalar_as< value_type >( rhs );
            zen.elementwise_apply( [&v]( value_type& x ) { x /= v; } );
            return zen;
        }
        template< typename U, Allocator B > requires ( !std::same_as< matrix< U, B >, zen_type > )
        zen_type& operator/=( const matrix< U, B >& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< U, value_type >, "feng::matrix operator/=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type converted{ zen.get_allocator(), rhs.row(), rhs.col() };
            std::transform( rhs.begin(), rhs.end(), converted.begin(), []( U const& x ) noexcept { return static_cast< value_type >( x ); } );
            return zen /= converted;
        }
        zen_type& operator/=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen.elementwise_apply( [&rhs]( value_type& v ) { v /= rhs; } );
            return zen;
        }
        zen_type& operator/=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            FENG_MATRIX_EXPECTS( rhs.row() == rhs.col() && zen.col() == rhs.row(), "operator /=: operand shape mismatch, ", zen.row(), "x", zen.col(), " / ", rhs.row(), "x", rhs.col(), " (the divisor must be square with as many rows as the dividend has columns)" );
            zen *= rhs.inverse();
            return zen;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_inverse
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::range_type range_type;
        // S7-R2: the inverse from the LU factors; an empty 0×0 matrix for a singular or nonfinite input (D-026)
        [[nodiscard]] zen_type inverse() const noexcept requires linalg_element< value_type >
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row() == zen.col(), "matrix::inverse: expecting a square matrix, but got ", zen.row(), "x", zen.col() );
            zen_type ans;
            if ( matrix_details::lu_inverse( zen, ans ) != linalg_status::ok )
                return zen_type{};
            return ans;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_load_txt
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Alloc >::value_type value_type;
        typedef typename crtp_typedef< Type, Alloc >::size_type size_type;
        [[nodiscard]] bool load_txt( std::string const& file_name ) noexcept
        {
            return load_txt(file_name.c_str());
        }
        // S5-R2/S5-R4 (F08, D-011): reads the whole file, parses it into a temporary with this matrix's allocator
        // and commits only on success; on failure returns false, leaves the matrix unchanged and prints one stderr
        // line naming load_txt and the reason. Never aborts.
        [[nodiscard]] bool load_txt( const char* const file_name ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::vector< std::uint8_t > bytes;
            if ( !matrix_details::read_file( file_name, bytes, "matrix::load_txt" ) )
                return false;
            zen_type tmp{ zen.get_allocator() };
            char const* why = "invalid text";
            if ( !matrix_details::parse_text< value_type >( reinterpret_cast< char const* >( bytes.data() ), bytes.size(), tmp, &why ) )
            {
                std::cerr << "matrix::load_txt -- " << why << ": " << file_name << "\n";
                return false;
            }
            zen = std::move( tmp );
            return true;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_load_binary
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Alloc >::value_type value_type;
        typedef typename crtp_typedef< Type, Alloc >::size_type size_type;
        [[nodiscard]] bool load_binary( std::string const& file_name ) noexcept
        {
            return load_binary( file_name.c_str() );
        }
        // S5-R2/S5-R4 (F08, D-011): reads the whole file, validates the counts against the payload length with
        // checked arithmetic and commits only on success; on failure returns false, leaves the matrix unchanged and
        // prints one stderr line naming load_binary and the reason. Never aborts.
        [[nodiscard]] bool load_binary( char const* const file_name ) noexcept
        {
            static_assert( matrix_details::is_binary_element_v< value_type >,
                           "load_binary: the native binary format supports arithmetic and std::complex<float|double|long double> elements only" );
            zen_type& zen = static_cast< zen_type& >( *this );
            std::vector< std::uint8_t > bytes;
            if ( !matrix_details::read_file( file_name, bytes, "matrix::load_binary" ) )
                return false;
            zen_type tmp{ zen.get_allocator() };
            char const* why = "invalid binary file";
            if ( !matrix_details::parse_binary< value_type >( bytes.data(), bytes.size(), tmp, &why ) )
            {
                std::cerr << "matrix::load_binary -- " << why << ": " << file_name << "\n";
                return false;
            }
            zen = std::move( tmp );
            return true;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_load_npy
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Alloc >::value_type value_type;
        typedef typename crtp_typedef< Type, Alloc >::size_type size_type;
        [[nodiscard]] bool load_npy( std::string const& file_name ) noexcept
        {
            return load_npy( file_name.c_str() );
        }
        // S5-R1/S5-R4 (F07, D-011): reads the whole file, validates and parses it into a temporary with this matrix's
        // allocator and commits by move assignment only on success. On failure returns false, leaves the matrix
        // unchanged and prints one stderr line naming load_npy and the reason; never aborts.
        [[nodiscard]] bool load_npy( char const* const file_name ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::vector< std::uint8_t > bytes;
            if ( !matrix_details::read_file( file_name, bytes, "matrix::load_npy" ) )
                return false;
            zen_type tmp{ zen.get_allocator() };
            char const* why = "invalid NPY file";
            if ( !matrix_details::parse_npy< value_type >( bytes.data(), bytes.size(), tmp, &why ) )
            {
                std::cerr << "matrix::load_npy -- " << why << ": " << file_name << "\n";
                return false;
            }
            zen = std::move( tmp );
            return true;
        }
    };//struct crtp_load_npy
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_minus_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;

        // S6-R5 (D-023): a scalar or matrix of another element type keeps T; the operand is converted as by
        // static_cast first (to T's value type for complex T and a real scalar); complex into real is rejected.
        template< typename S > requires matrix_details::compound_scalar< S, value_type >
        zen_type& operator-=( const S& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< S, value_type >, "feng::matrix operator-=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            auto const v = matrix_details::scalar_as< value_type >( rhs );
            zen.elementwise_apply( [&v]( value_type& x ) { x -= v; } );
            return zen;
        }
        template< typename U, Allocator B > requires ( !std::same_as< matrix< U, B >, zen_type > )
        zen_type& operator-=( const matrix< U, B >& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< U, value_type >, "feng::matrix operator-=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type converted{ zen.get_allocator(), rhs.row(), rhs.col() };
            std::transform( rhs.begin(), rhs.end(), converted.begin(), []( U const& x ) noexcept { return static_cast< value_type >( x ); } );
            return zen -= converted;
        }
        zen_type& operator-=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen.elementwise_apply( [&rhs]( value_type& v) { v -= rhs; } );
            return zen;
        }

        zen_type& operator-=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            FENG_MATRIX_EXPECTS( zen.row() == rhs.row() && zen.col() == rhs.col(), "operator -=: operand shape mismatch, ", zen.row(), "x", zen.col(), " -= ", rhs.row(), "x", rhs.col() );
            auto v = zen.data();
            auto x = rhs.data();
            auto const& elementwise_minus = [v, x]( size_type offset )
            {
                v[offset] -= x[offset];
            };
            matrix_details::parallel_work( elementwise_minus, 0UL, zen.size(), 1, matrix_details::elementwise_grain );
            return zen;
        }
    };

    namespace matrix_details
    {
        // S10-R3 (D-008): the GEMM kernel behind operator*= and direct_multiply. C = A * B on row-major contiguous
        // arrays (A is m x k, B is k x n, C is m x n). Every C entry is computed as ( ( 0 + a0*b0 ) + a1*b1 ) + ...
        // with k ascending, the order std::inner_product used before S10, so results are bit-identical to
        // gemm_reference for every element type (under the portable flags, which do not contract a*b+c into FMA).
        inline constexpr std::size_t gemm_k_block = 128;
        inline constexpr std::size_t gemm_n_block = 256;

        // Rows [i0, i1) of C. B rows are read contiguously (i-k-j order), four C rows share each B load, and the
        // k and j loops are blocked so a B panel stays in cache; narrow B (n < 4) uses a row-dot path instead.
        template < typename T >
        void gemm_rows( T const* __restrict a, T const* __restrict b, T* __restrict c, std::size_t i0, std::size_t i1, std::size_t K, std::size_t N ) noexcept
        {
            if ( !( i0 < i1 ) || N == 0 ) return;
            std::fill( c + i0 * N, c + i1 * N, T( 0 ) );
            if ( K == 0 ) return;
            if ( N < 4 )
            {
                std::size_t i = i0;
                for ( ; i + 4 <= i1; i += 4 )
                {
                    T const* __restrict a0 = a + i * K;
                    T const* __restrict a1 = a0 + K;
                    T const* __restrict a2 = a1 + K;
                    T const* __restrict a3 = a2 + K;
                    for ( std::size_t j = 0; j != N; ++j )
                    {
                        T s0 = c[i * N + j], s1 = c[( i + 1 ) * N + j], s2 = c[( i + 2 ) * N + j], s3 = c[( i + 3 ) * N + j];
                        for ( std::size_t k = 0; k != K; ++k )
                        {
                            T const bk = b[k * N + j];
                            s0 = s0 + a0[k] * bk;
                            s1 = s1 + a1[k] * bk;
                            s2 = s2 + a2[k] * bk;
                            s3 = s3 + a3[k] * bk;
                        }
                        c[i * N + j] = s0;
                        c[( i + 1 ) * N + j] = s1;
                        c[( i + 2 ) * N + j] = s2;
                        c[( i + 3 ) * N + j] = s3;
                    }
                }
                for ( ; i != i1; ++i )
                {
                    T const* __restrict a0 = a + i * K;
                    for ( std::size_t j = 0; j != N; ++j )
                    {
                        T s0 = c[i * N + j];
                        for ( std::size_t k = 0; k != K; ++k )
                            s0 = s0 + a0[k] * b[k * N + j];
                        c[i * N + j] = s0;
                    }
                }
                return;
            }
            for ( std::size_t jb = 0; jb < N; jb += gemm_n_block )
            {
                std::size_t const je = std::min( N, jb + gemm_n_block );
                for ( std::size_t kb = 0; kb < K; kb += gemm_k_block )
                {
                    std::size_t const ke = std::min( K, kb + gemm_k_block );
                    std::size_t i = i0;
                    for ( ; i + 4 <= i1; i += 4 )
                    {
                        T* __restrict c0 = c + i * N;
                        T* __restrict c1 = c0 + N;
                        T* __restrict c2 = c1 + N;
                        T* __restrict c3 = c2 + N;
                        T const* __restrict ar = a + i * K;
                        for ( std::size_t k = kb; k != ke; ++k )
                        {
                            T const x0 = ar[k], x1 = ar[K + k], x2 = ar[2 * K + k], x3 = ar[3 * K + k];
                            T const* __restrict bk = b + k * N;
                            for ( std::size_t j = jb; j != je; ++j )
                            {
                                T const y = bk[j];
                                c0[j] = c0[j] + x0 * y;
                                c1[j] = c1[j] + x1 * y;
                                c2[j] = c2[j] + x2 * y;
                                c3[j] = c3[j] + x3 * y;
                            }
                        }
                    }
                    for ( ; i != i1; ++i )
                    {
                        T* __restrict c0 = c + i * N;
                        T const* __restrict ar = a + i * K;
                        for ( std::size_t k = kb; k != ke; ++k )
                        {
                            T const x0 = ar[k];
                            T const* __restrict bk = b + k * N;
                            for ( std::size_t j = jb; j != je; ++j )
                                c0[j] = c0[j] + x0 * bk[j];
                        }
                    }
                }
            }
        }

        // C = A * B with the rows of C split into contiguous chunks over `workers` threads (parallel_workers; 0 or
        // 1 runs serially on the caller). Each entry is computed the same way whatever the split.
        template < typename T >
        void gemm_blocked( T const* a, T const* b, T* c, std::size_t M, std::size_t K, std::size_t N, std::size_t workers ) noexcept
        {
            std::size_t const w = effective_workers( std::size_t{ 0 }, M, workers );
            if ( w <= 1 )
            {
                gemm_rows( a, b, c, 0, M, K, N );
                return;
            }
            auto const chunk = [&]( std::size_t k ) noexcept
            {
                auto const [first, last] = chunk_bounds( std::size_t{ 0 }, M, w, k );
                gemm_rows( a, b, c, first, last, K, N );
            };
            parallel_workers( chunk, std::size_t{ 0 }, w, w );
        }

        // The pre-S10 kernel, kept as the reference and fallback: each C entry is a strided std::inner_product of
        // an A row and a B column from value_type( 0 ). The result keeps a's allocator.
        template < typename Matrix >
        Matrix gemm_reference( Matrix const& a, Matrix const& b ) noexcept
        {
            better_assert( a.col() == b.row() && "gemm_reference: dimesion not match!", "gemm_reference: operand shape mismatch, ", a.row(), "x", a.col(), " * ", b.row(), "x", b.col() );
            using value_type = typename Matrix::value_type;
            Matrix tmp( a.get_allocator(), a.row(), b.col() );
            for ( std::size_t i = 0; i != tmp.row(); ++i )
                for ( std::size_t j = 0; j != tmp.col(); ++j )
                    tmp[i][j] = std::inner_product( a.row_begin( i ), a.row_end( i ), b.col_begin( j ), value_type( 0 ) );
            return tmp;
        }
    }//namespace matrix_details

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_multiply_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::range_type range_type;
        // S6-R5 (D-023): a scalar or matrix of another element type keeps T; the operand is converted as by
        // static_cast first (to T's value type for complex T and a real scalar); complex into real is rejected.
        template< typename S > requires matrix_details::compound_scalar< S, value_type >
        zen_type& operator*=( const S& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< S, value_type >, "feng::matrix operator*=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            auto const v = matrix_details::scalar_as< value_type >( rhs );
            zen.elementwise_apply( [&v]( value_type& x ) { x *= v; } );
            return zen;
        }
        template< typename U, Allocator B > requires ( !std::same_as< matrix< U, B >, zen_type > )
        zen_type& operator*=( const matrix< U, B >& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< U, value_type >, "feng::matrix operator*=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type converted{ zen.get_allocator(), rhs.row(), rhs.col() };
            std::transform( rhs.begin(), rhs.end(), converted.begin(), []( U const& x ) noexcept { return static_cast< value_type >( x ); } );
            return zen *= converted;
        }
        zen_type& operator*=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            zen.elementwise_apply( [&rhs]( value_type& v ) { v *= rhs; } );

            return zen;
        }
        zen_type& direct_multiply( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            better_assert( zen.col() == other.row() && "direct_multiply: dimesion not match!", "direct_multiply: operand shape mismatch, ", zen.row(), "x", zen.col(), " * ", other.row(), "x", other.col() );
            zen_type tmp( zen.get_allocator(), zen.row(), other.col() ); // S3-R1: the product stays on this matrix's resource
            // S10-R3: the blocked kernel, rows split over work_workers( M*K*N, gemm_grain ) workers (gemm_blocked caps
            // them at M); the strided inner_product kernel it replaced is matrix_details::gemm_reference.
            std::size_t const work = matrix_details::saturating_work( matrix_details::saturating_work( zen.row(), zen.col() ), other.col() );
            matrix_details::gemm_blocked( zen.data(), other.data(), tmp.data(), zen.row(), zen.col(), other.col(), matrix_details::work_workers( work, matrix_details::gemm_grain ) );
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
            better_assert( zen.col() == other.row() && "operator *= :: the matrix dims not match!", "operator *=: operand shape mismatch, ", zen.row(), "x", zen.col(), " * ", other.row(), "x", other.col() );
            // S10-R3: every shape goes through the blocked kernel in serial and parallel builds; Strassen
            // (strassen_multiply, rr1 ... oc2) is no longer dispatched but stays callable.
            return direct_multiply( other );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_plus_equal_operator
    {
        typedef Matrix                                  zen_type;
        typedef crtp_typedef< Type, Alloc >         type_proxy_type;
        typedef typename type_proxy_type::value_type    value_type;
        typedef typename type_proxy_type::size_type     size_type;
        // S6-R5 (D-023): a scalar or matrix of another element type keeps T; the operand is converted as by
        // static_cast first (to T's value type for complex T and a real scalar); complex into real is rejected.
        template< typename S > requires matrix_details::compound_scalar< S, value_type >
        zen_type& operator+=( const S& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< S, value_type >, "feng::matrix operator+=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            auto const v = matrix_details::scalar_as< value_type >( rhs );
            zen.elementwise_apply( [&v]( value_type& x ) { x += v; } );
            return zen;
        }
        template< typename U, Allocator B > requires ( !std::same_as< matrix< U, B >, zen_type > )
        zen_type& operator+=( const matrix< U, B >& rhs ) noexcept
        {
            static_assert( matrix_details::converts_to_element_v< U, value_type >, "feng::matrix operator+=: a complex operand does not convert to a real element type" );
            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type converted{ zen.get_allocator(), rhs.row(), rhs.col() };
            std::transform( rhs.begin(), rhs.end(), converted.begin(), []( U const& x ) noexcept { return static_cast< value_type >( x ); } );
            return zen += converted;
        }
        zen_type& operator+=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            zen.elementwise_apply( [&rhs]( auto& v ){ v += rhs; } );

            return zen;
        }
        zen_type& operator+=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            FENG_MATRIX_EXPECTS( zen.row() == rhs.row() && zen.col() == rhs.col(), "operator +=: operand shape mismatch, ", zen.row(), "x", zen.col(), " += ", rhs.row(), "x", rhs.col() );

            auto x = zen.data();
            auto y = rhs.data();
            auto const& elementwise_add = [x, y]( size_type offset )
            {
                x[offset] += y[offset];
            };
            matrix_details::parallel_work( elementwise_add, 0UL, zen.size(), 1, matrix_details::elementwise_grain );
            return zen;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_prefix_minus
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type operator-() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            zen_type ans{ zen };
            //std::transform( ans.begin(), ans.end(), ans.begin(), []( value_type x ) { return -x; } );
            auto x = ans.data();
            auto const& minus_function = [x]( size_type offset )
            {
                x[offset] = -x[offset];
            };
            matrix_details::parallel_work( minus_function, 0UL, ans.size(), 1, matrix_details::elementwise_grain );
            return ans;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_prefix_plus
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        zen_type operator+() const noexcept
        {
            return static_cast< zen_type const& >( *this );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_reshape
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        zen_type& reshape( const size_type new_row, const size_type new_col ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            constexpr size_type size_max = std::numeric_limits< size_type >::max();
            FENG_MATRIX_EXPECTS( new_row == 0 || new_col <= size_max / new_row, "matrix reshape: rows*cols overflows, rows = ", new_row, ", cols = ", new_col );
            FENG_MATRIX_EXPECTS( new_row * new_col == zen.size(), "matrix reshape: size before and after reshape does not agree, use resize() instead; size() = ", zen.size(), ", rows = ", new_row, ", cols = ", new_col );
            matrix_private::storage_access::rows( zen ) = new_row;
            matrix_private::storage_access::cols( zen ) = new_col;
            return zen;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_resize
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& resize( const size_type new_row, const size_type new_col ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            if ( zen.size() == matrix_private::checked_count( zen.get_allocator(), new_row, new_col ) )
            {
                zen.reshape( new_row, new_col );
                return zen;
            }

            zen_type ans{ zen.get_allocator(), new_row, new_col };
            zen.swap( ans );
            return zen;
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_row_col_size
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        size_type row() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return matrix_private::storage_access::rows( zen );
        }
        size_type col() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return matrix_private::storage_access::cols( zen );
        }
        size_type size() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return matrix_private::storage_access::storage( zen ).size();
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_row_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
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

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_save_as_txt
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        [[nodiscard]] bool save_as_txt( std::string const& file_name ) const noexcept
        {
            return save_as_txt( file_name.c_str() );
        }
        // S5-R4 (D-011): false with one stderr line on a directory, open, write or close failure; never aborts.
        [[nodiscard]] bool save_as_txt( char const * const file_name ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            if ( file_name == nullptr ) return matrix_details::write_failed( "matrix::save_as_txt", "no file name given", "" );
            return matrix_details::write_stream( "matrix::save_as_txt", std::string{ file_name }, std::ios_base::out,
                                                 [&zen]( std::ofstream& ofs ) { matrix_details::write_text( ofs, zen ); } ); // S5-R2: loads back through load_txt
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_save_as_binary
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        [[nodiscard]] bool save_as_binary( std::string const& file_name ) const noexcept
        {
            return save_as_binary( file_name.c_str() );
        }
        [[nodiscard]] bool save_as_binary( char const* const file_name ) const noexcept
        {
            static_assert( matrix_details::is_binary_element_v< Type >,
                           "save_as_binary: the native binary format supports arithmetic and std::complex<float|double|long double> elements only" );
            zen_type const& zen = static_cast< zen_type const& >( *this );
            if ( file_name == nullptr ) return matrix_details::write_failed( "matrix::save_as_binary", "no file name given", "" );
            // S5-R4 (D-011): false with one stderr line on a directory, open, write or close failure; never aborts.
            return matrix_details::write_stream( "matrix::save_as_binary", std::string{ file_name }, std::ios_base::out | std::ios_base::binary,
                                                 [&zen]( std::ofstream& ofs )
                                                 {
                                                     auto const r = zen.row();
                                                     ofs.write( reinterpret_cast< char const* >( std::addressof( r ) ), sizeof( r ) );
                                                     auto const c = zen.col();
                                                     ofs.write( reinterpret_cast< char const* >( std::addressof( c ) ), sizeof( c ) );
                                                     ofs.write( reinterpret_cast< char const* >( zen.data() ), static_cast< std::streamsize >( sizeof( Type ) * zen.size() ) );
                                                 } );
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_save_as_npy
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        [[nodiscard]] bool save_as_npy( std::string const& file_name ) const noexcept
        {
            return save_as_npy( file_name.c_str() );
        }
        // D-022: a v1.0 C-order NPY file with the D-021 dtype of Type and the native-order payload; S5-R4 (D-011):
        // false with one stderr line on a directory, open, write or close failure; never aborts.
        [[nodiscard]] bool save_as_npy( char const* const file_name ) const noexcept
        {
            constexpr char kind = matrix_details::npy_dtype< Type >::kind;
            static_assert( kind != '\0', "save_as_npy: the element type has no NPY dtype (D-021)" );
            zen_type const& zen = static_cast< zen_type const& >( *this );
            if ( file_name == nullptr ) return matrix_details::write_failed( "matrix::save_as_npy", "no file name given", "" );

            char const order = sizeof( Type ) == 1 ? '|' : ( std::endian::native == std::endian::little ? '<' : '>' );
            std::string header{ "{'descr': '" };
            header += order;
            header += kind;
            header += std::to_string( sizeof( Type ) );
            header += "', 'fortran_order': False, 'shape': (";
            header += std::to_string( zen.row() );
            header += ", ";
            header += std::to_string( zen.col() );
            header += "), }";
            std::size_t const unpadded = 10 + header.size() + 1; // magic, version, length, header, '\n'
            header.append( ( 64 - unpadded % 64 ) % 64, ' ' );
            header += '\n';
            std::size_t const header_len = header.size(); // at most a few hundred bytes, fits the v1.0 u16 length

            return matrix_details::write_stream( "matrix::save_as_npy", std::string{ file_name }, std::ios_base::out | std::ios_base::binary,
                                                 [&]( std::ofstream& ofs )
                                                 {
                                                     char const preamble[10] = { '\x93', 'N', 'U', 'M', 'P', 'Y', '\x01', '\x00',
                                                                                 static_cast< char >( header_len & 0xff ),
                                                                                 static_cast< char >( ( header_len >> 8 ) & 0xff ) };
                                                     ofs.write( preamble, 10 );
                                                     ofs.write( header.data(), static_cast< std::streamsize >( header_len ) );
                                                     ofs.write( reinterpret_cast< char const* >( zen.data() ), static_cast< std::streamsize >( sizeof( Type ) * zen.size() ) );
                                                 } );
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_plot
    {
        typedef Matrix zen_type;
        [[nodiscard]] bool plot( std::string const& file_name, std::string const& color_map=std::string{"parula"} ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.save_as_bmp( file_name, color_map );
        }
    };//struct crtp_plot

    namespace
    {
        // adapted from https://github.com/miloyip/svpng/blob/master/svpng.inc
        // S5-R4 (D-011): returns false, with the reason in `*why`, when the size does not fit the format (width or
        // height above 2^31-1, a row or the IDAT chunk length overflowing 32 bits; checked before opening), or when
        // fopen, a write or fclose fails. Never aborts.
        inline static bool save_png( std::uint8_t const* img, std::uint64_t width, std::uint64_t height, int alpha, char const* const file_name, char const** why = nullptr ) noexcept
        {
            auto const fail = [why]( char const* reason ) noexcept { if ( why ) *why = reason; return false; };
            constexpr std::uint64_t png_max = 0x7fffffffULL;
            if ( width > png_max || height > png_max ) return fail( "image too large for PNG" );
            std::uint64_t row_bytes = 0, idat = 0;
            if ( !matrix_details::checked_mul( width, std::uint64_t{ alpha ? 4U : 3U }, row_bytes ) || ++row_bytes > std::numeric_limits< unsigned >::max() )
                return fail( "PNG row size overflows" );
            if ( !matrix_details::checked_mul( height, row_bytes + 5, idat ) || !matrix_details::checked_add( idat, std::uint64_t{ 6 }, idat ) || idat > png_max )
                return fail( "PNG image data size overflows" );
            if ( file_name == nullptr ) return fail( "no file name given" );

            constexpr unsigned t[] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
            unsigned const w = static_cast< unsigned >( width ), h = static_cast< unsigned >( height );
            unsigned a = 1, b = 0, c, p = static_cast< unsigned >( row_bytes ), x, y, i;
            FILE* fp = fopen( file_name, "wb" );
            if ( fp == nullptr ) return fail( "failed to open file" );

            for ( i = 0; i < 8; i++ )
                fputc( ( "\x89PNG\r\n\32\n" )[i], fp );;

            {
                {
                    fputc( ( 13 ) >> 24, fp );
                    fputc( ( ( 13 ) >> 16 ) & 255, fp );
                    fputc( ( ( 13 ) >> 8 ) & 255, fp );
                    fputc( ( 13 ) & 255, fp );
                }
                c = ~0U;

                for ( i = 0; i < 4; i++ )
                {
                    fputc( ( "IHDR" )[i], fp );
                    c ^= ( ( "IHDR" )[i] );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
            }
            {
                {
                    fputc( ( w ) >> 24, fp );
                    c ^= ( ( w ) >> 24 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( w ) >> 16 ) & 255, fp );
                    c ^= ( ( ( w ) >> 16 ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( w ) >> 8 ) & 255, fp );
                    c ^= ( ( ( w ) >> 8 ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( w ) & 255, fp );
                    c ^= ( ( w ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
            }
            {
                {
                    fputc( ( h ) >> 24, fp );
                    c ^= ( ( h ) >> 24 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( h ) >> 16 ) & 255, fp );
                    c ^= ( ( ( h ) >> 16 ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( h ) >> 8 ) & 255, fp );
                    c ^= ( ( ( h ) >> 8 ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( h ) & 255, fp );
                    c ^= ( ( h ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
            }
            {
                fputc( 8, fp );
                c ^= ( 8 );
                c = ( c >> 4 ) ^ t[c & 15];
                c = ( c >> 4 ) ^ t[c & 15];
            }
            {
                fputc( alpha ? 6 : 2, fp );
                c ^= ( alpha ? 6 : 2 );
                c = ( c >> 4 ) ^ t[c & 15];
                c = ( c >> 4 ) ^ t[c & 15];
            }

            for ( i = 0; i < 3; i++ )
            {
                fputc( ( "\0\0\0" )[i], fp );
                c ^= ( ( "\0\0\0" )[i] );
                c = ( c >> 4 ) ^ t[c & 15];
                c = ( c >> 4 ) ^ t[c & 15];
            }

            {
                fputc( ( ~c ) >> 24, fp );
                fputc( ( ( ~c ) >> 16 ) & 255, fp );
                fputc( ( ( ~c ) >> 8 ) & 255, fp );
                fputc( ( ~c ) & 255, fp );
            }

            {
                {
                    fputc( ( 2 + h * ( 5 + p ) + 4 ) >> 24, fp );
                    fputc( ( ( 2 + h * ( 5 + p ) + 4 ) >> 16 ) & 255, fp );
                    fputc( ( ( 2 + h * ( 5 + p ) + 4 ) >> 8 ) & 255, fp );
                    fputc( ( 2 + h * ( 5 + p ) + 4 ) & 255, fp );
                }
                c = ~0U;

                for ( i = 0; i < 4; i++ )
                {
                    fputc( ( "IDAT" )[i], fp );
                    c ^= ( ( "IDAT" )[i] );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
            }

            for ( i = 0; i < 2; i++ )
            {
                fputc( ( "\x78\1" )[i], fp );
                c ^= ( ( "\x78\1" )[i] );
                c = ( c >> 4 ) ^ t[c & 15];
                c = ( c >> 4 ) ^ t[c & 15];
            }

            for ( y = 0; y < h; y++ )
            {
                {
                    fputc( y == h - 1, fp );
                    c ^= ( y == h - 1 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    {
                        fputc( ( p ) & 255, fp );
                        c ^= ( ( p ) & 255 );
                        c = ( c >> 4 ) ^ t[c & 15];
                        c = ( c >> 4 ) ^ t[c & 15];
                    }
                    {
                        fputc( ( ( p ) >> 8 ) & 255, fp );
                        c ^= ( ( ( p ) >> 8 ) & 255 );
                        c = ( c >> 4 ) ^ t[c & 15];
                        c = ( c >> 4 ) ^ t[c & 15];
                    }
                }
                {
                    {
                        fputc( ( ~p ) & 255, fp );
                        c ^= ( ( ~p ) & 255 );
                        c = ( c >> 4 ) ^ t[c & 15];
                        c = ( c >> 4 ) ^ t[c & 15];
                    }
                    {
                        fputc( ( ( ~p ) >> 8 ) & 255, fp );
                        c ^= ( ( ( ~p ) >> 8 ) & 255 );
                        c = ( c >> 4 ) ^ t[c & 15];
                        c = ( c >> 4 ) ^ t[c & 15];
                    }
                }
                {
                    {
                        fputc( 0, fp );
                        c ^= ( 0 );
                        c = ( c >> 4 ) ^ t[c & 15];
                        c = ( c >> 4 ) ^ t[c & 15];
                    }
                    a = ( a + ( 0 ) ) % 65521;
                    b = ( b + a ) % 65521;
                }

                for ( x = 0; x < p - 1; x++, img++ )
                {
                    {
                        fputc( *img, fp );
                        c ^= ( *img );
                        c = ( c >> 4 ) ^ t[c & 15];
                        c = ( c >> 4 ) ^ t[c & 15];
                    }
                    a = ( a + ( *img ) ) % 65521;
                    b = ( b + a ) % 65521;
                }
            }

            {
                {
                    fputc( ( ( b << 16 ) | a ) >> 24, fp );
                    c ^= ( ( ( b << 16 ) | a ) >> 24 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( ( b << 16 ) | a ) >> 16 ) & 255, fp );
                    c ^= ( ( ( ( b << 16 ) | a ) >> 16 ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( ( b << 16 ) | a ) >> 8 ) & 255, fp );
                    c ^= ( ( ( ( b << 16 ) | a ) >> 8 ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
                {
                    fputc( ( ( b << 16 ) | a ) & 255, fp );
                    c ^= ( ( ( b << 16 ) | a ) & 255 );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
            }

            {
                fputc( ( ~c ) >> 24, fp );
                fputc( ( ( ~c ) >> 16 ) & 255, fp );
                fputc( ( ( ~c ) >> 8 ) & 255, fp );
                fputc( ( ~c ) & 255, fp );
            }

            {
                {
                    fputc( ( 0 ) >> 24, fp );
                    fputc( ( ( 0 ) >> 16 ) & 255, fp );
                    fputc( ( ( 0 ) >> 8 ) & 255, fp );
                    fputc( ( 0 ) & 255, fp );
                }
                c = ~0U;

                for ( i = 0; i < 4; i++ )
                {
                    fputc( ( "IEND" )[i], fp );
                    c ^= ( ( "IEND" )[i] );
                    c = ( c >> 4 ) ^ t[c & 15];
                    c = ( c >> 4 ) ^ t[c & 15];
                }
            }

            {
                fputc( ( ~c ) >> 24, fp );
                fputc( ( ( ~c ) >> 16 ) & 255, fp );
                fputc( ( ( ~c ) >> 8 ) & 255, fp );
                fputc( ( ~c ) & 255, fp );
            }

            bool const written = ferror( fp ) == 0;
            bool const closed = fclose( fp ) == 0;
            if ( !written || !closed ) return fail( "failed to write or close file" );
            return true;
        }//save_png
    }

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_save_as_png
    {
        typedef Matrix zen_type;
        [[nodiscard]] bool save_as_png( const std::string& file_name, std::string const& color_map = std::string{ "parula" } ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row() && "save_as_png: matrix row cannot be zero" );
            better_assert( zen.col() && "save_as_png: matrix column cannot be zero" );

            using matrix_details::bmp_details::color_maps;
            std::string const& map_name       = ( color_maps.find( color_map ) == color_maps.end() ) ? std::string{ "default" } : color_map;
            auto&& selected_map               = ( *( color_maps.find( map_name ) ) ).second;

            auto const [the_row, the_col] = zen.shape();
            // dependent on Type so the lookup waits until matrix is complete (F10); same type as before
            using byte_matrix = matrix<std::uint8_t, std::conditional_t<std::is_same_v<Type, Type>, std::allocator<std::uint8_t>, void>>;
            byte_matrix channel_r{ the_row, the_col };
            byte_matrix channel_g{ the_row, the_col };
            byte_matrix channel_b{ the_row, the_col };

            auto const& [mn, mx] = zen.minmax();
            std::vector<std::uint8_t> cache;
            cache.reserve( the_row * the_col * 3 );

            for ( auto row_index : matrix_details::range( the_row ) )
                for ( auto c : matrix_details::range( the_col ) )
                {
                    //auto const[ r_, g_, b_ ] = selected_map( (zen[the_row-row_index-1][c]-mn)/(mx-mn+1.0e-10) );
                    auto const[ r_, g_, b_ ] = selected_map( (zen[row_index][c]-mn)/(mx-mn+1.0e-10) );
                    cache.push_back( r_ );
                    cache.push_back( g_ );
                    cache.push_back( b_ );
                }

            // S5-R4 (D-011): an oversized image, an open, write or close failure returns false with one stderr line
            char const* why = "";
            if ( !save_png( cache.data(), the_col, the_row, 0, file_name.c_str(), &why ) )
                return matrix_details::write_failed( "matrix::save_as_png", why, file_name );
            return true;
        }
    };//struct crtp_save_as_png


    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_save_as_bmp
    {
        typedef Matrix zen_type;
        [[nodiscard]] bool save_as_bmp( const std::string& file_name, std::string const& color_map = std::string{ "parula" } ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            better_assert( zen.row() && "save_as_bmp: matrix row cannot be zero" );
            better_assert( zen.col() && "save_as_bmp: matrix column cannot be zero" );

            using matrix_details::bmp_details::color_maps;
            std::string const& map_name       = ( color_maps.find( color_map ) == color_maps.end() ) ? std::string{ "default" } : color_map;
            auto&& selected_map               = ( *( color_maps.find( map_name ) ) ).second;

            auto const [the_row, the_col] = zen.shape();
            // dependent on Type so the lookup waits until matrix is complete (F10); same type as before
            using byte_matrix = matrix<std::uint8_t, std::conditional_t<std::is_same_v<Type, Type>, std::allocator<std::uint8_t>, void>>;
            byte_matrix channel_r{ the_row, the_col };
            byte_matrix channel_g{ the_row, the_col };
            byte_matrix channel_b{ the_row, the_col };

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
            matrix_details::parallel_work( make_colormap, 0UL, the_row, the_col, matrix_details::callback_grain );

            auto const& encoding = matrix_details::encode_bmp_stream( channel_r, channel_g, channel_b );

            if ( !encoding ) return matrix_details::write_failed( "matrix::save_as_bmp", "failed to encode the BMP stream", file_name );

            std::string new_file_name{ file_name };
            std::string const extension{ ".bmp" };
            if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;

            // S5-R4 (D-011): false with one stderr line on a directory, open, write or close failure; never aborts.
            return matrix_details::write_stream( "matrix::save_as_bmp", new_file_name, std::ios_base::out | std::ios_base::binary,
                                                 [&encoding]( std::ofstream& stream )
                                                 { stream.write( reinterpret_cast<char const*>((*encoding).data()), static_cast<std::streamsize>( (*encoding).size() ) ); } );
        }

        [[nodiscard]] bool save_as_bmp( char const* const file_name ) const noexcept
        {
            if ( file_name == nullptr ) return matrix_details::write_failed( "matrix::save_as_bmp", "no file name given", "" );
            return save_as_bmp( std::string{ file_name } );
        }
    }; //crtp_save_as_bmp

    template < typename Matrix, typename Type, Allocator Alloc >
    using crtp_save_as_bmp_view = crtp_save_as_bmp<Matrix, Type, Alloc>;

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_save_as_pgm
    {
        typedef Matrix zen_type;
        [[nodiscard]] bool save_as_pgm( const std::string& file_name ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            std::string new_file_name{ file_name };
            std::string const extension{ ".pgm" };

            if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;

            // S5-R4 (D-011): false with one stderr line on a directory, open, write or close failure; never aborts.
            return matrix_details::write_stream( "matrix::save_as_pgm", new_file_name, std::ios_base::out, [&]( std::ofstream& stream )
            {
                stream << "P2\n";
                stream << zen.col() << " " << zen.row() << "\n";
                stream << "255\n";
                stream << "# Generated Portable GrayMap image for path [" << file_name << "]\n";

                double const max_val      = static_cast< double >( *std::max_element( zen.begin(), zen.end() ) );
                double const min_val      = static_cast< double >( *std::min_element( zen.begin(), zen.end() ) );
                double const divider      = max_val - min_val + 1.0e-10;

                for ( auto const r : matrix_details::range( zen.row() ) )
                {
                    for ( auto const c : matrix_details::range( zen.col() ) )
                    {
                        unsigned long const rgb =  static_cast<unsigned long>( 256.0 * ( zen[r][c] - min_val ) / divider );
                        stream << std::min( rgb, 255UL ) << " ";
                    }
                    stream << "\n";
                }
            } );
        }
        [[nodiscard]] bool save_as_pgm( char const* const file_name ) const noexcept
        {
            if ( file_name == nullptr ) return matrix_details::write_failed( "matrix::save_as_pgm", "no file name given", "" );
            return save_as_pgm( std::string{ file_name } );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_shrink_to_size
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;

        //reshape matrix to a new row and new col
        //  if new row or col are larger than the original, padding with zero
        //  otherwise, drop these elements
        zen_type& shrink_to_size( const size_type new_row, const size_type new_col ) noexcept
        {
            FENG_MATRIX_EXPECTS( new_row && new_col, "matrix shrink_to_size: zero extent, new_row = ", new_row, ", new_col = ", new_col );
            zen_type& zen = static_cast< zen_type& >( *this );

            if ( new_row == zen.row() && new_col == zen.col() )
                return zen;

            zen_type other{ zen.get_allocator(), new_row, new_col };
            std::fill( other.begin(), other.end(), value_type{} );
            size_type const the_rows_to_copy = std::min( zen.row(), new_row );
            size_type const the_cols_to_copy = std::min( zen.col(), new_col );

            for ( size_type r = 0; r != the_rows_to_copy; ++r )
                std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_cols_to_copy, other.row_begin( r ) );

            zen.swap( other );
            return zen;
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_stream_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        friend std::ostream& operator<<( std::ostream& lhs, zen_type const& rhs ) noexcept
        {
            lhs.precision( 18 );

            for ( size_type i = 0; i < rhs.row(); ++i )
            {
                std::copy( rhs.row_begin( i ), rhs.row_end( i ), std::ostream_iterator< value_type >( lhs, "\t" ) );
                lhs << "\n";
            }

            return lhs;
        }
        // S5-R2/S5-R4: reads the rest of the stream as load_txt text; on failure sets failbit and leaves rhs unchanged.
        friend std::istream& operator>>( std::istream& is, zen_type& rhs ) noexcept
        {
            std::istream::sentry const ok( is, true );
            if ( !ok ) return is;
            std::string text;
#if defined( __cpp_exceptions )
            // libstdc++'s filebuf throws std::ios_base::failure from underflow on a read error (e.g. EISDIR) whatever the
            // exception mask, so that one is turned into badbit|failbit. Only it is caught: std::bad_alloc and anything
            // else escapes this noexcept function and terminates, as a real allocation failure must (D-012).
            try
            {
                text.assign( std::istreambuf_iterator< char >( is ), std::istreambuf_iterator< char >() );
            }
            catch ( std::ios_base::failure const& )
            {
                is.setstate( std::ios::badbit | std::ios::failbit );
                return is;
            }
#else
            text.assign( std::istreambuf_iterator< char >( is ), std::istreambuf_iterator< char >() );
#endif
            is.setstate( std::ios::eofbit );
            zen_type tmp{ rhs.get_allocator() };
            if ( !matrix_details::parse_text< value_type >( text.data(), text.size(), tmp ) )
            {
                is.setstate( std::ios::failbit );
                return is;
            }
            rhs = std::move( tmp );
            return is;
        }
    };
    template < typename Matrix, typename T, Allocator A >
    struct crtp_swap
    {
        typedef Matrix zen_type;
        void swap( zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            if ( std::addressof( zen ) == std::addressof( other ) ) return;
            typedef typename zen_type::allocator_type allocator_type;
            typedef matrix_private::storage_access access;
            if ( std::allocator_traits< allocator_type >::propagate_on_container_swap::value || zen.get_allocator() == other.get_allocator() )
            {
                access::storage( zen ).swap( access::storage( other ) );
                std::swap( access::rows( zen ), access::rows( other ) );
                std::swap( access::cols( zen ), access::cols( other ) );
                return;
            }
            // S3: unequal allocators and no propagation on swap: exchange the contents element-wise, each side
            // keeping its own allocator; the moves below are between equal allocators, so they steal.
            zen_type to_zen{ zen.get_allocator(), other.row(), other.col() };
            std::copy( other.begin(), other.end(), to_zen.begin() );
            zen_type to_other{ other.get_allocator(), zen.row(), zen.col() };
            std::copy( zen.begin(), zen.end(), to_other.begin() );
            zen = std::move( to_zen );
            other = std::move( to_other );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_tr
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        [[nodiscard]] value_type tr() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return std::accumulate( zen.diag_begin(), zen.diag_end(), value_type() );
        }
    };
    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_transpose
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Alloc > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        [[nodiscard]] zen_type transpose() const noexcept
        {
            const zen_type& zen = static_cast< zen_type const& >( *this );
            zen_type ans( zen.get_allocator(), zen.col(), zen.row() );

            for ( size_type i = 0; i < zen.col(); ++i )
                std::copy( zen.col_begin( i ), zen.col_end( i ), ans.row_begin( i ) );

            return ans;
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_minmax
    {
        typedef Type value_type;
        typedef Matrix zen_type;

        template< typename LessThanCompare >
        [[nodiscard]] auto minmax( LessThanCompare comp ) const noexcept
        {
            // S6-R4 (F15): start from element 0, so all-negative and non-arithmetic inputs give their extremes
            auto const& zen = static_cast<zen_type const&>( *this );
            auto const [the_row, the_col] = zen.shape();
            FENG_MATRIX_EXPECTS( the_row != 0 && the_col != 0, "matrix minmax: empty matrix, shape ", the_row, "x", the_col );
            value_type min_val = zen[0][0];
            value_type max_val = zen[0][0];
            for ( auto r : matrix_details::range(the_row) )
                for ( auto c : matrix_details::range(the_col) )
                {
                    min_val = comp( zen[r][c], min_val ) ? zen[r][c] : min_val;
                    max_val = comp( max_val, zen[r][c] ) ? zen[r][c] : max_val;
                }

            return std::make_tuple( min_val, max_val );
        }

        [[nodiscard]] auto minmax() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return zen.minmax( []( value_type const& x, value_type const& y ) noexcept { return x < y; } );
        }
    };

    template < typename Matrix, typename Type, Allocator Alloc >
    struct crtp_min_max
    {
        typedef Type value_type;
        typedef Matrix zen_type;

        template< typename LessThanCompare >
        [[nodiscard]] value_type min( LessThanCompare comp ) const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            FENG_MATRIX_EXPECTS( zen.size() != 0, "matrix min: empty matrix, shape ", zen.row(), "x", zen.col() );
            return matrix_details::reduce( zen.begin() + 1, zen.end(), *zen.begin(), [&comp]( value_type const& a, value_type const& b ){ return comp(a, b) ? a : b; } );
        }

        template< typename LessThanCompare >
        [[nodiscard]] value_type max( LessThanCompare comp ) const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            FENG_MATRIX_EXPECTS( zen.size() != 0, "matrix max: empty matrix, shape ", zen.row(), "x", zen.col() );
            return matrix_details::reduce( zen.begin() + 1, zen.end(), *zen.begin(), [&comp]( value_type const& a, value_type const& b ){ return comp(a, b) ? b : a; } );
        }

        [[nodiscard]] value_type min() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return zen.min( []( value_type const& x, value_type const& y ) noexcept { return x < y; } );
        }

        [[nodiscard]] value_type max() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>( *this );
            return zen.max( []( value_type const& x, value_type const& y ) noexcept { return x < y; } );
        }

    };

    template < typename Matrix, typename Type, Allocator Alloc >
    using crtp_minmax_view = crtp_minmax<Matrix, Type, Alloc>;

    // S4-R2 (F06, F05; D-019): rank-2 views. Each keeps its parent (allocator, checked-mode origin and extent), a
    // pointer to the first viewed element (the parent's data() for an empty view), its extents and the parent's
    // physical row stride. A view does not own or extend the parent's lifetime; it is invalidated by the
    // parent's destruction, reallocation, assignment or shape change. Shared members of both view types.
    template < typename View, typename P, typename Matrix >
    struct view_members
    {
        typedef std::size_t                                                         size_type;
        typedef std::ptrdiff_t                                                      difference_type;
        typedef std::remove_const_t< std::remove_pointer_t< P > >                   value_type;
        typedef value_type const*                                                   const_pointer;
        typedef P                                                                   pointer;
        typedef pointer                                                             row_type;
        typedef const_pointer                                                       const_row_type;
        typedef stride_iterator< pointer >                                          col_type;
        typedef stride_iterator< const_pointer >                                    const_col_type;
        typedef std::reverse_iterator< col_type >                                   reverse_col_type;
        typedef std::reverse_iterator< const_col_type >                             const_reverse_col_type;
        typedef view_iterator< pointer >                                            iterator;
        typedef view_iterator< const_pointer >                                      const_iterator;
        typedef std::reverse_iterator< iterator >                                   reverse_iterator;
        typedef std::reverse_iterator< const_iterator >                             const_reverse_iterator;
        typedef std::iter_reference_t< pointer >                                    reference;
        typedef value_type const&                                                   const_reference;

    protected:
        Matrix const*                                                               parent_ = nullptr;
        pointer                                                                     data_ = nullptr;
        size_type                                                                   rows_ = 0;
        size_type                                                                   cols_ = 0;
        size_type                                                                   row_stride_ = 0;

        view_members() noexcept = default;
        view_members( Matrix const* parent, pointer data, size_type rows, size_type cols, size_type row_stride ) noexcept
            : parent_( parent ), data_( data ), rows_( rows ), cols_( cols ), row_stride_( row_stride ) { }

        // the first viewed element of rows [r0, r1), columns [c0, c1) of `mat`, after check_view_range
        template < typename Q >
        static Q first_element( Q origin, std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1, std::size_t parent_cols ) noexcept
        {
            return ( r0 == r1 || c0 == c1 ) ? origin : origin + ( r0 * parent_cols + c0 );
        }

        void check_row( size_type r ) const noexcept
        {
            FENG_MATRIX_EXPECTS( r < rows_ && "Row index out of boundary!", "matrix index: row ", r, " with row() ", rows_ );
        }

        // the parent's data() as a Q; a mutable view was made from a non-const parent, so dropping const is sound
        template < typename Q >
        Q origin() const noexcept
        {
            return Q( const_cast< value_type* >( parent_->data() ) );
        }

        template < typename Q >
        stride_iterator< Q > column( size_type c, difference_type index ) const noexcept
        {
            matrix_private::check_column( c, cols_ );
            Q const base = rows_ ? Q( data_ + c ) : Q( data_ );
            return stride_iterator< Q >( base, static_cast< difference_type >( row_stride_ ), static_cast< difference_type >( rows_ ), index, origin< Q >(), parent_->size() );
        }

        template < typename Q >
        view_iterator< Q > element( difference_type index ) const noexcept
        {
            return view_iterator< Q >( Q( data_ ), static_cast< difference_type >( row_stride_ ), static_cast< difference_type >( cols_ ),
                                       static_cast< difference_type >( rows_ * cols_ ), index, origin< Q >(), parent_->size() );
        }

    public:
        size_type row() const noexcept { return rows_; }
        size_type col() const noexcept { return cols_; }
        size_type size() const noexcept { return rows_ * cols_; }
        bool empty() const noexcept { return size() == 0; }
        size_type row_stride() const noexcept { return row_stride_; }
        auto get_allocator() const noexcept { return parent_->get_allocator(); }

        row_type operator[]( size_type r ) const noexcept { check_row( r ); return data_ + r * row_stride_; }
        reference at( size_type r, size_type c ) const noexcept
        {
            matrix_private::check_index( r, c, rows_, cols_ );
            return data_[ r * row_stride_ + c ];
        }

        row_type row_begin( size_type r ) const noexcept { check_row( r ); return data_ + r * row_stride_; }
        row_type row_end( size_type r ) const noexcept { return row_begin( r ) + cols_; }
        const_row_type row_cbegin( size_type r ) const noexcept { return row_begin( r ); }
        const_row_type row_cend( size_type r ) const noexcept { return row_end( r ); }

        col_type col_begin( size_type c ) const noexcept { return column< pointer >( c, 0 ); }
        col_type col_end( size_type c ) const noexcept { return column< pointer >( c, static_cast< difference_type >( rows_ ) ); }
        const_col_type col_cbegin( size_type c ) const noexcept { return column< const_pointer >( c, 0 ); }
        const_col_type col_cend( size_type c ) const noexcept { return column< const_pointer >( c, static_cast< difference_type >( rows_ ) ); }
        reverse_col_type col_rbegin( size_type c ) const noexcept { return reverse_col_type( col_end( c ) ); }
        reverse_col_type col_rend( size_type c ) const noexcept { return reverse_col_type( col_begin( c ) ); }
        const_reverse_col_type col_crbegin( size_type c ) const noexcept { return const_reverse_col_type( col_cend( c ) ); }
        const_reverse_col_type col_crend( size_type c ) const noexcept { return const_reverse_col_type( col_cbegin( c ) ); }

        iterator begin() const noexcept { return element< pointer >( 0 ); }
        iterator end() const noexcept { return element< pointer >( static_cast< difference_type >( size() ) ); }
        const_iterator cbegin() const noexcept { return element< const_pointer >( 0 ); }
        const_iterator cend() const noexcept { return element< const_pointer >( static_cast< difference_type >( size() ) ); }
        reverse_iterator rbegin() const noexcept { return reverse_iterator( end() ); }
        reverse_iterator rend() const noexcept { return reverse_iterator( begin() ); }
        const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( cend() ); }
        const_reverse_iterator crend() const noexcept { return const_reverse_iterator( cbegin() ); }
    };

    // the const view; make_view( m, {r0, r1}, {c0, c1} ) of an lvalue owner
    template < matrix_element Type, Allocator Alloc >
    struct matrix_view :
        view_members< matrix_view<Type, Alloc>, Type const*, matrix<Type, Alloc> >,
        crtp_save_as_bmp_view<matrix_view<Type, Alloc>, Type, Alloc>,
        crtp_shape_view<matrix_view<Type, Alloc>, Type, Alloc>,
        crtp_minmax_view<matrix_view<Type, Alloc>, Type, Alloc>
    {
    private:
        typedef view_members< matrix_view<Type, Alloc>, Type const*, matrix<Type, Alloc> > base_type;
    public:
        typedef typename base_type::size_type                                       size_type;
        typedef typename base_type::value_type                                      value_type;
        typedef Alloc                                                               allocator_type;
        typedef matrix<Type, Alloc>                                                 matrix_type;
        typedef std::pair<size_type, size_type>                                     range_type;

        matrix_view( matrix_type const& mat, range_type const& row_dim, range_type const& col_dim ) noexcept
        {
            matrix_private::check_view_range( row_dim.first, row_dim.second, col_dim.first, col_dim.second, mat.row(), mat.col() );
            (*this).parent_ = std::addressof( mat );
            (*this).data_ = base_type::first_element( mat.data(), row_dim.first, row_dim.second, col_dim.first, col_dim.second, mat.col() );
            (*this).rows_ = row_dim.second - row_dim.first;
            (*this).cols_ = col_dim.second - col_dim.first;
            (*this).row_stride_ = mat.col();
        }
        // S4-R3: a view of a temporary owner would dangle
        matrix_view( matrix_type&&, range_type const&, range_type const& ) = delete;
        matrix_view( matrix_type const&&, range_type const&, range_type const& ) = delete;

        // a mutable view converts to the const view of the same elements
        matrix_view( mutable_matrix_view<Type, Alloc> const& other ) noexcept
            : base_type( other.parent_, other.data_, other.rows_, other.cols_, other.row_stride_ ) { }

        matrix_view( matrix_view const& ) noexcept = default;
        matrix_view& operator=( matrix_view const& ) noexcept = default;

        // read-only access, as for a const matrix
        value_type operator()( size_type r, size_type c ) const noexcept { return (*this).at( r, c ); }
    };//struct matrix_view

    // the mutable view; make_mutable_view( m, {r0, r1}, {c0, c1} ) of a non-const lvalue owner. Constness is
    // shallow, as for std::span: a const mutable view still writes the parent's elements.
    template < matrix_element Type, Allocator Alloc >
    struct mutable_matrix_view :
        view_members< mutable_matrix_view<Type, Alloc>, Type*, matrix<Type, Alloc> >,
        crtp_save_as_bmp_view<mutable_matrix_view<Type, Alloc>, Type, Alloc>,
        crtp_shape_view<mutable_matrix_view<Type, Alloc>, Type, Alloc>,
        crtp_minmax_view<mutable_matrix_view<Type, Alloc>, Type, Alloc>
    {
    private:
        typedef view_members< mutable_matrix_view<Type, Alloc>, Type*, matrix<Type, Alloc> > base_type;
        friend struct matrix_view<Type, Alloc>;
    public:
        typedef typename base_type::size_type                                       size_type;
        typedef typename base_type::value_type                                      value_type;
        typedef Alloc                                                               allocator_type;
        typedef matrix<Type, Alloc>                                                 matrix_type;
        typedef std::pair<size_type, size_type>                                     range_type;

        mutable_matrix_view( matrix_type& mat, range_type const& row_dim, range_type const& col_dim ) noexcept
        {
            matrix_private::check_view_range( row_dim.first, row_dim.second, col_dim.first, col_dim.second, mat.row(), mat.col() );
            (*this).parent_ = std::addressof( mat );
            (*this).data_ = base_type::first_element( mat.data(), row_dim.first, row_dim.second, col_dim.first, col_dim.second, mat.col() );
            (*this).rows_ = row_dim.second - row_dim.first;
            (*this).cols_ = col_dim.second - col_dim.first;
            (*this).row_stride_ = mat.col();
        }
        // S4-R3: a view of a temporary owner would dangle; a mutable view of a const owner would write through it
        mutable_matrix_view( matrix_type const&, range_type const&, range_type const& ) = delete;
        mutable_matrix_view( matrix_type&&, range_type const&, range_type const& ) = delete;
        mutable_matrix_view( matrix_type const&&, range_type const&, range_type const& ) = delete;

        mutable_matrix_view( mutable_matrix_view const& ) noexcept = default;
        mutable_matrix_view& operator=( mutable_matrix_view const& ) noexcept = default;

        typename base_type::reference operator()( size_type r, size_type c ) const noexcept { return (*this).at( r, c ); }
    };//struct mutable_matrix_view

    template < matrix_element Type, Allocator Alloc = std::allocator<Type> >
    struct matrix : crtp_anti_diag_iterator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_apply< matrix< Type, Alloc >, Type, Alloc >
        , crtp_bracket_operator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_clear< matrix< Type, Alloc >, Type, Alloc >
        , crtp_clone< matrix< Type, Alloc >, Type, Alloc >
        , crtp_col_iterator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_copy< matrix< Type, Alloc >, Type, Alloc >
        , crtp_data< matrix< Type, Alloc >, Type, Alloc >
        , crtp_det< matrix< Type, Alloc >, Type, Alloc >
        , crtp_diag_iterator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_direct_iterator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_divide_equal_operator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_get_allocator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_inverse< matrix< Type, Alloc >, Type, Alloc >
        , crtp_item< matrix< Type, Alloc >, Type, Alloc >
        , crtp_load_binary< matrix< Type, Alloc >, Type, Alloc >
        , crtp_load_npy< matrix< Type, Alloc >, Type, Alloc >
        , crtp_load_txt< matrix< Type, Alloc >, Type, Alloc >
        , crtp_min_max< matrix< Type, Alloc >, Type, Alloc >
        , crtp_minmax< matrix< Type, Alloc >, Type, Alloc >
        , crtp_minus_equal_operator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_multiply_equal_operator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_opencv< matrix< Type, Alloc >, Type, Alloc >
        , crtp_plot< matrix< Type, Alloc >, Type, Alloc >
        , crtp_plus_equal_operator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_prefix_minus< matrix< Type, Alloc >, Type, Alloc >
        , crtp_prefix_plus< matrix< Type, Alloc >, Type, Alloc >
        , crtp_reshape< matrix< Type, Alloc >, Type, Alloc >
        , crtp_resize< matrix< Type, Alloc >, Type, Alloc >
        , crtp_row_col_size< matrix< Type, Alloc >, Type, Alloc >
        , crtp_row_iterator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_save_as_binary< matrix< Type, Alloc >, Type, Alloc >
        , crtp_save_as_npy< matrix< Type, Alloc >, Type, Alloc >
        , crtp_save_as_bmp< matrix< Type, Alloc >, Type, Alloc >
        , crtp_save_as_pgm< matrix< Type, Alloc >, Type, Alloc >
        , crtp_save_as_png< matrix< Type, Alloc >, Type, Alloc >
        , crtp_save_as_txt< matrix< Type, Alloc >, Type, Alloc >
        , crtp_shape< matrix< Type, Alloc >, Type, Alloc >
        , crtp_shrink_to_size< matrix< Type, Alloc >, Type, Alloc >
        , crtp_stream_operator< matrix< Type, Alloc >, Type, Alloc >
        , crtp_swap< matrix< Type, Alloc >, Type, Alloc >
        , crtp_tr< matrix< Type, Alloc >, Type, Alloc >
        , crtp_transpose< matrix< Type, Alloc >, Type, Alloc >
    {
        typedef matrix self_type;
        typedef crtp_typedef< Type, Alloc >                         type_proxy_type;
        typedef typename type_proxy_type::value_type                    value_type;
        typedef typename type_proxy_type::size_type                     size_type;
        typedef typename type_proxy_type::difference_type               difference_type;
        typedef typename type_proxy_type::pointer                       pointer;
        typedef typename type_proxy_type::allocator_type                allocator_type;
        typedef typename type_proxy_type::range_type                    range_type;

        static_assert( std::is_same_v< typename std::allocator_traits< Alloc >::value_type, Type >,
                       "feng::matrix<Type, Alloc>: the allocator's value_type must be Type; rebind the allocator to the element type" );

    private:
        friend struct matrix_private::storage_access;
        typedef std::allocator_traits< allocator_type >                 allocator_traits_type;
        // S3-R1: private RAII storage; storage_.size() == row_ * col_ after every operation.
        size_type                                                       row_ = 0;
        size_type                                                       col_ = 0;
        std::vector< value_type, allocator_type >                       storage_;

    public:
        ~matrix() noexcept = default;

        // S3-R2: the free swap follows the member swap (POCS, or element-wise for unequal allocators), not std::swap's
        // move-based fallback, which would follow POCMA instead.
        friend void swap( self_type& lhs, self_type& rhs ) noexcept { lhs.swap( rhs ); }

        matrix( std::integral auto row, std::integral auto col, std::initializer_list<value_type> const& value_list ) noexcept : matrix{}
        {
            FENG_MATRIX_EXPECTS( matrix_private::dimension_fits( row ), "matrix size: negative or unrepresentable rows = ", row );
            FENG_MATRIX_EXPECTS( matrix_private::dimension_fits( col ), "matrix size: negative or unrepresentable cols = ", col );
            matrix_private::checked_count( (*this).get_allocator(), static_cast<size_type>(row), static_cast<size_type>(col) );
            (*this).resize( static_cast<size_type>(row), static_cast<size_type>(col) );
            size_type elements_to_copy = std::min( (*this).size(), value_list.size() );
            std::copy( value_list.begin(), value_list.begin()+elements_to_copy, (*this).begin() );
        }

        // S3-R1: steals the storage; the source is left 0x0 with size 0.
        matrix( self_type&& other ) noexcept : row_{ other.row_ }, col_{ other.col_ }, storage_( std::move( other.storage_ ) )
        {
            other.storage_.clear();
            other.row_ = 0;
            other.col_ = 0;
        }

        // S3-R1: self-move keeps the contents; otherwise the vector honours POCMA and the source is left 0x0.
        self_type& operator = ( self_type&& other ) noexcept
        {
            if ( this == std::addressof( other ) ) return *this;
            // S3-R2: unequal allocators that do not propagate make the vector allocate on this side; check first.
            if constexpr ( !allocator_traits_type::propagate_on_container_move_assignment::value && !allocator_traits_type::is_always_equal::value )
                if ( (*this).get_allocator() != other.get_allocator() )
                    matrix_private::checked_count( (*this).get_allocator(), other.row_, other.col_ );
            storage_ = std::move( other.storage_ );
            row_ = other.row_;
            col_ = other.col_;
            other.storage_.clear();
            other.row_ = 0;
            other.col_ = 0;
            return *this;
        }

        // S3-R5: the selected allocator may differ from other's (and so may its max_size); check before copying.
        matrix( const self_type& other ) noexcept : row_{ other.row_ }, col_{ other.col_ },
            storage_( other.storage_, matrix_private::checked_allocator( allocator_traits_type::select_on_container_copy_construction( other.storage_.get_allocator() ), other.row_, other.col_ ) )
        {
        }

        template < typename T, Allocator A >
        matrix( matrix< T, A> const& other ) noexcept : matrix{ allocator_type( other.get_allocator() ), other.row(), other.col() }
        {
            std::copy( other.begin(), other.end(), (*this).begin() );
        }

        explicit matrix( const size_type r = 0, const size_type c = 0, value_type const& v = value_type{} ) noexcept : matrix{ allocator_type{}, r, c, v }
        {
        }

        explicit matrix( allocator_type a, const size_type r = 0, const size_type c = 0, value_type const& v = value_type{} ) noexcept : row_{ r }, col_{ c },
            storage_( matrix_private::checked_count( a, r, c ), v, a )
        {
        }

        template< typename T, Allocator A >
        matrix( matrix<T,A> const& other, std::initializer_list<size_type> rr, std::initializer_list<size_type> rc ) noexcept : storage_( allocator_type( other.get_allocator() ) )
        {
            matrix_private::check_clone_lists( rr.size(), rc.size() );
            auto [rr0, rr1] = std::make_pair( *(rr.begin()), *(rr.begin()+1) );
            auto [rc0, rc1] = std::make_pair( *(rc.begin()), *(rc.begin()+1) );
            (*this).clone( other, rr0, rr1, rc0, rc1 );
        }

        matrix( matrix const& other, std::initializer_list<size_type> rr, std::initializer_list<size_type> rc ) noexcept : storage_( other.get_allocator() )
        {
            matrix_private::check_clone_lists( rr.size(), rc.size() );
            auto [rr0, rr1] = std::make_pair( *(rr.begin()), *(rr.begin()+1) );
            auto [rc0, rc1] = std::make_pair( *(rc.begin()), *(rc.begin()+1) );
            (*this).clone( other, rr0, rr1, rc0, rc1 );
        }

        template < typename T, Allocator A >
        matrix( const matrix< T, A >& other, const range_type& rr, const range_type& rc ) noexcept : matrix{ other, {rr.first, rr.second}, {rc.first, rc.second} } {}

        matrix( matrix const& other, range_type const& rr, range_type const& rc ) noexcept : matrix{ other, {rr.first, rr.second}, {rc.first, rc.second} } {}

        template < typename T, Allocator A >
        matrix( const matrix< T, A >& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : matrix{ other, {r0, r1}, {c0, c1} } {}

        matrix( self_type const& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : matrix{ other, {r0, r1}, {c0, c1} } {}

        self_type& operator = ( const self_type& rhs ) noexcept
        {
            if ( this == std::addressof( rhs ) ) return *this; // S2-R4: self-assignment keeps the contents
            // S3-R1: check the count against the allocator the destination will hold, then let the vector honour POCCA.
            if constexpr ( allocator_traits_type::propagate_on_container_copy_assignment::value )
                matrix_private::checked_count( rhs.get_allocator(), rhs.row_, rhs.col_ );
            else
                matrix_private::checked_count( (*this).get_allocator(), rhs.row_, rhs.col_ );
            storage_ = rhs.storage_;
            row_ = rhs.row_;
            col_ = rhs.col_;
            return *this;
        }

        template < typename T, Allocator A >
        self_type& operator = ( const matrix< T, A >& rhs ) noexcept
        {
            if ( static_cast< void const* >( this ) == static_cast< void const* >( std::addressof( rhs ) ) ) return *this; // S2-R4
            ( *this ).copy( rhs );
            return *this;
        }

        self_type& operator = ( const value_type& v ) noexcept
        {
            std::fill( ( *this ).diag_begin(), ( *this ).diag_end(), v ); //TODO:should move to crtp_xxx
            return *this;
        }

        template< typename T >
        [[nodiscard]] auto astype() const noexcept
        {
            matrix<T, typename std::allocator_traits<Alloc>:: template rebind_alloc<T> > ans{ (*this).get_allocator(), (*this).row(), (*this).col() };
            std::copy( (*this).begin(), (*this).end(), ans.begin() ); //TODO: should move to crtp_xxx
            return ans;
        }

        // S3-R4 (F03), S4-R2: copies exactly the viewed rectangle, row by row, using the parent's allocator.
        matrix( matrix_view<Type, Alloc> const& v ) noexcept : matrix{ v.get_allocator(), v.row(), v.col() }
        {
            for ( size_type r = 0; r != v.row(); ++r )
                std::copy( v.row_begin( r ), v.row_end( r ), (*this).row_begin( r ) );
        }

        matrix( mutable_matrix_view<Type, Alloc> const& v ) noexcept : matrix{ v.get_allocator(), v.row(), v.col() }
        {
            for ( size_type r = 0; r != v.row(); ++r )
                std::copy( v.row_begin( r ), v.row_end( r ), (*this).row_begin( r ) );
        }

    };//struct matrix

    //
    // - begin of Matrix and ComplexMatrix concepts
    //

    template< typename T >
    struct is_matrix : std::false_type{};

    template< typename T, Allocator A >
    struct is_matrix< matrix<T,A> > : std::true_type{};

    template< typename M >
    inline constexpr bool is_matrix_v = is_matrix<M>::value;

    template< typename M >
    concept Matrix = is_matrix_v<M>;


    template< typename T >
    struct is_complex_matrix : std::false_type{};

    template< typename T, Allocator A >
    struct is_complex_matrix< matrix<std::complex<T>,A> > : std::true_type{};

    template< typename M >
    inline constexpr bool is_complex_matrix_v = is_complex_matrix<M>::value;

    template< typename M >
    concept ComplexMatrix = Matrix<M> && is_complex_matrix_v<M>; // refines Matrix so the complex overloads win

    //
    // - end of Matrix and ComplexMatrix concepts
    //

    namespace matrix_details
    {
        // integral 0 < floating 1 < complex 2
        template< matrix_scalar T >
        struct element_kind : std::integral_constant< int, std::is_integral_v< T > ? 0 : ( std::is_floating_point_v< T > ? 1 : 2 ) > {};

        template< typename T >
        struct element_real { using type = T; };
        template< typename X >
        struct element_real< std::complex< X > > { using type = X; };
        template< typename T >
        using element_real_t = typename element_real< T >::type;

        // matrix (+) matrix: std::common_type_t for two real types; complex<common_type_t<X, Y>> when either is complex.
        template< typename T, typename U >
        struct common_element { using type = T; }; // same non-scalar element type: unchanged
        template< matrix_scalar T, matrix_scalar U >
        struct common_element< T, U >
        {
            using type = std::conditional_t< element_kind< T >::value == 2 || element_kind< U >::value == 2,
                                             std::complex< std::common_type_t< element_real_t< T >, element_real_t< U > > >,
                                             std::common_type_t< T, U > >;
        };
        template< typename T, typename U >
        using common_element_t = typename common_element< T, U >::type;

        // matrix (+) scalar: T when kind(S) <= kind(T), else common_element_t<T, S>.
        template< typename T, typename S >
        struct scalar_result { using type = T; }; // same non-scalar element type: unchanged
        template< matrix_scalar T, matrix_scalar S >
        struct scalar_result< T, S >
        {
            using type = std::conditional_t< ( element_kind< S >::value <= element_kind< T >::value ), T, common_element_t< T, S > >;
        };
        template< typename T, typename S >
        using scalar_result_t = typename scalar_result< T, S >::type;

        // The operand pairs the operators accept: two scalars, or a non-scalar element type with itself (as before S6).
        template< typename S, typename T >
        concept scalar_operand_for = ( matrix_scalar< S > && matrix_scalar< T > ) || ( !matrix_scalar< T > && std::same_as< S, T > );
        template< typename T, typename U >
        concept element_pair = ( matrix_scalar< T > && matrix_scalar< U > ) || std::same_as< T, U >;

        template< typename R, typename T, Allocator A >
        using promoted_matrix_t = matrix< R, typename std::allocator_traits< A >::template rebind_alloc< R > >;

        // A copy of m with elements static_cast to R; the allocator is m's (as a copy selects it) rebound to R.
        template< typename R, typename T, Allocator A >
        promoted_matrix_t< R, T, A > promote( matrix< T, A > const& m ) noexcept
        {
            if constexpr ( std::same_as< promoted_matrix_t< R, T, A >, matrix< T, A > > )
                return m;
            else
            {
                typename std::allocator_traits< A >::template rebind_alloc< R > alloc( std::allocator_traits< A >::select_on_container_copy_construction( m.get_allocator() ) );
                promoted_matrix_t< R, T, A > ans{ alloc, m.row(), m.col() };
                std::transform( m.begin(), m.end(), ans.begin(), []( T const& x ) noexcept { return static_cast< R >( x ); } );
                return ans;
            }
        }

        // S9-R6: the one shape check for the matrix operands of an elementwise function (map, the binary family, fma);
        // a mismatch aborts with "<name>: operand shape mismatch, RxC and RxC" (or "RxC, RxC and RxC").
        template< Matrix M, Matrix N >
        void expect_same_shape( char const* name, M const& m, N const& n ) noexcept
        {
            FENG_MATRIX_EXPECTS( m.row() == n.row() && m.col() == n.col(), name, ": operand shape mismatch, ", m.row(), "x", m.col(), " and ", n.row(), "x", n.col() );
        }

        template< Matrix M, Matrix N, Matrix L >
        void expect_same_shape( char const* name, M const& m, N const& n, L const& l ) noexcept
        {
            FENG_MATRIX_EXPECTS( m.row() == n.row() && m.col() == n.col() && m.row() == l.row() && m.col() == l.col(), name, ": operand shape mismatch, ", m.row(), "x", m.col(), ", ", n.row(), "x", n.col(), " and ", l.row(), "x", l.col() );
        }

        // rhs as an operand of a compound operator on a matrix<R, B>: itself when the types match, else converted.
        template< typename R, Allocator B, typename T, Allocator A >
        decltype( auto ) operand_as( matrix< R, B > const& target, matrix< T, A > const& rhs ) noexcept
        {
            if constexpr ( std::same_as< matrix< R, B >, matrix< T, A > > )
                return ( rhs );
            else
            {
                matrix< R, B > ans{ target.get_allocator(), rhs.row(), rhs.col() };
                std::transform( rhs.begin(), rhs.end(), ans.begin(), []( T const& x ) noexcept { return static_cast< R >( x ); } );
                return ans;
            }
        }

        namespace map_impl_private
        {
            template< typename T, Allocator A >
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

            template< typename T, Allocator A, typename T2, Allocator A2 >
            auto map_impl( matrix<T, A> const& mat, matrix<T2, A2> const& nat ) noexcept
            {
                matrix_details::expect_same_shape( "map", mat, nat );
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T, T2> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), nat.begin(), ans.begin(), [&]( auto const& u, auto const& v, value_type& a ){ a = func(u, v); } );
                    return ans;
                };
            }

            template< typename T, Allocator A, typename T2, Allocator A2, typename T3, Allocator A3 >
            auto map_impl( matrix<T, A> const& mat, matrix<T2, A2> const& nat, matrix<T3, A3> const& lat ) noexcept
            {
                matrix_details::expect_same_shape( "map", mat, nat, lat );
                return [&]( auto const& func ) noexcept
                {
                    typedef typename std::invoke_result_t<decltype(func), T, T2, T3> value_type;
                    typename std::allocator_traits<A>:: template rebind_alloc<value_type> ans_alloc{ mat.get_allocator() };
                    matrix<value_type, decltype(ans_alloc)> ans{ ans_alloc, mat.row(), mat.col() };
                    matrix_details::for_each( mat.begin(), mat.end(), nat.begin(), lat.begin(), ans.begin(), [&]( auto const& u, auto const& v, auto const& w, value_type& a ){ a = func(u, v, w); } );
                    return ans;
                };
            }

            template< typename T, Allocator A, typename T2 >
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

            template< typename T, Allocator A, typename T2 >
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

        // S4-R4 (F12): the returned lambda holds a copy of func, so it may outlive the argument.
        template< typename Func >
        auto map( Func const& func ) noexcept
        {
            return [func]( auto const& ... mat ) noexcept
            {
                return map_impl_private::map_impl( mat ... )( func );
            };
        }

        namespace reduce_impl_private
        {
            // S4-R4, S6-R1 (F11, F12): index reads only, so no pointer past mat.end() is formed; the fold is
            // matrix_details::reduce_range. An optional trailing worker count defaults to work_workers( n, reduce_grain ) (S10-R3).
            template< typename T, Allocator A >
            auto reduce_impl( matrix<T, A> const& mat ) noexcept
            {
                return [&mat]( auto const& func, auto const& init, auto const... workers ) noexcept
                {
                    static_assert( sizeof...( workers ) <= 1, "reduce_impl( mat )( func, init [, workers] )" );
                    std::size_t const n = mat.size();
                    T const* const data = mat.data();
                    std::size_t w = work_workers( n, reduce_grain );
                    ( ( w = static_cast<std::size_t>( workers ) ), ... );
                    auto const at = [data]( std::size_t i ) noexcept -> T const& { return data[i]; };
                    return matrix_details::reduce_range<T>( at, n, static_cast<T>( init ), func, w );
                };
            }
        }//reduce_impl_private

        // S4-R4 (F12): the returned lambda holds copies of func and init, so it may outlive both.
        template< typename Func, typename Type >
        auto reduce( Func const& func, Type init ) noexcept
        {
            return [func, init]( auto const& mat ) noexcept
            {
                return reduce_impl_private::reduce_impl( mat )( func, init );
            };
        }

    }

    // S4-R2, S4-R3 (D-019): the const view of rows [r0, r1) and columns [c0, c1) of an lvalue owner; each list
    // holds exactly two values and the ranges must lie in the owner (checked in every build).
    template< typename Type, Allocator Alloc, typename Integer_Type >
    [[nodiscard]] matrix_view<Type, Alloc>
    make_view( matrix<Type, Alloc> const& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) noexcept
    {
        auto const rows = matrix_private::view_extent( row_dim, "row" );
        auto const cols = matrix_private::view_extent( col_dim, "column" );
        return matrix_view<Type, Alloc>{ owner, rows, cols };
    }

    // S4-R3: a view of a temporary owner would dangle.
    template< typename Type, Allocator Alloc, typename Integer_Type >
    matrix_view<Type, Alloc>
    make_view( matrix<Type, Alloc>&& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) = delete;

    template< typename Type, Allocator Alloc, typename Integer_Type >
    matrix_view<Type, Alloc>
    make_view( matrix<Type, Alloc> const&& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) = delete;

    // S4-R2, S4-R3 (D-019): the mutable view of rows [r0, r1) and columns [c0, c1) of a non-const lvalue owner.
    template< typename Type, Allocator Alloc, typename Integer_Type >
    [[nodiscard]] mutable_matrix_view<Type, Alloc>
    make_mutable_view( matrix<Type, Alloc>& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) noexcept
    {
        auto const rows = matrix_private::view_extent( row_dim, "row" );
        auto const cols = matrix_private::view_extent( col_dim, "column" );
        return mutable_matrix_view<Type, Alloc>{ owner, rows, cols };
    }

    // S4-R3: a mutable view of a const owner would write through it; one of a temporary owner would dangle.
    template< typename Type, Allocator Alloc, typename Integer_Type >
    mutable_matrix_view<Type, Alloc>
    make_mutable_view( matrix<Type, Alloc> const& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) = delete;

    template< typename Type, Allocator Alloc, typename Integer_Type >
    mutable_matrix_view<Type, Alloc>
    make_mutable_view( matrix<Type, Alloc>&& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) = delete;

    template< typename Type, Allocator Alloc, typename Integer_Type >
    mutable_matrix_view<Type, Alloc>
    make_mutable_view( matrix<Type, Alloc> const&& owner, std::initializer_list<Integer_Type> row_dim, std::initializer_list<Integer_Type> col_dim ) = delete;

    // S9-R4 (D-032): spans over an owner's m.size() contiguous elements and over row r's m.col() elements
    // (std::span< T const > for a const owner); r must name a row. Rvalue owners are rejected, the span would dangle.
    template< typename T, Allocator A >
    [[nodiscard]] std::span< T > as_span( matrix< T, A >& m ) noexcept { return { std::to_address( m.data() ), m.size() }; }
    template< typename T, Allocator A >
    [[nodiscard]] std::span< T const > as_span( matrix< T, A > const& m ) noexcept { return { std::to_address( m.data() ), m.size() }; }
    template< typename T, Allocator A >
    std::span< T const > as_span( matrix< T, A > const&& m ) = delete;

    template< typename M > requires is_matrix_v< std::remove_const_t< M > >
    [[nodiscard]] auto row_span( M& m, std::size_t r ) noexcept
    {
        FENG_MATRIX_EXPECTS( r < m.row(), "row_span: row ", r, " outside a matrix with ", m.row(), " rows" );
        return as_span( m ).subspan( r * m.col(), m.col() );
    }
    template< typename T, Allocator A >
    std::span< T const > row_span( matrix< T, A > const&& m, std::size_t r ) = delete;

#if defined( __cpp_lib_mdspan )
    // S9-R4: the layout_right mdspan of extents (row, col) aliasing m.data().
    template< typename T, Allocator A >
    [[nodiscard]] auto to_mdspan( matrix< T, A >& m ) noexcept { return std::mdspan< T, std::dextents< std::size_t, 2 > >( std::to_address( m.data() ), m.row(), m.col() ); }
    template< typename T, Allocator A >
    [[nodiscard]] auto to_mdspan( matrix< T, A > const& m ) noexcept { return std::mdspan< T const, std::dextents< std::size_t, 2 > >( std::to_address( m.data() ), m.row(), m.col() ); }
    template< typename T, Allocator A >
    std::mdspan< T const, std::dextents< std::size_t, 2 > > to_mdspan( matrix< T, A > const&& m ) = delete;
#endif
#if defined( __cpp_lib_submdspan )
    // S9-R4: rows [r0, r1) and columns [c0, c1) of to_mdspan( m ), validated as make_view's ranges (S4-R3).
    template< typename M, typename Integer_Type > requires is_matrix_v< std::remove_const_t< M > >
    [[nodiscard]] auto submdspan( M& m, std::initializer_list< Integer_Type > row_dim, std::initializer_list< Integer_Type > col_dim ) noexcept
    {
        auto const [r0, r1] = matrix_private::view_extent( row_dim, "row" );
        auto const [c0, c1] = matrix_private::view_extent( col_dim, "column" );
        matrix_private::check_view_range( r0, r1, c0, c1, m.row(), m.col() );
        return std::submdspan( to_mdspan( m ), std::pair{ r0, r1 }, std::pair{ c0, c1 } );
    }
    template< typename T, Allocator A, typename Integer_Type >
    void submdspan( matrix< T, A > const&& m, std::initializer_list< Integer_Type > row_dim, std::initializer_list< Integer_Type > col_dim ) = delete;
#endif

    // S6-R5 (D-023): matrix (+) matrix gives matrix< common_element_t< T1, T2 >, A1 rebound >; when that is
    // matrix< T1, A1 > the result is a copy of lhs as before, otherwise lhs promoted; rhs is converted to the result.
    template < typename T1, Allocator A1, typename T2, Allocator A2 > requires matrix_details::element_pair< T1, T2 >
    matrix_details::promoted_matrix_t< matrix_details::common_element_t< T1, T2 >, T1, A1 >
    operator+( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        FENG_MATRIX_EXPECTS( lhs.row() == rhs.row() && lhs.col() == rhs.col(), "operator +: operand shape mismatch, ", lhs.row(), "x", lhs.col(), " + ", rhs.row(), "x", rhs.col() );
        auto ans = matrix_details::promote< matrix_details::common_element_t< T1, T2 > >( lhs );
        ans += matrix_details::operand_as( ans, rhs );
        return ans;
    }

    template < typename T1, Allocator A1, typename T2, Allocator A2 > requires matrix_details::element_pair< T1, T2 >
    matrix_details::promoted_matrix_t< matrix_details::common_element_t< T1, T2 >, T1, A1 >
    operator-( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        FENG_MATRIX_EXPECTS( lhs.row() == rhs.row() && lhs.col() == rhs.col(), "operator -: operand shape mismatch, ", lhs.row(), "x", lhs.col(), " - ", rhs.row(), "x", rhs.col() );
        auto ans = matrix_details::promote< matrix_details::common_element_t< T1, T2 > >( lhs );
        ans -= matrix_details::operand_as( ans, rhs );
        return ans;
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 > requires matrix_details::element_pair< T1, T2 >
    matrix_details::promoted_matrix_t< matrix_details::common_element_t< T1, T2 >, T1, A1 >
    operator*( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        FENG_MATRIX_EXPECTS( lhs.col() == rhs.row(), "operator *: operand shape mismatch, ", lhs.row(), "x", lhs.col(), " * ", rhs.row(), "x", rhs.col() );
        auto ans = matrix_details::promote< matrix_details::common_element_t< T1, T2 > >( lhs );
        ans *= matrix_details::operand_as( ans, rhs );
        return ans;
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 > requires matrix_details::element_pair< T1, T2 >
    matrix_details::promoted_matrix_t< matrix_details::common_element_t< T1, T2 >, T1, A1 >
    operator/( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        FENG_MATRIX_EXPECTS( rhs.row() == rhs.col() && lhs.col() == rhs.row(), "operator /: operand shape mismatch, ", lhs.row(), "x", lhs.col(), " / ", rhs.row(), "x", rhs.col() );
        auto ans = matrix_details::promote< matrix_details::common_element_t< T1, T2 > >( lhs );
        ans /= matrix_details::operand_as( ans, rhs );
        return ans;
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    bool operator<( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        better_assert( lhs.row() == rhs.row() );
        better_assert( lhs.col() == rhs.col() );
        return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    bool operator==( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        better_assert( lhs.row() == rhs.row() );
        better_assert( lhs.col() == rhs.col() );
        return std::equal( lhs.begin(), lhs.end(), rhs.begin() );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    bool operator>( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        return !( ( lhs < rhs ) || ( lhs == rhs ) );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    bool operator>=( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        return !( lhs < rhs );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    bool operator<=( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
    {
        return !( lhs > rhs );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    matrix< T1, A1 >
    operator||( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
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
    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    matrix< T1, A1 >
    operator&&( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs ) noexcept
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
    template < typename T, Allocator A, typename T_ >
    matrix< T, A >
    operator*( const matrix< T, A >& lhs, const T_* const rhs ) noexcept
    {
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs, T() );

        return ans;
    }
    template < typename T, Allocator A, typename T_ >
    matrix< T, A >
    operator*( const T_* lhs, const matrix< T, A >& rhs ) noexcept
    {
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs, lhs + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T, Allocator A, typename T_ >
    matrix< T, A >
    operator*( const matrix< T, A >& lhs, const std::valarray< T_ >& rhs ) noexcept
    {
        better_assert( lhs.col() == rhs.size(), "matrix * valarray: shape mismatch, matrix columns ", lhs.col(), " but valarray length ", rhs.size() );
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), std::begin( rhs ), T() );

        return ans;
    }
    template < typename T, Allocator A, typename T_ >
    matrix< T, A >
    operator*( const std::valarray< T_ >& lhs, const matrix< T, A >& rhs ) noexcept
    {
        better_assert( rhs.row() == lhs.size(), "valarray * matrix: shape mismatch, valarray length ", lhs.size(), " but matrix rows ", rhs.row() );
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( std::begin( lhs ), std::begin( lhs ) + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T, Allocator A, typename T_ >
    matrix< T, A >
    operator*( const matrix< T, A >& lhs, const std::vector< T_ >& rhs ) noexcept
    {
        better_assert( lhs.col() == rhs.size(), "matrix * vector: shape mismatch, matrix columns ", lhs.col(), " but vector length ", rhs.size() );
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs.begin(), T() );

        return ans;
    }
    template < typename T, Allocator A, typename T_ >
    matrix< T, A >
    operator*( const std::vector< T_ >& lhs, const matrix< T, A >& rhs ) noexcept
    {
        // S2-R4 (B-003): the vector length must equal the matrix rows; only rhs.row() elements are read.
        better_assert( rhs.row() == lhs.size(), "vector * matrix: shape mismatch, vector length ", lhs.size(), " but matrix rows ", rhs.row() );
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs.begin(), lhs.begin() + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }

    template < typename T1, Allocator A1, typename T2, Allocator A2 >
    [[nodiscard]] matrix< T1, A1 > blkdiag( const matrix< T1, A1 >& m1, const matrix< T2, A2 >& m2 ) noexcept
    {
        return ( m1 || matrix< T1, A1 >{ m1.get_allocator(), m1.row(), m2.col() } ) && ( matrix< T1, A1 >{ m1.get_allocator(), m2.row(), m1.col() } || m2 );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename... Matrices >
    [[nodiscard]] matrix< T1, A1 > blkdiag( const matrix< T1, A1 >& m1, const matrix< T2, A2 >& m2, const Matrices& ... matrices ) noexcept
    {
        return blkdiag( blkdiag( m1, m2 ), matrices... );
    }
    template < typename T, Allocator A, typename... Matrices >
    [[nodiscard]] matrix< T, A > blk_diag( const matrix< T, A >& m, const Matrices& ... matrices ) noexcept
    {
        return blkdiag( m, matrices... );
    }
    template < typename T, Allocator A, typename... Matrices >
    [[nodiscard]] matrix< T, A > block_diag( const matrix< T, A >& m, const Matrices& ... matrices ) noexcept
    {
        return blkdiag( m, matrices... );
    }

    template < ComplexMatrix CMat >
    [[nodiscard]] CMat ctranspose( CMat const& m ) noexcept
    {
        return conj( m.transpose() );
    }
    template < typename T, Allocator A> requires linalg_element< T >
    [[nodiscard]] T det( const matrix< T, A >& m ) noexcept
    {
        return m.det();
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > diag( const matrix< T, A >& m, const std::ptrdiff_t offset = 0 ) noexcept
    {
        const std::uint_least64_t dim = std::min( m.row(), m.col() ) + ( offset > 0 ? offset : -offset );
        matrix< T, A > ans{ dim, dim };
        if ( m.diag_begin() != m.diag_end() ) // an empty source leaves diagonal `offset` absent from ans
            std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin( offset ) );
        return ans;
    }
    namespace diag_private
    {
        template < typename Itor >
        matrix< typename std::iterator_traits< Itor >::value_type >
        impl_diag( Itor first, Itor last, const std::ptrdiff_t offset = 0 ) noexcept
        {
            std::uint_least64_t dim = std::distance( first, last ) + ( offset > 0 ? offset : -offset );
            matrix< typename std::iterator_traits< Itor >::value_type > ans{ dim, dim };
            if ( first != last ) // an empty source leaves diagonal `offset` absent from ans
                std::copy( first, last, ans.diag_begin( offset ) );
            return ans;
        }
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T > diag( const std::vector< T, A >& v, const std::ptrdiff_t offset = 0 ) noexcept
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T > diag( const std::deque< T, A >& v, const std::ptrdiff_t offset = 0 ) noexcept
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename C, Allocator A>
    [[nodiscard]] matrix< T > diag( const std::set< T, C, A >& v, const std::ptrdiff_t offset = 0 ) noexcept
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename C, Allocator A>
    [[nodiscard]] matrix< T > diag( const std::multiset< T, C, A >& v, const std::ptrdiff_t offset = 0 ) noexcept
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T >
    [[nodiscard]] matrix< T > diag( const std::valarray< T >& v, const std::ptrdiff_t offset = 0 ) noexcept
    {
        return diag_private::impl_diag( std::begin( v ), std::end( v ), offset );
    }
    namespace make_diag_private
    {
        struct impl_make_diag
        {
            std::uint_least64_t pos;
            impl_make_diag( const std::uint_least64_t pos_ = 0 ) noexcept
                : pos( pos_ )
            {
            }
            template < typename T, Allocator A, typename Arg, typename... Args >
            void operator()( matrix< T, A >& m, const Arg& arg, const Args& ... args ) const noexcept
            {
                m[pos][pos] = arg;
                impl_make_diag( pos + 1 )( m, args... );
            }
            template < typename T, Allocator A, typename Arg >
            void operator()( matrix< T, A >& m, const Arg& arg ) const noexcept
            {
                *( m.diag_rbegin() ) = arg;
            }
        };
    }
    template < typename T, typename... Tn >
    [[nodiscard]] matrix< T > make_diag( const T& v1, const Tn& ... vn ) noexcept
    {
        const std::uint_least64_t n = 1 + sizeof...( vn );
        matrix< T > ans{ n, n };
        make_diag_private::impl_make_diag()( ans, v1, vn... );
        return ans;
    }
    template < typename T, Allocator A>
    void display( const matrix< T, A >& m ) noexcept
    {
        std::cout << m << std::endl;
    }
    template < typename T, Allocator A>
    void disp( const matrix< T, A >& m ) noexcept
    {
        display( m );
    }
    template < typename Matrix1, typename Matrix2 >
    [[nodiscard]] typename Matrix1::value_type
    dot( const Matrix1& m1, const Matrix2& m2 ) noexcept
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
            T operator()() const noexcept
            {
                return T( 1 );
            }
        };
        template < typename T >
        struct one_maker< std::complex< T >>
        {
            std::complex< T > operator()() const noexcept
            {
                return std::complex< T >( T( 1 ), T( 0 ) );
            }
        };
    };
    template < typename T, typename A    = std::allocator< T > >
    [[nodiscard]] matrix< T, A > eye( const std::uint_least64_t r, const std::uint_least64_t c ) noexcept
    {
        matrix< T > ans{ r, c };
        std::fill( ans.diag_begin(), ans.diag_end(), eye_private::one_maker< T >()() );
        return ans;
    }
    template < typename T, typename A    = std::allocator< T > >
    [[nodiscard]] matrix< T, A > eye( const std::uint_least64_t n ) noexcept
    {
        return eye< T, A >( n, n );
    }
    template < typename T, typename A    = std::allocator< T > >
    [[nodiscard]] matrix< T, A > eye( const matrix< T, A >& m ) noexcept
    {
        return eye< T, A >( m.row(), m.col() );
    }
    // S2-R3 (D-004): dim 1 maps row i to row rows-1-i, dim 2 maps column j to column cols-1-j; fewer than two rows
    // or columns returns the copy; any other dim aborts with `flipdim`.
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > flipdim( const matrix< T, A >& m, const std::uint_least64_t dim ) noexcept
    {
        FENG_MATRIX_EXPECTS( 1 == dim || 2 == dim, "matrix flipdim: dim should be 1 or 2, got ", dim );
        matrix< T, A > ans{ m };

        if ( 1 == dim )
        {
            if ( ans.row() < 2 )
                return ans;
            for ( std::uint_least64_t upper = 0, lower = ans.row() - 1; upper < lower; ++upper, --lower )
                std::swap_ranges( ans.row_begin( upper ), ans.row_end( upper ), ans.row_begin( lower ) );
            return ans;
        }

        if ( ans.col() < 2 )
            return ans;
        for ( std::uint_least64_t r = 0; r != ans.row(); ++r )
            std::reverse( ans.row_begin( r ), ans.row_end( r ) );
        return ans;
    }
    // D-004: MATLAB/NumPy convention, fliplr flips columns and flipud flips rows.
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > fliplr( const matrix< T, A >& m ) noexcept
    {
        return flipdim( m, 2 );
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > flipud( const matrix< T, A >& m ) noexcept
    {
        return flipdim( m, 1 );
    }
    template < typename T,
               typename A    = std::allocator< std::remove_cvref_t< T > >>
    [[nodiscard]] matrix< T, A > hilb( const std::uint_least64_t n ) noexcept
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
               typename A    = std::allocator< std::remove_cvref_t< T > >>
    [[nodiscard]] matrix< T, A > hilbert( const std::uint_least64_t n ) noexcept
    {
        return hilb< T, A >( n );
    }
    template < typename Matrix >
    [[nodiscard]] Matrix hilb( const std::uint_least64_t n, const Matrix& ) noexcept
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
    [[nodiscard]] Matrix hilbert( const std::uint_least64_t n, const Matrix& m ) noexcept
    {
        return hilb( n, m );
    }
    template < typename Matrix > requires linalg_element< typename Matrix::value_type >
    [[nodiscard]] Matrix inverse( const Matrix& m ) noexcept
    {
        return m.inverse();
    }
    template < typename Matrix > requires linalg_element< typename Matrix::value_type >
    [[nodiscard]] Matrix inv( const Matrix& m ) noexcept
    {
        return m.inverse();
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_column( const matrix< T, A >& m ) noexcept
    {
        return m.col() == 1;
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_column_matrix( const matrix< T, A >& m ) noexcept
    {
        return is_column( m );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool iscolumn( const matrix< T, A >& m ) noexcept
    {
        return is_column( m );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_empty( const matrix< T, A >& m ) noexcept
    {
        return m.size() == 0;
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_empty_matrix( const matrix< T, A >& m ) noexcept
    {
        return is_empty( m );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool isempty( const matrix< T, A >& m ) noexcept
    {
        return is_empty( m );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_equal( const matrix< T, A >& m1, const matrix< T, A > m2 ) noexcept
    {
        return m1 == m2;
    }
    template < typename M1, typename M2, typename... Mn >
    [[nodiscard]] bool is_equal( const M1& m1, const M2& m2, const Mn& ... mn ) noexcept
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool isequal( const matrix< T, A >& m1, const matrix< T, A > m2 ) noexcept
    {
        return is_equal( m1, m2 );
    }
    template < typename M1, typename M2, typename... Mn >
    [[nodiscard]] bool isequal( const M1& m1, const M2& m2, const Mn& ... mn ) noexcept
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< std::uint8_t > is_inf( const matrix< T, A >& m ) noexcept // D-017: 0/1 byte mask
    {
        matrix< std::uint8_t > ans( m.row(), m.col() );
        matrix_details::for_each( m.begin(), m.end(), ans.begin(), []( const T & v, std::uint8_t & a )
        {
            a = std::isinf( v ) ? 1 : 0;
        } );
        return ans;
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< std::uint8_t > isinf( const matrix< T, A >& m ) noexcept
    {
        return is_inf( m );
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< std::uint8_t > is_nan( const matrix< T, A >& m ) noexcept // D-017: 0/1 byte mask
    {
        matrix< std::uint8_t > ans( m.row(), m.col() );
        matrix_details::for_each( m.begin(), m.end(), ans.begin(), []( const T & v, std::uint8_t & a )
        {
            a = std::isnan( v ) ? 1 : 0;
        } );
        return ans;
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< std::uint8_t > isnan( const matrix< T, A >& m ) noexcept
    {
        return is_nan( m );
    }
    template < typename T, Allocator A, typename F >
    [[nodiscard]] bool is_orthogonal( const matrix< T, A >& m, F f ) noexcept
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
    template < typename T, Allocator A>
    [[nodiscard]] bool is_orthogonal( const matrix< T, A >& m ) noexcept
    {
        return is_orthogonal( m, []( const T v )
        {
            return v == T( 0 );
        } );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_positive_definite( const matrix< T, A >& m ) noexcept
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
    template < typename T, Allocator A>
    [[nodiscard]] bool is_row( const matrix< T, A >& m ) noexcept
    {
        return m.row() == 1;
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool is_row_matrix( const matrix< T, A >& m ) noexcept
    {
        return is_row( m );
    }
    template < typename T, Allocator A>
    [[nodiscard]] bool isrow( const matrix< T, A >& m ) noexcept
    {
        return is_row( m );
    }
    template < typename T, Allocator A, typename F >
    [[nodiscard]] bool is_symmetric( const matrix< T, A >& m, F f ) noexcept
    {
        if ( m.row() != m.col() )
            return false;

        for ( std::uint_least64_t i = 1; i != m.row(); ++i )
            if ( !std::equal( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), m.lower_diag_cbegin( i ), f ) )
                return false;

        return true;
    }

    template < typename T, Allocator A>
    [[nodiscard]] bool is_symmetric( const matrix< T, A >& m ) noexcept
    {
        return is_symmetric( m, []( const T v1, const T v2 )
        {
            return v1 == v2;
        } );
    }

    // S9-R1: the element type and allocator are parameters; the construction runs in std::uint_least64_t
    // arithmetic and stores each value as T, so magic( n ) is unchanged.
    template < typename T = std::uint_least64_t, typename A = std::allocator< T > >
    [[nodiscard]] matrix< T, A > magic( const std::uint_least64_t n ) noexcept
    {
        if ( 3 == n )
            return matrix< T, A >{ 3, 3, { 8, 1, 6, 3, 5, 7, 4, 9, 2 } };

        if ( 4 == n )
            return matrix< T, A >{ 4, 4, { 16, 3, 2, 13, 5, 10, 11, 8, 9, 6, 7, 12, 4, 15, 14, 1} };

        matrix< T, A > ans{ n, n }; // after the literal cases: one allocation per result

        if ( 2 == n ) return ans; // no magic for n = 2

        // odd case
        if ( n & 1 )
        {
            for ( std::uint_least64_t i = 0; i < n; ++i )
                for ( std::uint_least64_t j = 0; j < n; ++j )
                    //ans[( ( n - 1 ) / 2 + i - j + n ) % n][( 3 * n - 1 + j - 2 * i ) % n] = i * n + j + 1;
                    ans[n - (( 3 * n - 1 + j - 2 * i ) % n) - 1][n - (( ( n - 1 ) / 2 + i - j + n ) % n) - 1] = static_cast< T >( i * n + j + 1 );

            return ans;
        }

        // singly even LUX
        if ( n & 2 )
        {
            auto const half = n >> 1;
            auto const m_half = magic< std::uint_least64_t >( half );
            // L
            for ( auto r : matrix_details::range( (half+1) >> 1 ) )
                for ( auto c : matrix_details::range( half ) )
                {
                    auto const val = m_half[r][c];
                    ans[r<<1][c<<1] = static_cast< T >( val << 2 );         ans[r<<1][(c<<1)+1] = static_cast< T >( (val << 2) - 3 );
                    ans[(r<<1)+1][c<<1] = static_cast< T >( (val << 2) - 2 );   ans[(r<<1)+1][(c<<1)+1] = static_cast< T >( (val << 2) - 1 );
                }

            // U
            for ( auto c : matrix_details::range(half) )
            {
                auto const r = (half+1) >> 1;
                auto const val = m_half[r][c];
                ans[r<<1][c<<1] = static_cast< T >( (val << 2) - 3 );    ans[r<<1][(c<<1)+1] = static_cast< T >( (val << 2) );
                ans[(r<<1)+1][c<<1] = static_cast< T >( (val << 2) - 2 );    ans[(r<<1)+1][(c<<1)+1] = static_cast< T >( (val << 2) - 1 );
            }

            // swap central block
            if (1)
            {
                {
                    auto const [r,c] = std::make_tuple( (half-1)>>1, (half-1)>>1 );
                    auto const val = m_half[r][c];
                    ans[r<<1][c<<1] = static_cast< T >( (val << 2) - 3 );    ans[r<<1][(c<<1)+1] = static_cast< T >( (val << 2) );
                    ans[(r<<1)+1][c<<1] = static_cast< T >( (val << 2) - 2 );    ans[(r<<1)+1][(c<<1)+1] = static_cast< T >( (val << 2) - 1 );
                }
                {
                    auto const [r,c] = std::make_tuple( (half+1)>>1, (half+1)>>1 );
                    auto const val = m_half[r][c];
                    ans[r<<1][c<<1] = static_cast< T >( (val << 2) );    ans[r<<1][(c<<1)+1] = static_cast< T >( (val << 2) - 3 );
                    ans[(r<<1)+1][c<<1] = static_cast< T >( (val << 2) - 2 );    ans[(r<<1)+1][(c<<1)+1] = static_cast< T >( (val << 2) - 1 );
                }
            }
            // X
            for ( auto r : matrix_details::range( (half+3) >> 1, half ) )
                for ( auto c : matrix_details::range( half ) )
                {
                    auto const& val = m_half[r][c];
                    ans[r<<1][c<<1] = static_cast< T >( (val << 2) - 3 );   ans[r<<1][(c<<1)+1] = static_cast< T >( (val << 2) );
                    ans[(r<<1)+1][c<<1] = static_cast< T >( (val << 2) - 1 );   ans[(r<<1)+1][(c<<1)+1] = static_cast< T >( (val << 2) - 2 );
                }

            return ans;
        }

        // doubly even <X>
        std::iota( ans.begin(), ans.end(), static_cast< T >( 1 ) );
        std::reverse( ans.diag_begin(), ans.diag_end() );
        std::reverse( ans.anti_diag_begin(), ans.anti_diag_end() );
        std::swap_ranges( ans.lower_diag_begin( n >> 1 ), ans.lower_diag_end( n >> 1 ), ans.upper_diag_rbegin( n >> 1 ) );
        std::swap_ranges( ans.upper_anti_diag_begin( n >> 1 ), ans.upper_anti_diag_end( n >> 1 ), ans.lower_anti_diag_rbegin( n >> 1 ) );
        return ans;
    }

    template< Matrix Mat >
    [[nodiscard]] auto max( Mat const& m ) noexcept
    {
        FENG_MATRIX_EXPECTS( m.size() != 0, "feng::max: empty matrix, shape ", m.row(), "x", m.col() );
        return *std::max_element( m.begin(), m.end() );
    }

    template< Matrix Mat >
    [[nodiscard]] auto min( Mat const& m ) noexcept
    {
        FENG_MATRIX_EXPECTS( m.size() != 0, "feng::min: empty matrix, shape ", m.row(), "x", m.col() );
        return *std::min_element( m.begin(), m.end() );
    }

    template < typename T >
    [[nodiscard]] matrix<T> arange( std::uint_least64_t start, const std::uint_least64_t stop, const std::uint_least64_t step = 1ULL ) noexcept
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
    [[nodiscard]] matrix<T> arange( const std::uint_least64_t length ) noexcept
    {
        matrix<T> ans{ 1, length };
        std::iota( ans.begin(), ans.end(), T{0} );
        return ans;
    }

    template < typename T >
    [[nodiscard]] matrix<T> linspace( T start, T stop, const std::uint_least64_t num = 50ULL, bool end_point=true ) noexcept
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

    template < Matrix Mat >
    [[nodiscard]] Mat ones_like( Mat const& mat ) noexcept
    {
        return Mat{ mat.get_allocator(), mat.row(), mat.col(), typename Mat::value_type{1} };
    }

    template < typename T >
    [[nodiscard]] matrix<T, std::allocator<T>> ones( std::integral auto r, std::integral auto c ) noexcept
    {
        matrix< T > ans{ static_cast<unsigned long>(r), static_cast<unsigned long>(c), T{ 1 } };
        return ans;
    }

    template < typename T >
    [[nodiscard]] matrix<T, std::allocator<T>> ones( std::integral auto n ) noexcept
    {
        return ones< T >( n, n );
    }

    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > ones( A const& alloc, std::integral auto r, std::integral auto c ) noexcept
    {
        return matrix< T, A >{ alloc, static_cast<unsigned long>(r) , static_cast<unsigned long>(c), T{1} };
    }

    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > ones( A const& alloc, std::integral auto n ) noexcept
    {
        return matrix< T, A >{ alloc, static_cast<unsigned long>(n) , static_cast<unsigned long>(n), T{1} };
    }


    template < Matrix Mat >
    [[nodiscard]] Mat zeros_like( Mat const& mat ) noexcept
    {
        return Mat{ mat.get_allocator(), mat.row(), mat.col(), typename Mat::value_type{} };
    }

    template < typename T >
    [[nodiscard]] matrix<T, std::allocator<T>> zeros( std::integral auto r, std::integral auto c ) noexcept
    {
        matrix< T > ans{ static_cast<unsigned long>(r), static_cast<unsigned long>(c), T{} };
        return ans;
    }

    template < typename T >
    [[nodiscard]] matrix<T, std::allocator<T>> zeros( std::integral auto n ) noexcept
    {
        return zeros< T >( n, n );
    }

    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > zeros( A const& alloc, std::integral auto r, std::integral auto c ) noexcept
    {
        return matrix< T, A >{ alloc, static_cast<unsigned long>(r) , static_cast<unsigned long>(c), T{} };
    }

    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > zeros( A const& alloc, std::integral auto n ) noexcept
    {
        return matrix< T, A >{ alloc, static_cast<unsigned long>(n) , static_cast<unsigned long>(n), T{} };
    }



    template < typename T >
    [[nodiscard]] auto empty( const std::integral auto r, const std::integral auto c ) noexcept
    {
        return matrix< T >{ static_cast<unsigned long>(r), static_cast<unsigned long>(c) };
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > empty( A const& alloc, std::integral auto r, std::integral auto c ) noexcept
    {
        return matrix< T, A >{ alloc, static_cast<unsigned long>(r), static_cast<unsigned long>(c) };
    }

    // S7-R3 (F13, D-026, D-028): one-sided Hestenes Jacobi SVD of an m×n A, real or complex. The kernel runs on
    // B = A (m ≥ n) or B = Aᴴ (m < n, the roles of U and V are then swapped), with complex rotations for complex T;
    // a sweep visits every column pair and B has converged when every pair has |b_iᴴb_j| ≤ ε·‖b_i‖‖b_j‖. Thin
    // factors: u() is m×k, v() is n×k, k = min(m, n), and s() is a std::vector of the k singular values (the real
    // type of T), descending, with A = U·diag(s)·Vᴴ. A u column (or, for m < n, v column) whose singular value is
    // exactly zero is completed by Gram–Schmidt on unit vectors, so UᴴU = VᴴV = I on all k columns. status() is ok,
    // not_converged (the sweep limit was reached; the factors hold the last iterate) or nonfinite (a NaN or inf in
    // A; the factors are then empty). A is scaled by its largest |a_ij| for the iteration and s is scaled back.
    template < typename T, Allocator A_ = std::allocator< T > > requires linalg_element< T >
    class svd_factorization
    {
    public:
        using matrix_type = matrix< T, A_ >;
        using real_type = matrix_details::linalg_real_t< T >;

        svd_factorization() noexcept = default;
        explicit svd_factorization( matrix_type const& a, std::size_t max_sweeps = 64 ) noexcept : m_( a.row() ), n_( a.col() )
        {
            factor( a, max_sweeps );
        }

        [[nodiscard]] linalg_status status() const noexcept { return status_; }
        [[nodiscard]] bool ok() const noexcept { return status_ == linalg_status::ok; }
        [[nodiscard]] std::size_t sweeps() const noexcept { return sweeps_; }
        [[nodiscard]] matrix_type const& u() const noexcept { return u_; }
        [[nodiscard]] matrix_type const& v() const noexcept { return v_; }
        [[nodiscard]] std::vector< real_type > const& s() const noexcept { return s_; }

        // the relative cutoff: rtol < 0 selects the default max(m, n)·ε (numpy 2 pinv, D-028)
        [[nodiscard]] real_type cutoff( real_type rtol = real_type( -1 ) ) const noexcept
        {
            if ( rtol < real_type( 0 ) ) rtol = static_cast< real_type >( std::max( m_, n_ ) ) * std::numeric_limits< real_type >::epsilon();
            return s_.empty() ? real_type( 0 ) : rtol * s_[0];
        }
        // the number of s_i > rtol·s_1
        [[nodiscard]] std::size_t rank( real_type rtol = real_type( -1 ) ) const noexcept
        {
            real_type const tol = cutoff( rtol );
            return static_cast< std::size_t >( std::count_if( s_.begin(), s_.end(), [tol]( real_type x ) { return x > tol; } ) );
        }
        // A⁺ = V·diag(1/s_i)·Uᴴ (n×m) with every s_i ≤ rtol·s_1 zeroed; a status other than ok and an empty value
        // when the factorization is not ok
        [[nodiscard]] linalg_result< matrix_type > pinverse( real_type rtol = real_type( -1 ) ) const noexcept
        {
            if ( !ok() ) return { matrix_type{}, status_ };
            std::size_t const r = rank( rtol );
            matrix_type x( n_, m_ );
            std::fill( x.begin(), x.end(), T{ 0 } );
            for ( std::size_t i = 0; i != n_; ++i )
                for ( std::size_t c = 0; c != r; ++c )
                {
                    T const vi = v_[i][c] / s_[c];
                    T* const xi = x.data() + i * m_;
                    for ( std::size_t j = 0; j != m_; ++j ) xi[j] += vi * conj_( u_[j][c] );
                }
            if ( !matrix_details::all_finite( x.data(), x.size() ) ) return { matrix_type{}, linalg_status::nonfinite };
            return { std::move( x ), linalg_status::ok };
        }

    private:
        static T conj_( T const& x ) noexcept
        {
            if constexpr ( matrix_private::is_std_complex_v< T > ) return std::conj( x );
            else return x;
        }

        // makes the k-th column of the column-major M×k array `q` (columns 0..k-1 orthonormal) a unit vector
        // orthogonal to them: the first unit vector e_t whose residual after two Gram–Schmidt passes exceeds 1/2
        static void complete_( std::vector< T >& q, std::size_t M, std::size_t k ) noexcept
        {
            T* const x = q.data() + k * M;
            for ( std::size_t t = 0; t != M; ++t )
            {
                std::fill( x, x + M, T{ 0 } );
                x[t] = T{ 1 };
                for ( int pass = 0; pass != 2; ++pass )
                    for ( std::size_t c = 0; c != k; ++c )
                    {
                        T const* const qc = q.data() + c * M;
                        T d{ 0 };
                        for ( std::size_t i = 0; i != M; ++i ) d += conj_( qc[i] ) * x[i];
                        for ( std::size_t i = 0; i != M; ++i ) x[i] -= d * qc[i];
                    }
                real_type nrm{ 0 };
                for ( std::size_t i = 0; i != M; ++i ) nrm += std::norm( x[i] );
                nrm = std::sqrt( nrm );
                if ( nrm > real_type( 0.5 ) )
                {
                    for ( std::size_t i = 0; i != M; ++i ) x[i] /= nrm;
                    return;
                }
            }
        }

        void factor( matrix_type const& a, std::size_t max_sweeps ) noexcept
        {
            using R = real_type;
            T const* const ad = a.data();
            if ( !matrix_details::all_finite( ad, m_ * n_ ) ) { status_ = linalg_status::nonfinite; return; }
            bool const tall = m_ >= n_;
            std::size_t const M = tall ? m_ : n_; // B is M×N, M ≥ N
            std::size_t const N = tall ? n_ : m_;
            R scale{ 0 };
            for ( std::size_t i = 0; i != m_ * n_; ++i ) scale = std::max( scale, R( std::abs( ad[i] ) ) );
            R const inv_scale = scale > R( 0 ) ? R( 1 ) / scale : R( 1 );
            // column j of B at b[j·M ..), column j of W at w[j·N ..); B·W stays the rotated B and W is unitary
            std::vector< T > b( M * N ), w( N * N, T{ 0 } );
            for ( std::size_t i = 0; i != m_; ++i )
                for ( std::size_t j = 0; j != n_; ++j )
                {
                    T const x = ad[i * n_ + j] * inv_scale;
                    if ( tall ) b[j * M + i] = x;
                    else b[i * M + j] = conj_( x );
                }
            for ( std::size_t j = 0; j != N; ++j ) w[j * N + j] = T{ 1 };

            R const eps = std::numeric_limits< R >::epsilon();
            bool converged = N < 2;
            while ( !converged && sweeps_ < max_sweeps )
            {
                ++sweeps_;
                bool rotated = false;
                for ( std::size_t p = 0; p + 1 < N; ++p )
                    for ( std::size_t q = p + 1; q != N; ++q )
                    {
                        T* const bp = b.data() + p * M;
                        T* const bq = b.data() + q * M;
                        R alpha{ 0 }, beta{ 0 };
                        T gamma{ 0 };
                        for ( std::size_t i = 0; i != M; ++i )
                        {
                            alpha += std::norm( bp[i] );
                            beta += std::norm( bq[i] );
                            gamma += conj_( bp[i] ) * bq[i];
                        }
                        R const g = std::abs( gamma );
                        if ( g == R( 0 ) || g <= eps * std::sqrt( alpha ) * std::sqrt( beta ) ) continue;
                        rotated = true;
                        // with e = γ/|γ| the pair (b_p, ē·b_q) has the real inner product |γ|: rotate it by (c, s)
                        R const zeta = ( beta - alpha ) / ( R( 2 ) * g );
                        R const t = ( zeta >= R( 0 ) ? R( 1 ) : R( -1 ) ) / ( std::abs( zeta ) + std::hypot( R( 1 ), zeta ) );
                        R const c = R( 1 ) / std::sqrt( R( 1 ) + t * t );
                        R const s = c * t;
                        T const ebar = conj_( gamma / g );
                        auto const rotate = [c, s, ebar]( T* x, T* y, std::size_t len ) noexcept
                        {
                            for ( std::size_t i = 0; i != len; ++i )
                            {
                                T const xi = x[i];
                                T const yi = ebar * y[i];
                                x[i] = c * xi - s * yi;
                                y[i] = s * xi + c * yi;
                            }
                        };
                        rotate( bp, bq, M );
                        rotate( w.data() + p * N, w.data() + q * N, N );
                    }
                converged = !rotated;
            }
            status_ = converged ? linalg_status::ok : linalg_status::not_converged;

            // singular values, descending order, then normalized left vectors of B
            std::vector< R > norms( N );
            for ( std::size_t j = 0; j != N; ++j )
            {
                R acc{ 0 };
                for ( std::size_t i = 0; i != M; ++i ) acc += std::norm( b[j * M + i] );
                norms[j] = std::sqrt( acc );
            }
            std::vector< std::size_t > idx( N );
            for ( std::size_t j = 0; j != N; ++j ) idx[j] = j;
            std::stable_sort( idx.begin(), idx.end(), [&norms]( std::size_t x, std::size_t y ) { return norms[x] > norms[y]; } );
            std::vector< T > ub( M * N ), vb( N * N );
            s_.resize( N );
            for ( std::size_t r = 0; r != N; ++r )
            {
                std::size_t const j = idx[r];
                s_[r] = norms[j] * scale;
                std::copy( w.begin() + j * N, w.begin() + j * N + N, vb.begin() + r * N );
                if ( norms[j] > R( 0 ) )
                    for ( std::size_t i = 0; i != M; ++i ) ub[r * M + i] = b[j * M + i] / norms[j];
            }
            for ( std::size_t r = 0; r != N; ++r ) // exact zeros sort last
                if ( norms[idx[r]] == R( 0 ) ) complete_( ub, M, r );

            // A = B (tall): U = U_B, V = W; A = Bᴴ (wide): U = W, V = U_B
            auto const fill = []( matrix_type& out, std::vector< T > const& src, std::size_t rows, std::size_t k )
            {
                out.resize( rows, k );
                for ( std::size_t i = 0; i != rows; ++i )
                    for ( std::size_t c = 0; c != k; ++c ) out[i][c] = src[c * rows + i];
            };
            fill( u_, tall ? ub : vb, m_, N );
            fill( v_, tall ? vb : ub, n_, N );
        }

        matrix_type u_;
        matrix_type v_;
        std::vector< real_type > s_;
        linalg_status status_ = linalg_status::ok;
        std::size_t sweeps_ = 0;
        std::size_t m_ = 0;
        std::size_t n_ = 0;
    };

    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] svd_factorization< T, A > svd_factor( matrix< T, A > const& a, std::size_t max_sweeps = 64 ) noexcept
    {
        return svd_factorization< T, A >( a, max_sweeps );
    }

    // The one pseudoinverse (S7-R3, D-026, D-028): out = A⁺ (n×m) with s_i ≤ rtol·s_1 zeroed, rtol < 0 selecting
    // max(m, n)·ε; out is assigned only when the status is ok.
    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] linalg_status pinverse( matrix< T, A > const& a, matrix< T, A >& out, matrix_details::linalg_real_t< T > rtol = -1 ) noexcept
    {
        auto r = svd_factor( a ).pinverse( rtol );
        if ( r.ok() ) out = std::move( r.value );
        return r.status;
    }

    // value-returning forms: an empty 0×0 matrix when the SVD is not ok (D-026)
    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] matrix< T, A > pinverse( matrix< T, A > const& a, matrix_details::linalg_real_t< T > rtol = -1 ) noexcept
    {
        matrix< T, A > out;
        (void)pinverse( a, out, rtol ); // the status is reported as the empty result
        return out;
    }
    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] matrix< T, A > pinv( matrix< T, A > const& a, matrix_details::linalg_real_t< T > rtol = -1 ) noexcept
    {
        return pinverse( a, rtol );
    }
    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] matrix< T, A > svd_inverse( matrix< T, A > const& a ) noexcept
    {
        return pinverse( a );
    }

    // Legacy SVD: max_its is the sweep limit. On success returns 0 with u m×k, w k×k diagonal (the singular values,
    // descending) and v n×k, k = min(m, n), so A = u·w·vᴴ; otherwise returns 1 with u, w and v unchanged.
    template < typename T, typename A_ = std::allocator< T > > requires linalg_element< T >
    [[nodiscard]] std::uint_least64_t
    singular_value_decomposition( matrix<T,A_> const& A,
                                  matrix<T,A_>& u,
                                  matrix<T,A_>& w,
                                  matrix<T,A_>& v,
                                  std::uint_least64_t const max_its = 64 ) noexcept
    {
        auto const f = svd_factor( A, static_cast< std::size_t >( max_its ) );
        if ( !f.ok() )
            return 1;
        std::size_t const k = f.s().size();
        matrix<T,A_> d( k, k );
        std::fill( d.begin(), d.end(), T{ 0 } );
        for ( std::size_t i = 0; i != k; ++i ) d[i][i] = T( f.s()[i] );
        u = f.u();
        v = f.v();
        w = std::move( d );
        return 0;
    }

    // (u, w, v) as above, or nullopt when the SVD is not ok
    template< typename T, Allocator A> requires linalg_element< T >
    [[nodiscard]] std::optional< std::tuple<matrix<T, A>, matrix<T, A>, matrix<T, A>> >
    singular_value_decomposition( matrix<T,A> const& a ) noexcept
    {
        if ( matrix<T, A> u, w, v; singular_value_decomposition( a, u, w, v ) ) // fail
            return {};
        else // success
            return std::make_tuple( std::move( u ), std::move( w ), std::move( v ) );
    }

    template< typename T, Allocator A> requires linalg_element< T >
    [[nodiscard]] auto svd( matrix<T,A> const& a ) noexcept
    {
        return singular_value_decomposition( a );
    }

    // S6-R3 (F17, D-024): random matrices from a caller-owned std::uniform_random_bit_generator. Elements are
    // drawn in row-major order from the engine alone (no global state). Floating parts lie in the open interval
    // (0, 1): a draw equal to 0 or 1 is redrawn. Integral elements come from std::uniform_int_distribution over
    // [numeric_limits<T>::min(), max()]; types the distribution does not accept (bool and the character types)
    // are drawn as int or unsigned (long long or unsigned long long when wider) and narrowed.
    namespace matrix_details
    {
        template< typename T >
        inline constexpr bool random_dependent_false_v = false;

        template< typename T >
        inline constexpr bool uniform_int_type_v =
            std::is_same_v< T, short > || std::is_same_v< T, int > || std::is_same_v< T, long > || std::is_same_v< T, long long > ||
            std::is_same_v< T, unsigned short > || std::is_same_v< T, unsigned int > || std::is_same_v< T, unsigned long > ||
            std::is_same_v< T, unsigned long long >;

        template< typename T >
        using uniform_draw_t = std::conditional_t< uniform_int_type_v< T >, T,
                               std::conditional_t< ( sizeof( T ) <= sizeof( int ) ),
                                                   std::conditional_t< std::is_signed_v< T >, int, unsigned >,
                                                   std::conditional_t< std::is_signed_v< T >, long long, unsigned long long > > >;

        template< std::floating_point X, typename G >
        X real_open_unit( G& g ) noexcept
        {
            std::uniform_real_distribution< X > dist{ X{ 0 }, X{ 1 } };
            for ( ;; )
            {
                X const x = dist( g );
                if ( X{ 0 } < x && x < X{ 1 } ) return x;
            }
        }

        // a seed for the legacy overloads: the nonzero seed as given, else a steady_clock count mixed with an
        // object address (no hardware random device: it may throw)
        inline std::uint_least64_t legacy_seed( unsigned int seed, void const* address ) noexcept
        {
            if ( 0 != seed ) return seed;
            auto const ticks = static_cast< std::uint_least64_t >( std::chrono::steady_clock::now().time_since_epoch().count() );
            auto const where = static_cast< std::uint_least64_t >( reinterpret_cast< std::uintptr_t >( address ) );
            return ticks ^ ( where * 0x9E3779B97F4A7C15ULL );
        }
    }

    template < typename T = double, typename A = std::allocator< T >, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > random_impl( const std::uint_least64_t r, const std::uint_least64_t c, G& g ) noexcept
    {
        matrix< T, A > ans{ r, c };
        if constexpr ( std::is_floating_point_v< T > )
        {
            for ( auto& x : ans ) x = matrix_details::real_open_unit< T >( g );
        }
        else if constexpr ( matrix_private::is_std_complex_v< T > )
        {
            using X = typename T::value_type;
            for ( auto& x : ans )
            {
                X const re = matrix_details::real_open_unit< X >( g );
                X const im = matrix_details::real_open_unit< X >( g );
                x = T{ re, im };
            }
        }
        else if constexpr ( std::is_integral_v< T > )
        {
            using D = matrix_details::uniform_draw_t< T >;
            std::uniform_int_distribution< D > dist{ static_cast< D >( std::numeric_limits< T >::min() ), static_cast< D >( std::numeric_limits< T >::max() ) };
            for ( auto& x : ans ) x = static_cast< T >( dist( g ) );
        }
        else
        {
            static_assert( matrix_details::random_dependent_false_v< T >, "feng::random/rand: the element type must be a floating-point, std::complex or integral type" );
        }
        return ans;
    }

    // engine overloads
    template < typename T = double, typename A = std::allocator< T >, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > random( std::integral auto r, std::integral auto c, G&& g ) noexcept
    {
        return random_impl< T, A >( r, c, g );
    }
    template < typename T = double, typename A = std::allocator< T >, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > random( std::integral auto n, G&& g ) noexcept
    {
        return random_impl< T, A >( n, n, g );
    }
    template < typename T = double, typename A = std::allocator< T >, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > rand( const std::uint_least64_t r, const std::uint_least64_t c, G&& g ) noexcept
    {
        return random_impl< T, A >( r, c, g );
    }
    template < typename T = double, typename A = std::allocator< T >, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > rand( const std::uint_least64_t n, G&& g ) noexcept
    {
        return random_impl< T, A >( n, n, g );
    }
    template < typename T, Allocator A, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > rand_like( matrix< T, A > const& mat, G&& g ) noexcept
    {
        auto const [row, col] = mat.shape();
        return random_impl< T, A >( row, col, g );
    }
    template < typename T, Allocator A, typename G >
    requires std::uniform_random_bit_generator< std::remove_cvref_t< G > >
    [[nodiscard]] matrix< T, A > random_like( matrix< T, A > const& mat, G&& g ) noexcept
    {
        auto const [row, col] = mat.shape();
        return random_impl< T, A >( row, col, g );
    }

    // legacy overloads: a local std::mt19937_64 seeded per matrix_details::legacy_seed
    template < typename T = double, typename A = std::allocator< T > >
    [[nodiscard]] matrix< T, A > rand( const std::uint_least64_t r, const std::uint_least64_t c, unsigned int seed = 0 ) noexcept
    {
        std::mt19937_64 engine{ 0 };
        engine.seed( matrix_details::legacy_seed( seed, &engine ) );
        return random_impl< T, A >( r, c, engine );
    }
    template < typename T = double, typename A = std::allocator< T > >
    [[nodiscard]] matrix< T, A > rand( const std::uint_least64_t n ) noexcept
    {
        return rand< T, A >( n, n );
    }

    template < typename T = double, typename A = std::allocator< T > >
    [[nodiscard]] matrix< T, A > random( std::integral auto r, std::integral auto c ) noexcept
    {
        return rand< T, A >( r, c );
    }
    template < typename T = double, typename A = std::allocator< T > >
    [[nodiscard]] matrix< T, A > random( const std::integral auto n ) noexcept
    {
        return rand< T, A >( n );
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > rand_like( matrix<T, A> const& mat ) noexcept
    {
        auto const[row, col] = mat.shape();
        return random<T, A>( row, col );
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > random_like( matrix<T, A> const& mat ) noexcept
    {
        return rand_like<T,A>(mat);
    }
    template < typename T, Allocator A> //pytorch style; uniform in (0, 1), an alias of rand_like
    [[nodiscard]] matrix< T, A > randn_like( matrix<T, A> const& mat ) noexcept
    {
        return rand_like<T,A>(mat);
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A >
    repmat( const matrix< T, A >& m, const std::uint_least64_t r, const std::uint_least64_t c ) noexcept
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
               typename A = std::allocator< std::remove_cvref_t< typename std::iterator_traits< Itor1 >::value_type > >>
    [[nodiscard]] matrix< typename std::iterator_traits< Itor1 >::value_type, A > toeplitz( Itor1 i1_, Itor1 _i1, Itor2 i2_, Itor2 _i2 ) noexcept
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

               typename A    = std::allocator< std::remove_cvref_t< typename std::iterator_traits< Itor >::value_type > >>
    [[nodiscard]] matrix< typename std::iterator_traits< Itor >::value_type, A > toeplitz( Itor i_, Itor _i ) noexcept
    {
        return toeplitz( i_, _i, i_, _i );
    }
    template < typename Matrix >
    [[nodiscard]] typename Matrix::value_type tr( const Matrix& m ) noexcept
    {
        return m.tr();
    }
    template < typename Matrix >
    [[nodiscard]] Matrix transpose( const Matrix& m ) noexcept
    {
        return m.transpose();
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > tril( const matrix< T, A >& m ) noexcept
    {
        matrix< T, A > ans{ m.row(), m.col() };

        for ( std::uint_least64_t i = 0; i != m.col(); ++i )
            std::copy( m.lower_diag_cbegin( i ), m.lower_diag_cend( i ), ans.lower_diag_begin( i ) );

        return ans;
    }
    template < typename T, Allocator A>
    [[nodiscard]] matrix< T, A > triu( const matrix< T, A >& m ) noexcept
    {
        matrix< T, A > ans{ m.row(), m.col() };

        for ( std::uint_least64_t i = 0; i != m.col(); ++i )
            std::copy( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), ans.upper_diag_begin( i ) );

        return ans;
    }
    // S6-R5 (D-023): one template per operator and side. The result is matrix< scalar_result_t< T, S >, A rebound >:
    // T (and lhs's allocator, as a copy selects it) when the scalar's kind is not higher than T's, the scalar
    // converted to T, or to T's value type for complex T, first; otherwise the matrix is promoted to the result type.
    // Signed overflow and integer division by zero are preconditions; unsigned arithmetic wraps.
    namespace matrix_details
    {
        template< typename T, Allocator A, typename S, typename Op >
        promoted_matrix_t< scalar_result_t< T, S >, T, A > scalar_apply( matrix< T, A > const& m, S const& s, Op op ) noexcept
        {
            using R = scalar_result_t< T, S >;
            auto ans = promote< R >( m );
            auto const v = scalar_as< R >( s );
            std::transform( ans.begin(), ans.end(), ans.begin(), [&v, &op]( R const& x ) noexcept { return static_cast< R >( op( x, v ) ); } );
            return ans;
        }
    }

    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator+( const matrix< T, A >& lhs, const S& rhs ) noexcept
    {
        return matrix_details::scalar_apply( lhs, rhs, []( auto const& x, auto const& y ) noexcept { return x + y; } );
    }
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator+( const S& lhs, const matrix< T, A >& rhs ) noexcept
    {
        return matrix_details::scalar_apply( rhs, lhs, []( auto const& x, auto const& y ) noexcept { return y + x; } );
    }
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator-( const matrix< T, A >& lhs, const S& rhs ) noexcept
    {
        return matrix_details::scalar_apply( lhs, rhs, []( auto const& x, auto const& y ) noexcept { return x - y; } );
    }
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator-( const S& lhs, const matrix< T, A >& rhs ) noexcept
    {
        return matrix_details::scalar_apply( rhs, lhs, []( auto const& x, auto const& y ) noexcept { return y - x; } );
    }
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator*( const matrix< T, A >& lhs, const S& rhs ) noexcept
    {
        return matrix_details::scalar_apply( lhs, rhs, []( auto const& x, auto const& y ) noexcept { return x * y; } );
    }
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator*( const S& lhs, const matrix< T, A >& rhs ) noexcept
    {
        return matrix_details::scalar_apply( rhs, lhs, []( auto const& x, auto const& y ) noexcept { return y * x; } );
    }
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator/( const matrix< T, A >& lhs, const S& rhs ) noexcept
    {
        return matrix_details::scalar_apply( lhs, rhs, []( auto const& x, auto const& y ) noexcept { return x / y; } );
    }
    // s / m keeps its meaning, s times m.inverse(); only its result type follows D-023.
    template < typename T, Allocator A, typename S > requires matrix_details::scalar_operand_for< S, T >
    matrix_details::promoted_matrix_t< matrix_details::scalar_result_t< T, S >, T, A >
    operator/( const S& lhs, const matrix< T, A >& rhs ) noexcept
    {
        using R = matrix_details::scalar_result_t< T, S >;
        if constexpr ( std::same_as< R, T > )
            return rhs.inverse() * matrix_details::scalar_as< R >( lhs );
        else
            return matrix_details::promote< R >( rhs ).inverse() * matrix_details::scalar_as< R >( lhs );
    }
    template < typename T, Allocator A>
    matrix< T, A >
    operator||( const matrix< T, A >& lhs, const T& rhs ) noexcept
    {
        matrix< T, A > ans( lhs.row(), lhs.col() + 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.col_begin( lhs.col() ), ans.col_end( lhs.col() ), rhs );
        return ans;
    }
    template < typename T, Allocator A>
    matrix< T, A >
    operator||( const T& lhs, const matrix< T, A >& rhs ) noexcept
    {
        matrix< T, A > ans( rhs.row(), rhs.col() + 1 );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) + 1 );

        std::fill( ans.col_begin( 0 ), ans.col_end( 0 ), rhs );
        return ans;
    }
    template < typename T, Allocator A>
    matrix< T, A >
    operator&&( const matrix< T, A >& lhs, const T& rhs ) noexcept
    {
        matrix< T, A > ans( lhs.row() + 1, lhs.col() );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.row_begin( lhs.row() ), ans.row_end( lhs.row() ), rhs );
        return ans;
    }
    template < typename T, Allocator A>
    matrix< T, A >
    operator&&( const T& lhs, const matrix< T, A >& rhs ) noexcept
    {
        matrix< T, A > ans( rhs.row() + 1, rhs.col() );

        for ( std::uint_least64_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i + 1 ) );

        std::fill( ans.row_begin( 0 ), ans.row_end( 0 ), rhs );
        return ans;
    }
    template < typename T, Allocator A>
    matrix< T, A >
    operator^( const matrix< T, A >& lhs, std::uint_least64_t n ) noexcept
    {
        better_assert( lhs.row() == lhs.col() );
        auto const r = lhs.row();

        if ( 0 == n )
            return eye< T >( r, r );

        if ( 1 == n )
            return lhs;

        if ( n & 1 )
            return ( lhs ^ ( n - 1 ) ) * lhs;

        auto const& lhs_2 = lhs ^ ( n >> 1 );
        return lhs_2 * lhs_2;
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename T3, Allocator A3 >
    [[nodiscard]] int backward_substitution( const matrix< T1, A1 >& A,
                               matrix< T2, A2 >& x,
                               const matrix< T3, A3 >& b ) noexcept
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
    namespace iterative_private
    {
        // Shared set-up of cgs and bicgstab (S7-R5, D-029): the starting iterate is x when x is n×1 and nonzero,
        // else b (the legacy rule); the vectors are plain arrays of A's element type.
        template < typename T >
        struct krylov
        {
            std::size_t n;
            std::vector< T > a, b, x;

            template < typename M1, typename M2, typename M3 >
            krylov( M1 const& A, M2 const& x0, M3 const& b0 ) : n( A.row() ), a( n * n ), b( n ), x( n )
            {
                for ( std::size_t i = 0; i != n; ++i )
                {
                    for ( std::size_t j = 0; j != n; ++j ) a[i * n + j] = static_cast< T >( A[i][j] );
                    b[i] = static_cast< T >( b0[i][0] );
                }
                bool const use_x = x0.row() == n && x0.col() == 1 && std::any_of( x0.begin(), x0.end(), []( auto v ) { return v != decltype( v )( 0 ); } );
                for ( std::size_t i = 0; i != n; ++i ) x[i] = use_x ? static_cast< T >( x0[i][0] ) : b[i];
            }
            bool finite_input() const noexcept
            {
                return matrix_details::all_finite( a.data(), a.size() ) && matrix_details::all_finite( b.data(), b.size() );
            }
            void mul( std::vector< T > const& v, std::vector< T >& out ) const noexcept // out = A·v
            {
                for ( std::size_t i = 0; i != n; ++i )
                {
                    T s{ 0 };
                    for ( std::size_t j = 0; j != n; ++j ) s += a[i * n + j] * v[j];
                    out[i] = s;
                }
            }
            static T dot( std::vector< T > const& u, std::vector< T > const& v ) noexcept
            {
                T s{ 0 };
                for ( std::size_t i = 0; i != u.size(); ++i ) s += u[i] * v[i];
                return s;
            }
            static T norm( std::vector< T > const& v ) noexcept
            {
                T s{ 0 };
                for ( auto const e : v ) s += e * e;
                return std::sqrt( s );
            }
            T residual( std::vector< T > const& xv, std::vector< T >& tmp ) const noexcept // ‖b − A·xv‖₂
            {
                mul( xv, tmp );
                for ( std::size_t i = 0; i != n; ++i ) tmp[i] = b[i] - tmp[i];
                return norm( tmp );
            }
            static bool usable( T v ) noexcept { return v != T( 0 ) && std::isfinite( v ); }
            template < typename M >
            void store( M& out, std::vector< T > const& v ) const noexcept
            {
                if ( out.row() != n || out.col() != 1 ) out.resize( n, 1 );
                for ( std::size_t i = 0; i != n; ++i ) out[i][0] = static_cast< typename M::value_type >( v[i] );
            }
        };
    }//namespace iterative_private

    // S7-R5 (D-029): BiCGSTAB for A·x = b. Returns 0 when ‖b − A·x‖₂ ≤ eps·‖b‖₂ (a zero b gives x = 0), and 1 for a
    // nonfinite A or b (x unchanged), a breakdown (a zero or nonfinite denominator) or max_loops iterations without
    // convergence; x then holds the last finite iterate.
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename T3, Allocator A3 >
    [[nodiscard]] int biconjugate_gradient_stabilized_method( const matrix< T1, A1 >& A,
            matrix< T2, A2 >& x,
            const matrix< T3, A3 >& b,
            const std::uint_least64_t max_loops = 100,
            const T1 eps                = 1.0e-10 ) noexcept
    {
        typedef T1 value_type;
        better_assert( A.row() == A.col(), "bicgstab: expecting a square A, but got ", A.row(), "x", A.col() );
        better_assert( A.row() == b.row() && b.col() == 1, "bicgstab: expecting a column b with ", A.row(), " rows, but got ", b.row(), "x", b.col() );
        iterative_private::krylov< value_type > k( A, x, b );
        if ( !k.finite_input() ) return 1;
        std::size_t const n = k.n;
        value_type const bnorm = k.norm( k.b );
        if ( bnorm == value_type( 0 ) )
        {
            std::fill( k.x.begin(), k.x.end(), value_type( 0 ) );
            k.store( x, k.x );
            return 0;
        }
        value_type const tol = eps * bnorm;
        std::vector< value_type > r( n ), tmp( n ), p( n, 0 ), v( n, 0 ), s( n ), t( n ), xn( n );
        if ( !std::isfinite( k.residual( k.x, r ) ) ) return 1;
        if ( k.norm( r ) <= tol ) { k.store( x, k.x ); return 0; }
        std::vector< value_type > const r_hat = r;
        value_type rho_old( 1 ), alpha( 1 ), omega( 1 );
        int result = 1;
        for ( std::uint_least64_t loops = 0; loops != max_loops; ++loops )
        {
            value_type const rho = k.dot( r_hat, r );
            if ( !k.usable( rho ) ) break;
            if ( loops == 0 )
                p = r;
            else
            {
                value_type const beta = ( rho / rho_old ) * ( alpha / omega );
                if ( !std::isfinite( beta ) ) break;
                for ( std::size_t i = 0; i != n; ++i ) p[i] = r[i] + beta * ( p[i] - omega * v[i] );
            }
            k.mul( p, v );
            value_type const den = k.dot( r_hat, v );
            if ( !k.usable( den ) ) break;
            alpha = rho / den;
            if ( !std::isfinite( alpha ) ) break;
            for ( std::size_t i = 0; i != n; ++i ) { s[i] = r[i] - alpha * v[i]; xn[i] = k.x[i] + alpha * p[i]; }
            if ( !matrix_details::all_finite( xn.data(), n ) ) break;
            if ( k.residual( xn, tmp ) <= tol ) { k.x = xn; result = 0; break; }
            k.mul( s, t );
            value_type const tt = k.dot( t, t );
            if ( !k.usable( tt ) ) { k.x = xn; break; }
            omega = k.dot( t, s ) / tt;
            if ( !k.usable( omega ) ) { k.x = xn; break; }
            for ( std::size_t i = 0; i != n; ++i ) { xn[i] += omega * s[i]; r[i] = s[i] - omega * t[i]; }
            if ( !matrix_details::all_finite( xn.data(), n ) ) break;
            k.x = xn;
            if ( k.residual( k.x, tmp ) <= tol ) { result = 0; break; }
            rho_old = rho;
        }
        k.store( x, k.x );
        return result;
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename T3, Allocator A3 >
    [[nodiscard]] int bicgstab( const matrix< T1, A1 >& A,
                  matrix< T2, A2 >& x,
                  const matrix< T3, A3 >& b,
                  const std::uint_least64_t max_loops = 100,
                  const T1 eps                = 1.0e-10 ) noexcept
    {
        return biconjugate_gradient_stabilized_method( A, x, b, max_loops, eps );
    }
    // S7-R4 (F13, D-026, D-027): Cholesky factorization A = L·Lᴴ of a real symmetric or complex Hermitian
    // positive definite A. status() is ok or not_positive_definite; not_positive_definite also covers a
    // non-symmetric (non-Hermitian) A, |a_ij − conj(a_ji)| > n·ε·‖A‖∞, a nonfinite input and a singular or
    // indefinite A: a pivot d_k = a_kk − Σ|l_kj|² that is not > n·ε·max_i |a_ii| (the D-027 scale). The factor is
    // computed from the lower triangle; l() is lower triangular with a real positive diagonal when ok and an
    // empty 0×0 matrix otherwise (never NaN).
    template < typename T, Allocator A_ = std::allocator< T > > requires linalg_element< T >
    class cholesky_factorization
    {
    public:
        using matrix_type = matrix< T, A_ >;
        using real_type = matrix_details::linalg_real_t< T >;

        cholesky_factorization() noexcept = default;
        explicit cholesky_factorization( matrix_type const& a ) noexcept
        {
            better_assert( a.row() == a.col(), "feng::cholesky_factor: expecting a square matrix, but got ", a.row(), "x", a.col() );
            factor( a );
        }

        [[nodiscard]] linalg_status status() const noexcept { return status_; }
        [[nodiscard]] bool ok() const noexcept { return status_ == linalg_status::ok; }
        [[nodiscard]] matrix_type const& l() const noexcept { return l_; }

    private:
        static T conj_( T const& x ) noexcept
        {
            if constexpr ( matrix_private::is_std_complex_v< T > ) return std::conj( x );
            else return x;
        }

        void factor( matrix_type const& a ) noexcept
        {
            using R = real_type;
            std::size_t const n = a.row();
            T const* const ad = a.data();
            status_ = linalg_status::not_positive_definite;
            if ( !matrix_details::all_finite( ad, n * n ) ) return;
            R norm_inf{ 0 }, max_diag{ 0 };
            for ( std::size_t i = 0; i != n; ++i )
            {
                R s{ 0 };
                for ( std::size_t j = 0; j != n; ++j ) s += std::abs( ad[i * n + j] );
                norm_inf = std::max( norm_inf, s );
                max_diag = std::max( max_diag, R( std::abs( ad[i * n + i] ) ) );
            }
            R const eps = std::numeric_limits< R >::epsilon();
            R const sym_tol = static_cast< R >( n ) * eps * norm_inf;
            for ( std::size_t i = 0; i != n; ++i )
                for ( std::size_t j = i; j != n; ++j )
                    if ( std::abs( ad[i * n + j] - conj_( ad[j * n + i] ) ) > sym_tol ) return;
            R const pivot_tol = static_cast< R >( n ) * eps * max_diag;
            matrix_type l( a );
            T* const ld = l.data();
            std::fill( ld, ld + n * n, T{ 0 } );
            for ( std::size_t j = 0; j != n; ++j )
            {
                R d = std::real( ad[j * n + j] );
                for ( std::size_t k = 0; k != j; ++k ) d -= std::norm( ld[j * n + k] );
                if ( !( d > pivot_tol ) || !std::isfinite( d ) ) return;
                R const ljj = std::sqrt( d );
                ld[j * n + j] = T( ljj );
                for ( std::size_t i = j + 1; i != n; ++i )
                {
                    T s = ad[i * n + j];
                    for ( std::size_t k = 0; k != j; ++k ) s -= ld[i * n + k] * conj_( ld[j * n + k] );
                    ld[i * n + j] = s / ljj;
                }
            }
            if ( !matrix_details::all_finite( ld, n * n ) ) return;
            l_ = std::move( l );
            status_ = linalg_status::ok;
        }

        matrix_type l_;
        linalg_status status_ = linalg_status::ok;
    };

    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] cholesky_factorization< T, A > cholesky_factor( matrix< T, A > const& a ) noexcept
    {
        return cholesky_factorization< T, A >( a );
    }

    // Legacy Cholesky: 0 with a = L (A = L·Lᴴ), or 1 with a unchanged when cholesky_factor is not ok (S7-R4).
    template < typename Matrix1, typename Matrix2 > requires linalg_element< typename Matrix1::value_type >
    [[nodiscard]] int cholesky_decomposition( const Matrix1& m, Matrix2& a ) noexcept
    {
        better_assert( m.row() == m.col(), "cholesky_decomposition: expecting a square matrix, but got ", m.row(), "x", m.col() );
        auto const f = cholesky_factor( m );
        if ( !f.ok() )
            return 1;
        a = f.l();
        return 0;
    }
    // S7-R5 (D-029): conjugate gradient squared for A·x = b; the same return and x rules as bicgstab.
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename T3, Allocator A3 >
    [[nodiscard]] int conjugate_gradient_squared( const matrix< T1, A1 >& A,
                                    matrix< T2, A2 >& x,
                                    const matrix< T3, A3 >& b,
                                    const std::uint_least64_t max_loops = 100,
                                    const T1 eps                = 1.0e-10 ) noexcept
    {
        typedef T1 value_type;
        better_assert( A.row() == A.col(), "cgs: expecting a square A, but got ", A.row(), "x", A.col() );
        better_assert( A.row() == b.row() && b.col() == 1, "cgs: expecting a column b with ", A.row(), " rows, but got ", b.row(), "x", b.col() );
        iterative_private::krylov< value_type > k( A, x, b );
        if ( !k.finite_input() ) return 1;
        std::size_t const n = k.n;
        value_type const bnorm = k.norm( k.b );
        if ( bnorm == value_type( 0 ) )
        {
            std::fill( k.x.begin(), k.x.end(), value_type( 0 ) );
            k.store( x, k.x );
            return 0;
        }
        value_type const tol = eps * bnorm;
        std::vector< value_type > r( n ), tmp( n ), u( n ), p( n ), q( n ), uq( n ), v( n ), xn( n );
        if ( !std::isfinite( k.residual( k.x, r ) ) ) return 1;
        if ( k.norm( r ) <= tol ) { k.store( x, k.x ); return 0; }
        std::vector< value_type > const r_hat = r;
        value_type rho_old( 1 );
        int result = 1;
        for ( std::uint_least64_t loops = 0; loops != max_loops; ++loops )
        {
            value_type const rho = k.dot( r_hat, r );
            if ( !k.usable( rho ) ) break;
            if ( loops == 0 )
            {
                u = r;
                p = u;
            }
            else
            {
                value_type const beta = rho / rho_old;
                if ( !std::isfinite( beta ) ) break;
                for ( std::size_t i = 0; i != n; ++i )
                {
                    u[i] = r[i] + beta * q[i];
                    p[i] = u[i] + beta * ( q[i] + beta * p[i] );
                }
            }
            k.mul( p, v );
            value_type const den = k.dot( r_hat, v );
            if ( !k.usable( den ) ) break;
            value_type const alpha = rho / den;
            if ( !std::isfinite( alpha ) ) break;
            for ( std::size_t i = 0; i != n; ++i )
            {
                q[i] = u[i] - alpha * v[i];
                uq[i] = u[i] + q[i];
                xn[i] = k.x[i] + alpha * uq[i];
            }
            if ( !matrix_details::all_finite( xn.data(), n ) ) break;
            k.x = xn;
            k.mul( uq, tmp );
            for ( std::size_t i = 0; i != n; ++i ) r[i] -= alpha * tmp[i];
            if ( k.residual( k.x, tmp ) <= tol ) { result = 0; break; }
            rho_old = rho;
        }
        k.store( x, k.x );
        return result;
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename T3, Allocator A3 >
    [[nodiscard]] int cgs( const matrix< T1, A1 >& A,
             matrix< T2, A2 >& x,
             const matrix< T3, A3 >& b,
             const std::uint_least64_t max_loops = 100,
             const T1 eps                = 1.0e-10 ) noexcept
    {
        return conjugate_gradient_squared( A, x, b, max_loops, eps );
    }
    // Experimental (S7, D-029): not qualified; householder has no status and is not covered by the S7 tests.
    template < typename Matrix1, typename Matrix2, typename Matrix3 >
    void householder( const Matrix1& A, Matrix2& Q, Matrix3& D ) noexcept
    {
        typedef Matrix1 matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        better_assert( A.row() == A.col() );
        size_type const n     = A.row();
        value_type const zero = value_type( 0 );
        value_type const two  = value_type( 2 );
        Matrix1 const I       = eye< value_type >( n );
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
        typename Matrix::value_type norm( const Matrix& A ) noexcept
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
        void rotate( Matrix1& A, Matrix2& V, const std::uint_least64_t p, const std::uint_least64_t q ) noexcept
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
    // Experimental (S7, D-029): not qualified; eigen_jacobi has an unbounded rotation loop and no status.
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, std::begin( Lambda ), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_jacobi( A, V, Lambda.diag_begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    std::uint_least64_t eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) ) noexcept
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
    // Experimental (S7, D-029): not qualified; cyclic_eigen_jacobi ignores eps and reports no status.
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, std::uint_least64_t max_rot = 80, [[maybe_unused]] const T eps = T( 1.0e-10 ) ) noexcept
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
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
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, std::uint_least64_t const max_rot = 80, const T eps = T( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, std::uint_least64_t const max_rot = 80, const T eps = T( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, std::begin( Lambda ), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    std::uint_least64_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, std::uint_least64_t const max_rot = 80, const T_ eps = T_( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return cyclic_eigen_jacobi( A, V, Lambda.diag_begin(), max_rot, eps );
    }
    // Experimental (S7, D-029): not qualified; eigen_real_symmetric rests on householder and eigen_jacobi.
    template < typename Matrix1, typename Matrix2, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, std::begin( Lambda ), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-10 ) ) noexcept
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_real_symmetric( A, V, Lambda.diag_begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) ) noexcept
    {
        better_assert( A.row() == A.col() );
        Matrix1 D( A );
        Matrix1 Q( A );
        householder( A, Q, D );
        eigen_jacobi( D, V, o, eps );
        V = Q * V;
    }
    // Experimental (S7, D-029): not qualified; eigen_hermitian rests on eigen_real_symmetric.
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-20 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-20 ) ) noexcept
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, std::begin( Lambda ), eps );
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T, typename A_, typename T_ = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-20 ) ) noexcept
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_hermitian_impl( A, V, Lambda.diag_begin(), eps );
    }
    template < typename T1, Allocator A1, typename T2, Allocator A2, typename Otor, typename T = double >
    void eigen_hermitian_impl( const matrix< std::complex< T1 >, A1 >& A, matrix< std::complex< T2 >, A2 >& V, Otor o, const T eps = T( 1.0e-20 ) ) noexcept
    {
        better_assert( A.row() == A.col() );
        std::uint_least64_t const n = A.row();
        auto const A_       = real( A );
        auto const B_       = imag( A );
        auto const AA       = ( A_ || ( -B_ ) ) && ( B_ || A_ );
        matrix< T1, typename std::allocator_traits< A1 >::template rebind_alloc< T1 > > VV( n + n, n + n );
        matrix< T1, typename std::allocator_traits< A1 >::template rebind_alloc< T1 > > LL( n + n, n + n );
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
    // Experimental (S7, D-029): not qualified; eigen_power_iteration has an unbounded loop and no status.
    template < typename T, typename A_, typename O >
    T eigen_power_iteration( const matrix< T, A_ >& A, O output, const T eps = T( 1.0e-5 ) ) noexcept
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
    T eigen_power_iteration( const matrix< T, A_ >& A, const T eps = T( 1.0e-5 ) ) noexcept
    {
        matrix< T, A_ > b( A.col(), 1 );
        return eigen_power_iteration( A, b.begin(), eps );
    }
    template < typename T, typename A_, typename O >
    T eigen_power_iteration( const matrix< std::complex< T >, A_ >& A, O output, const T eps = T( 1.0e-5 ) ) noexcept
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
    T eigen_power_iteration( const matrix< std::complex< T >, A_ >& A, const T eps = T( 1.0e-5 ) ) noexcept
    {
        matrix< std::complex< T >, A_ > b( A.col(), 1 );
        return eigen_power_iteration( A, b.begin(), eps );
    }
    /*
    template < typename Matrix >
    [[nodiscard]] typename Matrix::value_type
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
    [[nodiscard]] typename Matrix::value_type
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
    template < Matrix Mat >
    [[nodiscard]] auto norm_1( Mat const& A ) noexcept
    {
        typedef typename Mat::value_type value_type;
        std::vector< value_type > m( A.col() );

        for ( std::uint_least64_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }

    template < typename T, typename A_ >
    [[nodiscard]] T norm_1( const matrix< std::complex< T >, A_ >& A ) noexcept
    {
        std::vector< T > m( A.col() );

        for ( std::uint_least64_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), T( 0 ), []( const T u, const std::complex< T >& v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }
    template < Matrix Mat >
    [[nodiscard]] auto norm_2( Mat const& A ) noexcept
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
    // S7-R5 (D-029): matrix exponential by scaling and squaring with the degree-13 Padé approximant (Higham 2005).
    // expm(0) = I and a diagonal A gives diag(exp(a_ii)) exactly (shortcut); the scaling 2^-s is applied with
    // std::ldexp; the Padé quotient F = (V − U)⁻¹(V + U) is solved with lu_factor; a nonfinite A (or a failed
    // Padé solve) gives an all-NaN matrix of A's shape. Real and complex elements.
    template < Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] Mat expm( Mat const& A ) noexcept
    {
        typedef Mat matrix_type;
        typedef typename matrix_type::value_type T;
        typedef typename expm_private::fix_complex_value_type< T >::value_type value_type;
        better_assert( A.row() == A.col(), "feng::expm: expecting a square matrix, but got ", A.row(), "x", A.col() );
        std::size_t const n = A.row();
        auto nan_result = [&A]()
        {
            matrix_type ans( A );
            value_type const q = std::numeric_limits< value_type >::quiet_NaN();
            if constexpr ( matrix_private::is_std_complex_v< T > ) std::fill( ans.begin(), ans.end(), T( q, q ) );
            else std::fill( ans.begin(), ans.end(), T( q ) );
            return ans;
        };
        if ( !matrix_details::all_finite( A.data(), n * n ) ) return nan_result();

        bool diagonal = true;
        for ( std::size_t i = 0; i != n && diagonal; ++i )
            for ( std::size_t j = 0; j != n; ++j )
                if ( i != j && A[i][j] != T( 0 ) ) { diagonal = false; break; }
        if ( diagonal )
        {
            matrix_type ans( A );
            std::fill( ans.begin(), ans.end(), T( 0 ) );
            for ( std::size_t i = 0; i != n; ++i ) ans[i][i] = std::exp( A[i][i] );
            return ans;
        }

        static const value_type theta13 = 5.371920351148152e+000;
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
        value_type norm_A( 0 ); // ‖A‖₁
        for ( std::size_t j = 0; j != n; ++j )
        {
            value_type sum( 0 );
            for ( std::size_t i = 0; i != n; ++i ) sum += std::abs( A[i][j] );
            norm_A = std::max( norm_A, sum );
        }
        value_type const ratio = norm_A / theta13;
        int const s = ratio > value_type( 1 ) ? static_cast< int >( std::ceil( std::log2( ratio ) ) ) : 0;
        matrix_type _A( A );
        if ( s )
            for ( auto& v : _A )
            {
                if constexpr ( matrix_private::is_std_complex_v< T > ) v = T( std::ldexp( v.real(), -s ), std::ldexp( v.imag(), -s ) );
                else v = std::ldexp( v, -s );
            }
        matrix_type I( A );
        std::fill( I.begin(), I.end(), T( 0 ) );
        for ( std::size_t i = 0; i != n; ++i ) I[i][i] = T( 1 );
        matrix_type const _A2 = _A * _A;
        matrix_type const _A4 = _A2 * _A2;
        matrix_type const _A6 = _A2 * _A4;
        matrix_type const U   = _A * ( _A6 * ( c[14] * _A6 + c[12] * _A4 + c[10] * _A2 ) + c[8] * _A6 + c[6] * _A4 + c[4] * _A2 + c[2] * I );
        matrix_type const V   = _A6 * ( c[13] * _A6 + c[11] * _A4 + c[9] * _A2 ) + c[7] * _A6 + c[5] * _A4 + c[3] * _A2 + c[1] * I;
        auto quotient = lu_factor( matrix_type( V - U ) ).solve( matrix_type( V + U ) );
        if ( !quotient.ok() ) return nan_result();
        matrix_type F = std::move( quotient.value );
        for ( int i = 0; i != s; ++i )
            F = F * F;
        return F;
    }

    // S7-R5 (D-026): exp(A) into out; nonfinite (out unchanged) for a nonfinite A or a nonfinite result, else ok
    template < typename T, Allocator A_ > requires linalg_element< T >
    [[nodiscard]] linalg_status expm( matrix< T, A_ > const& a, matrix< T, A_ >& out ) noexcept
    {
        if ( !matrix_details::all_finite( a.data(), a.size() ) ) return linalg_status::nonfinite;
        matrix< T, A_ > e = expm( a );
        if ( !matrix_details::all_finite( e.data(), e.size() ) ) return linalg_status::nonfinite;
        out = std::move( e );
        return linalg_status::ok;
    }
    // S8-R2, S8-R3 (F14, D-007, D-031): numpy.fft.fft2/ifft2 by a separable in-house FFT (radix-2 for power-of-two
    // lengths, Bluestein through a power-of-two convolution otherwise) and fftshift/ifftshift as numpy rolls.
    namespace matrix_details
    {
        template < typename T >
        struct fft_complex;
        template < std::integral T >
        struct fft_complex< T >
        {
            using type = std::complex< double >;
        };
        template < std::floating_point T >
        struct fft_complex< T >
        {
            using type = std::complex< T >;
        };
        template < typename T >
        struct fft_complex< std::complex< T > >
        {
            using type = std::complex< T >;
        };
        // D-031: float, double, long double -> complex of the same real type; complex<T> stays; integers -> complex<double>
        template < typename T >
        using fft_complex_t = typename fft_complex< T >::type;

        template < typename R >
        inline constexpr R fft_pi = static_cast< R >( 3.141592653589793238462643383279502884L );

        // twiddle table for a power-of-two length n and a sign: tw[j] = exp(sign 2 pi i j / n), j < n/2, each by std::polar
        template < typename R >
        void fft_twiddles( std::vector< std::complex< R > >& tw, std::size_t n, int sign )
        {
            tw.resize( n / 2 );
            R const step = static_cast< R >( sign ) * R( 2 ) * fft_pi< R > / static_cast< R >( n );
            for ( std::size_t j = 0; j != n / 2; ++j )
                tw[j] = std::polar( R( 1 ), step * static_cast< R >( j ) );
        }

        // in-place iterative radix-2 transform of a power-of-two length n with a precomputed twiddle table tw
        template < typename R >
        void fft_radix2_apply( std::complex< R >* data, std::size_t n, std::complex< R > const* tw ) noexcept
        {
            for ( std::size_t i = 1, j = 0; i < n; ++i )
            {
                std::size_t bit = n >> 1;
                for ( ; j & bit; bit >>= 1 )
                    j ^= bit;
                j ^= bit;
                if ( i < j ) std::swap( data[i], data[j] );
            }
            for ( std::size_t len = 2; len <= n; len <<= 1 )
            {
                std::size_t const half = len >> 1;
                std::size_t const stride = n / len;
                for ( std::size_t i = 0; i < n; i += len )
                    for ( std::size_t j = 0; j != half; ++j )
                    {
                        std::complex< R > const u = data[i + j];
                        std::complex< R > const v = data[i + j + half] * tw[j * stride];
                        data[i + j]        = u + v;
                        data[i + j + half] = u - v;
                    }
            }
        }

        // in-place iterative radix-2 transform of a power-of-two length n; sign -1 forward, +1 inverse (unscaled);
        // twiddles exp(sign 2 pi i j / n) computed per index with std::polar, no recurrence (per-call tables: the
        // reference path behind fft2_reference)
        template < typename R >
        void fft_radix2( std::complex< R >* data, std::size_t n, int sign ) noexcept
        {
            std::vector< std::complex< R > > tw;
            fft_twiddles( tw, n, sign );
            fft_radix2_apply( data, n, tw.data() );
        }

        // Bluestein chirp w_k = exp(sign i pi (k^2 mod 2n) / n), k < n
        template < typename R >
        void fft_chirp( std::vector< std::complex< R > >& w, std::size_t n, int sign )
        {
            w.resize( n );
            std::uint_least64_t const two_n = 2 * static_cast< std::uint_least64_t >( n );
            std::uint_least64_t q = 0; // k^2 mod 2n, exact in 64-bit integers: (k+1)^2 = k^2 + 2k + 1
            for ( std::size_t k = 0; k != n; ++k )
            {
                w[k] = std::polar( R( 1 ), static_cast< R >( sign ) * fft_pi< R > * static_cast< R >( q ) / static_cast< R >( n ) );
                q = ( q + 2 * static_cast< std::uint_least64_t >( k ) + 1 ) % two_n;
            }
        }

        // Bluestein: X_k = w_k sum_j (x_j w_j) conj(w_{k-j}), w_k = exp(sign i pi (k^2 mod 2n) / n), the
        // convolution done by radix-2 transforms of length M, the power of two >= 2n - 1 (reference path)
        template < typename R >
        void fft_bluestein( std::complex< R >* data, std::size_t n, int sign ) noexcept
        {
            std::size_t const m = std::bit_ceil( 2 * n - 1 );
            std::vector< std::complex< R > > w;
            fft_chirp( w, n, sign );
            std::vector< std::complex< R > > a( m ), b( m );
            for ( std::size_t k = 0; k != n; ++k )
                a[k] = data[k] * w[k];
            b[0] = std::conj( w[0] );
            for ( std::size_t k = 1; k != n; ++k )
                b[k] = b[m - k] = std::conj( w[k] );
            fft_radix2( a.data(), m, -1 );
            fft_radix2( b.data(), m, -1 );
            for ( std::size_t k = 0; k != m; ++k )
                a[k] *= b[k];
            fft_radix2( a.data(), m, +1 );
            R const inv_m = R( 1 ) / static_cast< R >( m ); // m is a power of two: exact
            for ( std::size_t k = 0; k != n; ++k )
                data[k] = w[k] * ( a[k] * inv_m );
        }

        // 1-D transform of a contiguous sequence of length n (reference path)
        template < typename R >
        void fft_1d( std::complex< R >* data, std::size_t n, int sign ) noexcept
        {
            if ( n < 2 ) return;
            if ( std::has_single_bit( n ) )
                fft_radix2( data, n, sign );
            else
                fft_bluestein( data, n, sign );
        }

        // S10-R3 (B-042, fft-plans): everything a length-n transform of one sign needs, built once per fft2 call
        // by the same expressions as fft_radix2 / fft_bluestein: the twiddle table (radix-2 n), or the chirp w, the
        // forward transform of b and the length-m tables for -1 and +1 (Bluestein n), plus the length-m scratch a
        template < typename R >
        struct fft_plan
        {
            std::size_t n = 0;
            std::size_t m = 0; // 0 for radix-2 (or n < 2), else the Bluestein convolution length
            std::vector< std::complex< R > > tw;     // radix-2: exp(sign 2 pi i j / n)
            std::vector< std::complex< R > > w;      // Bluestein chirp
            std::vector< std::complex< R > > b_hat;  // forward radix-2 transform of b
            std::vector< std::complex< R > > tw_fwd; // length-m tables, sign -1
            std::vector< std::complex< R > > tw_inv; // and +1
            std::vector< std::complex< R > > a;      // scratch

            fft_plan( std::size_t n_, int sign ) : n{ n_ }
            {
                if ( n < 2 ) return;
                if ( std::has_single_bit( n ) )
                {
                    fft_twiddles( tw, n, sign );
                    return;
                }
                m = std::bit_ceil( 2 * n - 1 );
                fft_chirp( w, n, sign );
                fft_twiddles( tw_fwd, m, -1 );
                fft_twiddles( tw_inv, m, +1 );
                b_hat.assign( m, std::complex< R >{} );
                b_hat[0] = std::conj( w[0] );
                for ( std::size_t k = 1; k != n; ++k )
                    b_hat[k] = b_hat[m - k] = std::conj( w[k] );
                fft_radix2_apply( b_hat.data(), m, tw_fwd.data() );
                a.resize( m );
            }

            void operator()( std::complex< R >* data ) noexcept
            {
                if ( n < 2 ) return;
                if ( m == 0 )
                {
                    fft_radix2_apply( data, n, tw.data() );
                    return;
                }
                for ( std::size_t k = 0; k != n; ++k )
                    a[k] = data[k] * w[k];
                std::fill( a.begin() + static_cast< std::ptrdiff_t >( n ), a.end(), std::complex< R >{} );
                fft_radix2_apply( a.data(), m, tw_fwd.data() );
                for ( std::size_t k = 0; k != m; ++k )
                    a[k] *= b_hat[k];
                fft_radix2_apply( a.data(), m, tw_inv.data() );
                R const inv_m = R( 1 ) / static_cast< R >( m ); // m is a power of two: exact
                for ( std::size_t k = 0; k != n; ++k )
                    data[k] = w[k] * ( a[k] * inv_m );
            }
        };

        // the input converted to the complex result type
        template < Matrix Mat >
        auto fft2_load( Mat const& x )
        {
            using complex_type = fft_complex_t< typename Mat::value_type >;
            using real_type    = typename complex_type::value_type;
            std::size_t const r = x.row();
            std::size_t const c = x.col();
            matrix< complex_type > X( r, c );
            if ( r == 0 || c == 0 ) return X;
            for ( std::size_t i = 0; i != r; ++i )
                for ( std::size_t j = 0; j != c; ++j )
                {
                    auto const& v = x[i][j];
                    if constexpr ( std::is_same_v< typename Mat::value_type, complex_type > )
                        X[i][j] = v;
                    else
                        X[i][j] = complex_type{ static_cast< real_type >( v ), real_type( 0 ) };
                }
            return X;
        }

        // ifft (sign +1) divides by R*C
        template < typename C >
        void fft2_scale( matrix< C >& X, int sign ) noexcept
        {
            using real_type = typename C::value_type;
            if ( sign > 0 )
            {
                real_type const n = static_cast< real_type >( X.row() ) * static_cast< real_type >( X.col() );
                for ( auto& v : X ) v /= n;
            }
        }

        // 2-D transform: rows in place, then each column gathered, transformed and scattered, through one plan per
        // length (the row plan reused for the columns when r == c)
        template < Matrix Mat >
        auto fft2( Mat const& x, int sign ) noexcept
        {
            using complex_type = fft_complex_t< typename Mat::value_type >;
            using real_type    = typename complex_type::value_type;
            std::size_t const r = x.row();
            std::size_t const c = x.col();
            auto X = fft2_load( x );
            if ( r == 0 || c == 0 ) return X;
            fft_plan< real_type > row_plan( c, sign );
            for ( std::size_t i = 0; i != r; ++i )
                row_plan( X.data() + i * c );
            if ( r > 1 )
            {
                std::optional< fft_plan< real_type > > own;
                if ( r != c ) own.emplace( r, sign );
                fft_plan< real_type >& col_plan = r == c ? row_plan : *own;
                std::vector< complex_type > col( r );
                for ( std::size_t j = 0; j != c; ++j )
                {
                    for ( std::size_t i = 0; i != r; ++i ) col[i] = X[i][j];
                    col_plan( col.data() );
                    for ( std::size_t i = 0; i != r; ++i ) X[i][j] = col[i];
                }
            }
            fft2_scale( X, sign );
            return X;
        }

        // S10-R3 reference: the per-row transform fft2 replaced (tables rebuilt for every row and column); tests only
        template < Matrix Mat >
        auto fft2_reference( Mat const& x, int sign ) noexcept
        {
            std::size_t const r = x.row();
            std::size_t const c = x.col();
            auto X = fft2_load( x );
            if ( r == 0 || c == 0 ) return X;
            for ( std::size_t i = 0; i != r; ++i )
                fft_1d( X.data() + i * c, c, sign );
            if ( r > 1 )
            {
                std::vector< typename decltype( X )::value_type > col( r );
                for ( std::size_t j = 0; j != c; ++j )
                {
                    for ( std::size_t i = 0; i != r; ++i ) col[i] = X[i][j];
                    fft_1d( col.data(), r, sign );
                    for ( std::size_t i = 0; i != r; ++i ) X[i][j] = col[i];
                }
            }
            fft2_scale( X, sign );
            return X;
        }

        // out[(i + dr) mod R][(j + dc) mod C] = x[i][j], a new matrix of x's type and allocator
        template < Matrix Mat >
        std::remove_cvref_t< Mat > roll2( Mat const& x, std::ptrdiff_t dr, std::ptrdiff_t dc ) noexcept
        {
            using out_type = std::remove_cvref_t< Mat >;
            std::size_t const r = x.row();
            std::size_t const c = x.col();
            out_type out{ x.get_allocator(), r, c };
            if ( r == 0 || c == 0 ) return out;
            std::ptrdiff_t const sr = static_cast< std::ptrdiff_t >( r );
            std::ptrdiff_t const sc = static_cast< std::ptrdiff_t >( c );
            std::size_t const sdr = static_cast< std::size_t >( ( dr % sr + sr ) % sr );
            std::size_t const sdc = static_cast< std::size_t >( ( dc % sc + sc ) % sc );
            for ( std::size_t i = 0; i != r; ++i )
            {
                auto const src = x.row_begin( i );
                auto const dst = out.row_begin( ( i + sdr ) % r );
                std::copy( src, src + static_cast< std::ptrdiff_t >( c - sdc ), dst + static_cast< std::ptrdiff_t >( sdc ) );
                std::copy( src + static_cast< std::ptrdiff_t >( c - sdc ), src + sc, dst );
            }
            return out;
        }
    }//namespace matrix_details

    // numpy.fft.fft2: X[k][l] = sum x[m][n] exp(-2 pi i (km/R + ln/C))
    template < Matrix Mat >
    [[nodiscard]] auto fft( Mat const& x ) noexcept
    {
        return matrix_details::fft2( x, -1 );
    }

    // numpy.fft.fftshift: roll by (R/2, C/2), no transform
    template < Matrix Mat >
    [[nodiscard]] auto fftshift( Mat const& x ) noexcept
    {
        return matrix_details::roll2( x, static_cast< std::ptrdiff_t >( x.row() / 2 ), static_cast< std::ptrdiff_t >( x.col() / 2 ) );
    }

    template < Matrix Mat >
    [[nodiscard]] int forward_substitution( Mat const& A, Mat& x, Mat const& b ) noexcept
    {
        typedef Mat matrix_type;
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

    // S7-R4 (F15, D-026): reduced row echelon form of any m×n A. Columns c = 0..n−1 are visited in turn; the pivot
    // of column c is the first entry of largest |·| in rows r..m−1 and counts only when it exceeds
    // max(m, n)·ε·‖A‖∞ (MATLAB's rref tolerance); the row index r advances only on a pivot, so zero and dependent
    // columns are skipped (their entries in rows r..m−1 are set to 0). Pivot entries are exactly 1 and the other
    // entries of a pivot column exactly 0. status is ok, or nonfinite (r = A, no pivots) for a NaN or inf in A.
    template < typename T, Allocator A_ = std::allocator< T > >
    struct rref_result
    {
        matrix< T, A_ > r;
        std::vector< std::size_t > pivot_columns;
        std::size_t rank = 0;
        linalg_status status = linalg_status::ok;
    };

    template < typename T, Allocator A_ > requires linalg_element< T >
    [[nodiscard]] rref_result< T, A_ > row_echelon( matrix< T, A_ > const& m ) noexcept
    {
        using R = matrix_details::linalg_real_t< T >;
        rref_result< T, A_ > ans{ m, {}, 0, linalg_status::ok };
        std::size_t const rows = m.row();
        std::size_t const cols = m.col();
        T* const a = ans.r.data();
        if ( !matrix_details::all_finite( a, rows * cols ) ) { ans.status = linalg_status::nonfinite; return ans; }
        R norm_inf{ 0 };
        for ( std::size_t i = 0; i != rows; ++i )
        {
            R s{ 0 };
            for ( std::size_t j = 0; j != cols; ++j ) s += std::abs( a[i * cols + j] );
            norm_inf = std::max( norm_inf, s );
        }
        R const tol = static_cast< R >( std::max( rows, cols ) ) * std::numeric_limits< R >::epsilon() * norm_inf;
        std::size_t r = 0;
        for ( std::size_t c = 0; c != cols && r != rows; ++c )
        {
            std::size_t p = r;
            R best = std::abs( a[r * cols + c] );
            for ( std::size_t i = r + 1; i != rows; ++i )
                if ( R const v = std::abs( a[i * cols + c] ); v > best ) { best = v; p = i; }
            if ( !( best > tol ) )
            {
                for ( std::size_t i = r; i != rows; ++i ) a[i * cols + c] = T( 0 );
                continue;
            }
            if ( p != r ) std::swap_ranges( a + r * cols, a + r * cols + cols, a + p * cols );
            T* const pr = a + r * cols;
            T const pivot = pr[c];
            for ( std::size_t j = c + 1; j != cols; ++j ) pr[j] /= pivot;
            pr[c] = T( 1 );
            for ( std::size_t i = 0; i != rows; ++i )
            {
                if ( i == r ) continue;
                T* const ri = a + i * cols;
                T const f = ri[c];
                if ( f == T( 0 ) ) continue;
                for ( std::size_t j = c + 1; j != cols; ++j ) ri[j] -= f * pr[j];
                ri[c] = T( 0 );
            }
            ans.pivot_columns.push_back( c );
            ++r;
        }
        // columns left once every row holds a pivot need no zeroing: rows r..m−1 do not exist
        ans.rank = r;
        return ans;
    }

    // Legacy spellings (S7-R4): the RREF of any shape, nullopt only for a nonfinite input
    template < Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] std::optional<Mat> gauss_jordan_elimination( Mat const& m ) noexcept
    {
        auto res = row_echelon( m );
        if ( res.status != linalg_status::ok )
            return {};
        return std::optional<Mat>{ std::move( res.r ) };
    }

    // matlab alias
    template < Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] std::optional<Mat> rref( Mat const& m ) noexcept
    {
        return gauss_jordan_elimination( m );
    }

    // numpy.fft.ifft2: exp(+2 pi i (km/R + ln/C)), divided by R*C
    template < typename T, Allocator A >
    [[nodiscard]] auto ifft( matrix<T, A> const& x ) noexcept
    {
        return matrix_details::fft2( x, +1 );
    }
    // numpy.fft.ifftshift: roll by (-(R/2), -(C/2)), no transform
    template < Matrix Mat >
    [[nodiscard]] auto ifftshift( Mat const& x ) noexcept
    {
        return matrix_details::roll2( x, -static_cast< std::ptrdiff_t >( x.row() / 2 ), -static_cast< std::ptrdiff_t >( x.col() / 2 ) );
    }

    // S7-R1 (F13, D-026, D-027): the partial-pivoting LU object. Factors of a square A with P·A = L·U; row i of
    // P·A is row pivots()[i] of A; status ok, singular (rank < n) or nonfinite.
    template < typename T, Allocator A_ = std::allocator< T > > requires linalg_element< T >
    class lu_factorization
    {
    public:
        using matrix_type = matrix< T, A_ >;

        lu_factorization() noexcept = default;
        explicit lu_factorization( matrix_type const& a ) noexcept : lu_( a ), piv_( a.row() )
        {
            better_assert( a.row() == a.col(), "feng::lu_factor: expecting a square matrix, but got ", a.row(), "x", a.col() );
            info_ = matrix_details::lu_in_place( lu_.data(), lu_.row(), piv_.data() );
        }

        [[nodiscard]] linalg_status status() const noexcept { return info_.status; }
        [[nodiscard]] bool ok() const noexcept { return info_.status == linalg_status::ok; }
        [[nodiscard]] std::size_t rank() const noexcept { return info_.rank; }
        [[nodiscard]] std::size_t size() const noexcept { return lu_.row(); }
        [[nodiscard]] std::vector< std::size_t > const& pivots() const noexcept { return piv_; }

        // unit lower triangular factor
        [[nodiscard]] matrix_type l() const noexcept
        {
            std::size_t const n = size();
            matrix_type ans( n, n );
            std::fill( ans.begin(), ans.end(), T{ 0 } );
            for ( std::size_t i = 0; i != n; ++i )
            {
                for ( std::size_t j = 0; j != i; ++j ) ans[i][j] = lu_[i][j];
                ans[i][i] = T{ 1 };
            }
            return ans;
        }
        // upper triangular factor
        [[nodiscard]] matrix_type u() const noexcept
        {
            std::size_t const n = size();
            matrix_type ans( n, n );
            std::fill( ans.begin(), ans.end(), T{ 0 } );
            for ( std::size_t i = 0; i != n; ++i )
                for ( std::size_t j = i; j != n; ++j ) ans[i][j] = lu_[i][j];
            return ans;
        }
        // permutation matrix: P[i][pivots()[i]] = 1
        [[nodiscard]] matrix_type p() const noexcept
        {
            std::size_t const n = size();
            matrix_type ans( n, n );
            std::fill( ans.begin(), ans.end(), T{ 0 } );
            for ( std::size_t i = 0; i != n; ++i ) ans[i][piv_[i]] = T{ 1 };
            return ans;
        }
        // sign(P)·∏u_kk; exactly 0 when rank < n, 1 for 0×0, NaN for a nonfinite input
        [[nodiscard]] T det() const noexcept
        {
            return matrix_details::lu_det( lu_.data(), size(), info_ );
        }
        // X with A·X = B (B n×k); singular or nonfinite: that status and an empty value
        [[nodiscard]] linalg_result< matrix_type > solve( matrix_type const& b ) const noexcept
        {
            better_assert( b.row() == size(), "feng::lu_factorization::solve: B must have ", size(), " rows, but got ", b.row(), "x", b.col() );
            if ( !ok() ) return { matrix_type{}, info_.status };
            matrix_type x( size(), b.col() );
            matrix_details::lu_solve( lu_.data(), size(), piv_.data(), b.data(), b.col(), x.data() );
            if ( !matrix_details::all_finite( x.data(), x.size() ) ) return { matrix_type{}, linalg_status::nonfinite };
            return { std::move( x ), linalg_status::ok };
        }
        [[nodiscard]] linalg_result< matrix_type > inverse() const noexcept
        {
            matrix_type id( size(), size() );
            std::fill( id.begin(), id.end(), T{ 0 } );
            for ( std::size_t i = 0; i != size(); ++i ) id[i][i] = T{ 1 };
            return solve( id );
        }

    private:
        matrix_type lu_;
        std::vector< std::size_t > piv_;
        matrix_details::lu_info info_{ linalg_status::ok, 0, 1 };
    };

    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] lu_factorization< T, A > lu_factor( matrix< T, A > const& a ) noexcept
    {
        return lu_factorization< T, A >( a );
    }

    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] linalg_result< matrix< T, A > > solve( matrix< T, A > const& a, matrix< T, A > const& b ) noexcept
    {
        better_assert( a.row() == a.col() && a.row() == b.row(), "feng::solve: expecting a square A and B with as many rows, but got A ", a.row(), "x", a.col(), " and B ", b.row(), "x", b.col() );
        return lu_factor( a ).solve( b );
    }

    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] linalg_result< matrix< T, A > > try_inverse( matrix< T, A > const& a ) noexcept
    {
        better_assert( a.row() == a.col(), "feng::try_inverse: expecting a square matrix, but got ", a.row(), "x", a.col() );
        return lu_factor( a ).inverse();
    }

    // S9-R5 (D-032): the S5 loaders' outcome and file formats, for load_expected; declared in every mode.
    enum class io_status { ok, failed };
    enum class io_format { txt, binary, npy };

#if defined( __cpp_lib_expected )
    // S9-R5: std::expected views of the status results: the value (or factorization) when ok, else the status.
    template < typename V >
    [[nodiscard]] std::expected< V, linalg_status > to_expected( linalg_result< V > r ) noexcept
    {
        if ( !r.ok() ) return std::unexpected( r.status );
        return std::expected< V, linalg_status >{ std::in_place, std::move( r.value ) };
    }

    template < typename F > requires requires( F const& f ) { { f.status() } -> std::same_as< linalg_status >; }
    [[nodiscard]] std::expected< F, linalg_status > to_expected( F f ) noexcept
    {
        if ( f.status() != linalg_status::ok ) return std::unexpected( f.status() );
        return std::expected< F, linalg_status >{ std::in_place, std::move( f ) };
    }

    template < typename T, Allocator A >
    [[nodiscard]] std::expected< rref_result< T, A >, linalg_status > to_expected( rref_result< T, A > r ) noexcept
    {
        if ( r.status != linalg_status::ok ) return std::unexpected( r.status );
        return std::expected< rref_result< T, A >, linalg_status >{ std::in_place, std::move( r ) };
    }

    // S9-R5: the matrix load_txt, load_binary or load_npy reads from path, or io_status::failed (the loader has
    // printed its one stderr line).
    template < typename T, Allocator A = std::allocator< T > >
    [[nodiscard]] std::expected< matrix< T, A >, io_status > load_expected( std::string const& path, io_format format ) noexcept
    {
        matrix< T, A > m;
        bool const ok = format == io_format::npy ? m.load_npy( path ) : format == io_format::txt ? m.load_txt( path ) : m.load_binary( path );
        if ( !ok ) return std::unexpected( io_status::failed );
        return std::expected< matrix< T, A >, io_status >{ std::in_place, std::move( m ) };
    }
#endif

    // out is assigned only when the status is ok (D-026)
    template < typename T, Allocator A > requires linalg_element< T >
    [[nodiscard]] linalg_status inverse( matrix< T, A > const& a, matrix< T, A >& out ) noexcept
    {
        better_assert( a.row() == a.col(), "feng::inverse: expecting a square matrix, but got ", a.row(), "x", a.col() );
        return matrix_details::lu_inverse( a, out );
    }

    // Legacy two-output LU (MATLAB form): L = Pᵀ·L₀ is a permuted unit lower triangle and A = L·U. Returns 0, or 1
    // with L and U unchanged when the factorization is not ok (singular or nonfinite).
    template< Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] int lu_decomposition( Mat const& A, Mat& L, Mat& U ) noexcept
    {
        typedef typename Mat::value_type value_type;
        better_assert( A.row() == A.col(), "lu_decomposition: expecting a square matrix, but got ", A.row(), "x", A.col() );
        auto const f = lu_factor( A );
        if ( !f.ok() )
            return 1;
        std::size_t const n = A.row();
        auto const& piv = f.pivots();
        Mat l0 = f.l();
        Mat l( n, n );
        std::fill( l.begin(), l.end(), value_type{ 0 } );
        for ( std::size_t i = 0; i != n; ++i ) // row piv[i] of Pᵀ·L₀ is row i of L₀
            std::copy( l0.row_begin( i ), l0.row_end( i ), l.row_begin( piv[i] ) );
        U = f.u();
        L = std::move( l );
        return 0;
    }

    template< Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] std::optional<std::tuple<Mat, Mat>> lu_decomposition( Mat const& A ) noexcept
    {
        if ( Mat L, U; lu_decomposition( A, L, U ) == 1 )
            return {};
        else
            return std::make_tuple( L, U );
    }

    // Legacy solver for a column b: 0 with x = A⁻¹b, or 1 with x unchanged when A is singular or nonfinite.
    template< Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] int lu_solver( Mat const& A, Mat& x, Mat const& b ) noexcept
    {
        better_assert( A.row() == A.col() && A.row() == b.row() && b.col() == 1, "lu_solver: expecting a square A and a column b with as many rows, but got A ", A.row(), "x", A.col(), " and b ", b.row(), "x", b.col() );
        auto r = lu_factor( A ).solve( b );
        if ( !r.ok() )
            return 1;
        x = std::move( r.value );
        return 0;
    }

    template< Matrix Mat > requires linalg_element< typename Mat::value_type >
    [[nodiscard]] std::optional<Mat> lu_solver( Mat const& A, Mat const& b ) noexcept
    {
        if ( Mat x; lu_solver(A, x, b) == 1 )
            return {};
        else
            return x;
    }

    namespace matrix_details
    {
        // S8-R1 (F15, D-030): the full 2-D convolution of non-empty A (ra x ca) and B (rb x cb), written directly:
        // out[i][j] = sum of A[p][q] * B[i-p][j-q] over the overlap (the kernel reversed), no padded copy. Rows of
        // the result are split with parallel; small problems stay on the calling thread.
        template< Matrix Mat >
        Mat conv_full( Mat const& A, Mat const& B ) noexcept
        {
            using value_type = typename Mat::value_type;
            std::size_t const ra = A.row(), ca = A.col(), rb = B.row(), cb = B.col();
            std::size_t const ro = ra + rb - 1, co = ca + cb - 1;
            Mat out{ A.get_allocator(), ro, co };
            value_type const* const a = A.data();
            value_type const* const b = B.data();
            value_type* const o = out.data();
            auto const row = [=]( std::size_t i ) noexcept
            {
                std::size_t const p0 = i + 1 > rb ? i + 1 - rb : 0;
                std::size_t const p1 = std::min( i + 1, ra );
                for ( std::size_t j = 0; j != co; ++j )
                {
                    std::size_t const q0 = j + 1 > cb ? j + 1 - cb : 0;
                    std::size_t const q1 = std::min( j + 1, ca );
                    value_type acc{ 0 };
                    for ( std::size_t p = p0; p != p1; ++p )
                    {
                        value_type const* const ap = a + p * ca;
                        value_type const* const bp = b + ( i - p ) * cb;
                        for ( std::size_t q = q0; q != q1; ++q )
                            acc += ap[q] * bp[j - q];
                    }
                    o[i * co + j] = acc;
                }
            };
            std::size_t const work = ro * co * std::min( ra * ca, rb * cb );
            parallel( row, std::size_t{ 0 }, ro, work < ( std::size_t{ 1 } << 16 ) ? std::numeric_limits< unsigned long >::max() : 0UL );
            return out;
        }
    }

    // S8-R1 (F15): scipy.signal.convolve2d( A, B, "full" ); an operand with a zero dimension gives 0x0 (D-030).
    template< Matrix Mat >
    [[nodiscard]] Mat conv( Mat const& A, Mat const& B ) noexcept
    {
        if ( ( 0 == A.size() ) || ( 0 == B.size() ) )
            return Mat{ A.get_allocator(), 0, 0 };
        return matrix_details::conv_full( A, B );
    }

    // S8-R1 (F15): scipy.signal.convolve2d( A, B, mode ) for mode "full", "same" or "valid"; any other mode aborts
    // (D-011). same is A's shape cropped from full at ((rb-1)/2, (cb-1)/2); valid needs one operand to contain the
    // other in both dimensions and aborts otherwise. Empty operands (D-030): full and valid give 0x0, same gives
    // zeros of A's shape.
    template< Matrix Mat >
    [[nodiscard]] Mat conv( Mat const& A, Mat const& B, std::string const& mode ) noexcept
    {
        bool const full = mode == "full", same = mode == "same", valid = mode == "valid";
        better_assert( full || same || valid, "conv: unknown mode '", mode, "'; expecting \"full\", \"same\" or \"valid\"" );

        auto const [ra, ca] = A.shape();
        auto const [rb, cb] = B.shape();

        if ( ( 0 == A.size() ) || ( 0 == B.size() ) )
            return same ? Mat{ A.get_allocator(), ra, ca } : Mat{ A.get_allocator(), 0, 0 };

        if ( full )
            return matrix_details::conv_full( A, B );

        if ( same )
        {
            Mat const f = matrix_details::conv_full( A, B );
            return { f, { (rb-1)/2, ra + (rb-1)/2 }, { (cb-1)/2, ca + (cb-1)/2 } };
        }

        bool const a_contains_b = ra >= rb && ca >= cb;
        bool const b_contains_a = rb >= ra && cb >= ca;
        better_assert( a_contains_b || b_contains_a, "conv: 'valid' mode needs one operand to contain the other in both dimensions, but got ", ra, "x", ca, " and ", rb, "x", cb );
        if ( a_contains_b )
        {
            Mat const f = matrix_details::conv_full( A, B );
            return { f, { rb-1, ra }, { cb-1, ca } };
        }
        Mat const f = matrix_details::conv_full( B, A );
        return { f, { ra-1, rb }, { ca-1, cb } };
    }

    template< typename ... Args >
    [[nodiscard]] auto conv2( Args const& ... args ) noexcept
    {
        return conv( args... );
    }

    namespace matrix_details
    {
        // S5-R3 (F08): parses a complete BMP file image of `size` bytes into its red, green and blue channels.
        // Accepts the 14-byte file header with signature BM, an info header of size 40, 52, 56, 108 or 124 inside
        // the file, planes 1, compression 0 (BI_RGB), 24 or 32 bits per pixel (alpha ignored), width > 0, height
        // != 0 and != INT32_MIN, a pixel-data offset at or past the end of the info header and offset +
        // stride * |height| <= size in checked 64-bit arithmetic (trailing bytes allowed). A positive height is
        // stored bottom-up, a negative one top-down. On failure returns an empty optional and sets *why when given.
        inline std::optional<std::array<matrix<std::uint8_t>,3>> parse_bmp( std::uint8_t const* bytes, std::size_t size, char const** why = nullptr ) noexcept
        {
            auto fail = [why]( char const* reason ) noexcept -> std::optional<std::array<matrix<std::uint8_t>,3>>
            {
                if ( why ) *why = reason;
                return std::nullopt;
            };
            auto u16 = [bytes]( std::size_t at ) noexcept { return static_cast<std::uint32_t>( bytes[at] ) | ( static_cast<std::uint32_t>( bytes[at+1] ) << 8 ); };
            auto u32 = [u16]( std::size_t at ) noexcept { return u16( at ) | ( u16( at+2 ) << 16 ); };

            if ( bytes == nullptr || size < 18 ) return fail( "file too short for the BMP headers" );
            if ( bytes[0] != 'B' || bytes[1] != 'M' ) return fail( "bad BMP signature" );
            std::uint32_t const info_size = u32( 14 );
            if ( info_size != 40 && info_size != 52 && info_size != 56 && info_size != 108 && info_size != 124 )
                return fail( "unsupported BMP info header size" );
            std::uint64_t const headers_end = 14 + std::uint64_t{ info_size };
            if ( headers_end > size ) return fail( "BMP info header extends past the end of the file" );
            std::int32_t const width = static_cast<std::int32_t>( u32( 18 ) );
            std::int32_t const height = static_cast<std::int32_t>( u32( 22 ) );
            std::uint32_t const planes = u16( 26 );
            std::uint32_t const bpp = u16( 28 );
            std::uint32_t const compression = u32( 30 );
            std::uint64_t const offset = u32( 10 );
            if ( planes != 1 ) return fail( "BMP planes is not 1" );
            if ( compression != 0 ) return fail( "compressed BMP is not supported" );
            if ( bpp != 24 && bpp != 32 ) return fail( "unsupported BMP bit depth" );
            if ( width <= 0 ) return fail( "BMP width is not positive" );
            if ( height == 0 || height == std::numeric_limits<std::int32_t>::min() ) return fail( "bad BMP height" );
            if ( offset < headers_end ) return fail( "BMP pixel data overlaps the headers" );

            std::uint64_t const cols = static_cast<std::uint64_t>( width );
            std::uint64_t const rows = height < 0 ? static_cast<std::uint64_t>( -static_cast<std::int64_t>( height ) ) : static_cast<std::uint64_t>( height );
            std::uint64_t bits = 0, stride = 0, pixels = 0, pixels_end = 0;
            if ( !checked_mul( cols, std::uint64_t{ bpp }, bits ) || !checked_add( bits, std::uint64_t{ 31 }, bits ) )
                return fail( "BMP row size overflows" );
            stride = bits / 32 * 4;
            if ( !checked_mul( stride, rows, pixels ) || !checked_add( offset, pixels, pixels_end ) )
                return fail( "BMP pixel array size overflows" );
            if ( pixels_end > size ) return fail( "BMP pixel data extends past the end of the file" );

            std::array<matrix<std::uint8_t>,3> ans{ matrix<std::uint8_t>( rows, cols ), matrix<std::uint8_t>( rows, cols ), matrix<std::uint8_t>( rows, cols ) };
            auto& [mat_r, mat_g, mat_b] = ans;
            std::uint64_t const step = bpp / 8;
            for ( std::uint64_t k = 0; k != rows; ++k )
            {
                std::uint64_t const r = height > 0 ? rows - 1 - k : k;
                std::uint8_t const* p = bytes + offset + k * stride;
                for ( std::uint64_t c = 0; c != cols; ++c, p += step )
                {
                    mat_b[r][c] = p[0];
                    mat_g[r][c] = p[1];
                    mat_r[r][c] = p[2];
                }
            }
            return ans;
        }
    }//namespace matrix_details

    // S5-R3/S5-R4 (D-011): reads the whole file and parses it with matrix_details::parse_bmp; on failure returns an
    // empty optional and prints one stderr line naming load_bmp and the reason. Never aborts.
    [[nodiscard]] inline std::optional<std::array<matrix<std::uint8_t>,3>> load_bmp( std::string const& file_path ) noexcept
    {
        std::vector<std::uint8_t> bytes;
        if ( !matrix_details::read_file( file_path.c_str(), bytes, "feng::load_bmp" ) )
            return std::nullopt;
        char const* why = "invalid BMP file";
        auto ans = matrix_details::parse_bmp( bytes.data(), bytes.size(), &why );
        if ( !ans )
            std::cerr << "feng::load_bmp -- " << why << ": " << file_path << "\n";
        return ans;
    }

    template< Matrix Mat >
    [[nodiscard]] auto pooling( Mat const& mat, std::uint_least64_t dim_r, std::uint_least64_t dim_c, std::string const& pooling_action = "mean" ) noexcept
    {
        typedef typename Mat::value_type T;

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

        if (dim_r == 0 || dim_c == 0) return Mat{};

        if (dim_r==1 && dim_c==1) return mat;

        auto init_value =(*iterator).second.first;
        auto const& the_function =(*iterator).second.second;
        auto const [row, col] = mat.shape();
        auto const [new_row, new_col] = std::make_pair( row/dim_r, col/dim_c );
        Mat ans{ new_row, new_col, T{0} };

        auto const& make_pooling = [&ans, &mat, &the_function, new_col=new_col, dim_r, dim_c, init_value]( std::uint_least64_t r )
        {
            for ( auto c : matrix_details::range( new_col ) )
            {
                ans[r][c] = init_value;
                for ( auto rr : matrix_details::range( dim_r ) )
                    ans[r][c] = std::accumulate( mat.row_begin(r*dim_r+rr)+c*dim_c, mat.row_begin(r*dim_r+rr)+c*dim_c+dim_c, ans[r][c], the_function );
            }
        };

        matrix_details::parallel_work( make_pooling, 0UL, new_row, matrix_details::saturating_work( new_col, matrix_details::saturating_work( dim_r, dim_c ) ), matrix_details::callback_grain );

        return ans;
    }

    template< Matrix Mat >
    [[nodiscard]] auto pooling( Mat const& mat, std::uint_least64_t dim, std::string const& pooling_action = "mean"  ) noexcept
    {
        return pooling( mat, dim, dim, pooling_action );
    }

    template< Matrix Mat >
    // S5-R4 (D-011): true on success; false with one stderr line on a directory, open, write or close failure, never
    // aborting (mismatched channel shapes remain a contract violation). S5-R3: the file loads back through load_bmp
    // to the scaled channels in the same orientation.
    [[nodiscard]] bool save_as_bmp( std::string const& file_name, Mat const& red_channel, Mat const& green_channel, Mat const& blue_channel ) noexcept
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
        // encode_bmp_stream writes its row k as the k-th stored (bottom-up) row, so channel row k holds image row
        // row-1-k (the member save_as_bmp flips the same way)
        for ( auto r : matrix_details::range(row) )
            for ( auto c : matrix_details::range(col) )
            {
                auto const k = row - 1 - r;
                channel_r[k][c] = static_cast<std::uint8_t>( 256.0 * (red_channel[r][c] - red_mn) / (red_mx-red_mn+0.1) );
                channel_g[k][c] = static_cast<std::uint8_t>( 256.0 * (green_channel[r][c] - green_mn) / (green_mx-green_mn+0.1) );
                channel_b[k][c] = static_cast<std::uint8_t>( 256.0 * (blue_channel[r][c] - blue_mn) / (blue_mx-blue_mn+0.1) );
            }

        // encode rgb to bitmap stream
        auto const& encoding = matrix_details::encode_bmp_stream( channel_r, channel_g, channel_b );
        if ( !encoding ) return matrix_details::write_failed( "feng::save_as_bmp", "failed to encode the BMP stream", file_name );

        // write file stream
        std::string new_file_name{ file_name };
        std::string const extension{ ".bmp" };
        if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
            new_file_name += extension;
        return matrix_details::write_stream( "feng::save_as_bmp", new_file_name, std::ios_base::out | std::ios_base::binary,
                                             [&encoding]( std::ofstream& stream )
                                             { stream.write( reinterpret_cast<char const*>((*encoding).data()), static_cast<std::streamsize>( (*encoding).size() ) ); } );
    }

    // S5-R4 (D-011): the member save_as_bmp's result.
    template< Matrix Mat >
    [[nodiscard]] bool save_as_bmp( std::string const& file_name, Mat const& mat, std::string const& colormap = std::string{"parula"} ) noexcept
    {
        return mat.save_as_bmp( file_name, colormap );
    }

    template< typename T >
    [[nodiscard]] auto meshgrid( T const& x, T const& y ) noexcept
    {
        unsigned long const row = static_cast<unsigned long>( y );
        unsigned long const col = static_cast<unsigned long>( x );

        matrix<T> mat_x{ row, col, T{} };
        matrix<T> mat_y{ row, col, T{} };

        auto const& parallel_func = [&]( unsigned long r )
        {
            for ( auto c = 0UL; c != col; ++c )
            {
                mat_x[r][c] = static_cast<T>(r);
                mat_y[r][c] = static_cast<T>(c);
            }
        };

        matrix_details::parallel_work( parallel_func, 0UL, row, matrix_details::saturating_work( col, 2 ), matrix_details::elementwise_grain );

        return std::make_pair( mat_y, mat_x );
    }

    namespace matrix_details
    {
        // S9-R6: the one elementwise transform. Returns matrix< R, A rebound to R > of m's shape holding f( x ) for
        // each element x of m, assigned (so converted) to R; R is f's result type unless given as transform< R >.
        // The allocator is m's, rebound; the elements are written by the parallel for_each (S6).
        // Further matrix operands ns (same shape as m; checked by transform_checked) are read alongside: f( x, y... ).
        template < typename R = void, typename T, Allocator A, typename F, Matrix... Ns >
        auto transform( matrix< T, A > const& m, F const& f, Ns const&... ns ) noexcept
        {
            using result_type = std::conditional_t< std::is_void_v< R >, std::invoke_result_t< F const&, T const&, typename Ns::value_type const&... >, R >;
            using result_alloc = typename std::allocator_traits< A >::template rebind_alloc< result_type >;
            matrix< result_type, result_alloc > ans{ result_alloc( m.get_allocator() ), m.row(), m.col() };
            matrix_details::for_each( ans.begin(), ans.end(), m.begin(), ns.begin()..., [&f]( result_type& v, T const& x, auto const&... y ) { v = f( x, y... ); } );
            return ans;
        }

        // Two or three matrix operands: the shape check (expect_same_shape, under name), then transform.
        template < typename R = void, typename F, Matrix M, Matrix... Ns >
        auto transform_checked( char const* name, F const& f, M const& m, Ns const&... ns ) noexcept
        {
            matrix_details::expect_same_shape( name, m, ns... );
            return matrix_details::transform< R >( m, f, ns... );
        }
    }

    //
    // - begin of unary functions
    //
    // Each keeps its pre-S9 result type: m's type, or the integer type cmath returns (ilogb, lrint, llrint, lround, llround).
    template< Matrix Mat >
    [[nodiscard]] auto abs( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::abs( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto exp( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::exp( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto exp2( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::exp2( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto expm1( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::expm1( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto log( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::log( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto log10( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::log10( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto log1p( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::log1p( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto log2( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::log2( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto sqrt( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::sqrt( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto cbrt( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::cbrt( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto sin( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::sin( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto cos( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::cos( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto tan( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::tan( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto asin( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::asin( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto acos( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::acos( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto atan( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::atan( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto sinh( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::sinh( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto cosh( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::cosh( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto tanh( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::tanh( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto asinh( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::asinh( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto acosh( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::acosh( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto atanh( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::atanh( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto erf( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::erf( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto erfc( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::erfc( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto tgamma( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::tgamma( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto lgamma( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::lgamma( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto trunc( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::trunc( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto round( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::round( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto ceil( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::ceil( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto floor( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::floor( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto rint( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::rint( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto logb( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::logb( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto comp_ellint_1( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::comp_ellint_1( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto comp_ellint_2( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::comp_ellint_2( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto comp_ellint_3( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::comp_ellint_3( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto expint( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::expint( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto riemann_zeta( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::riemann_zeta( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto nearbyint( Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, []( auto const x ){ return std::nearbyint( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto ilogb( Mat const& m ) noexcept { return matrix_details::transform< int >( m, []( auto const x ){ return std::ilogb( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto lrint( Mat const& m ) noexcept { return matrix_details::transform< long >( m, []( auto const x ){ return std::lrint( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto llrint( Mat const& m ) noexcept { return matrix_details::transform< long long >( m, []( auto const x ){ return std::llrint( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto lround( Mat const& m ) noexcept { return matrix_details::transform< long >( m, []( auto const x ){ return std::lround( x ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto llround( Mat const& m ) noexcept { return matrix_details::transform< long long >( m, []( auto const x ){ return std::llround( x ); } ); }

    //
    // - end of unary functions
    //


    //
    // - begin of trinary and binary functions
    //
    // Each keeps m's type (zeros_like( m ) before S9): the matrix-matrix forms check shapes in transform_checked,
    // the scalar forms capture the scalar.

    template< Matrix Mat >
    [[nodiscard]] auto fma( Mat const& m1, Mat const& m2, Mat const& m3 ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "fma", []( auto const x, auto const y, auto const z ){ return std::fma( x, y, z ); }, m1, m2, m3 ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto ldexp( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "ldexp", []( auto const x, auto const y ){ return std::ldexp( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto ldexp( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::ldexp( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto ldexp( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::ldexp( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto scalbn( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "scalbn", []( auto const x, auto const y ){ return std::scalbn( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto scalbn( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::scalbn( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto scalbn( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::scalbn( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto scalbln( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "scalbln", []( auto const x, auto const y ){ return std::scalbln( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto scalbln( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::scalbln( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto scalbln( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::scalbln( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto pow( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "pow", []( auto const x, auto const y ){ return std::pow( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto pow( Mat const& m, std::integral auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::pow( x, y ); } ); } // <- pow accept integral as the second argument
    template< Matrix Mat >
    [[nodiscard]] auto pow( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::pow( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto pow( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::pow( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto hypot( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "hypot", []( auto const x, auto const y ){ return std::hypot( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto hypot( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::hypot( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto hypot( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::hypot( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto fmod( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "fmod", []( auto const x, auto const y ){ return std::fmod( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto fmod( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fmod( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto fmod( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fmod( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto remainder( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "remainder", []( auto const x, auto const y ){ return std::remainder( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto remainder( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::remainder( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto remainder( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::remainder( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto copysign( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "copysign", []( auto const x, auto const y ){ return std::copysign( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto copysign( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::copysign( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto copysign( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::copysign( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto nextafter( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "nextafter", []( auto const x, auto const y ){ return std::nextafter( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto nextafter( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::nextafter( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto nextafter( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::nextafter( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto fdim( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "fdim", []( auto const x, auto const y ){ return std::fdim( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto fdim( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fdim( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto fdim( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fdim( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto fmax( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "fmax", []( auto const x, auto const y ){ return std::fmax( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto fmax( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fmax( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto fmax( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fmax( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto fmin( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "fmin", []( auto const x, auto const y ){ return std::fmin( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto fmin( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fmin( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto fmin( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::fmin( y, x ); } ); }

    template< Matrix Mat, Matrix Nat >
    [[nodiscard]] auto atan2( Mat const& m, Nat const& n ) noexcept { return matrix_details::transform_checked< typename Mat::value_type >( "atan2", []( auto const x, auto const y ){ return std::atan2( x, y ); }, m, n ); }
    template< Matrix Mat >
    [[nodiscard]] auto atan2( Mat const& m, std::floating_point auto y ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::atan2( x, y ); } ); }
    template< Matrix Mat >
    [[nodiscard]] auto atan2( std::floating_point auto y, Mat const& m ) noexcept { return matrix_details::transform< typename Mat::value_type >( m, [y]( auto const x ){ return std::atan2( y, x ); } ); }

    //
    // - end of trinary and binary functions
    //

    //
    // - begin of Complex Functions
    //
    // real, imag, abs, arg and norm return the complex value type, rebinding m's allocator; conj, proj and polar keep m's type.
    template< ComplexMatrix CMat >
    [[nodiscard]] auto real( CMat const& cm ) noexcept { return matrix_details::transform< typename CMat::value_type::value_type >( cm, []( auto const& x ){ return std::real( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto imag( CMat const& cm ) noexcept { return matrix_details::transform< typename CMat::value_type::value_type >( cm, []( auto const& x ){ return std::imag( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto abs( CMat const& cm ) noexcept { return matrix_details::transform< typename CMat::value_type::value_type >( cm, []( auto const& x ){ return std::abs( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto arg( CMat const& cm ) noexcept { return matrix_details::transform< typename CMat::value_type::value_type >( cm, []( auto const& x ){ return std::arg( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto norm( CMat const& cm ) noexcept { return matrix_details::transform< typename CMat::value_type::value_type >( cm, []( auto const& x ){ return std::norm( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto conj( CMat const& m ) noexcept { return matrix_details::transform< typename CMat::value_type >( m, []( auto const x ){ return std::conj( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto proj( CMat const& m ) noexcept { return matrix_details::transform< typename CMat::value_type >( m, []( auto const x ){ return std::proj( x ); } ); }
    template< ComplexMatrix CMat >
    [[nodiscard]] auto polar( CMat const& m ) noexcept { return matrix_details::transform< typename CMat::value_type >( m, []( auto const x ){ return std::polar( x ); } ); }

    //
    // - end of Complex Functions
    //


    ///
    /// - begin of reduce functions
    ///

    template< Matrix Mat >
    [[nodiscard]] auto sum( Mat const& m ) noexcept
    {
        return matrix_details::reduce( m.begin(), m.end(), typename Mat::value_type{}, []( auto const x, auto const y ){ return x+y; } );
    }

    // S6-R4 (F15, D-011, D-025): mean, variance and standard_deviation compute in stat_type_t<T> (double for
    // integral T, T for floating T, complex<X> for complex T) and return the mean's real type for the spread
    // (X for complex<X>); variance divides sum |x - mean|^2 by n - ddof. Empty input or n <= ddof aborts.
    namespace matrix_details
    {
        template< typename T >
        struct stat_type { using type = T; using real_type = T; };
        template< std::integral T >
        struct stat_type< T > { using type = double; using real_type = double; };
        template< typename X >
        struct stat_type< std::complex< X > > { using type = std::complex< X >; using real_type = X; };

        template< typename T >
        using stat_type_t = typename stat_type< T >::type;
        template< typename T >
        using stat_real_t = typename stat_type< T >::real_type;

        template< Matrix Mat >
        stat_real_t< typename Mat::value_type > variance_unchecked( Mat const& m, std::uint_least64_t ddof ) noexcept
        {
            using S = stat_type_t< typename Mat::value_type >;
            using R = stat_real_t< typename Mat::value_type >;
            S total{};
            for ( auto const& x : m ) total += static_cast< S >( x );
            S const mu = total / static_cast< R >( m.size() );
            R acc{};
            for ( auto const& x : m )
            {
                S const d = static_cast< S >( x ) - mu;
                if constexpr ( matrix_private::is_std_complex_v< S > )
                    acc += std::norm( d );
                else
                    acc += d * d;
            }
            return acc / static_cast< R >( m.size() - ddof );
        }
    }

    template< Matrix Mat >
    [[nodiscard]] auto mean( Mat const& m ) noexcept
    {
        using S = matrix_details::stat_type_t< typename Mat::value_type >;
        using R = matrix_details::stat_real_t< typename Mat::value_type >;
        FENG_MATRIX_EXPECTS( m.size() != 0, "feng::mean: empty matrix, shape ", m.row(), "x", m.col() );
        S total{};
        for ( auto const& x : m ) total += static_cast< S >( x );
        return S{ total / static_cast< R >( m.size() ) };
    }

    template< Matrix Mat >
    [[nodiscard]] auto variance( Mat const& m, std::uint_least64_t ddof = 0 ) noexcept
    {
        FENG_MATRIX_EXPECTS( m.size() != 0 && m.size() > ddof, "feng::variance: needs more elements than ddof, shape ", m.row(), "x", m.col(), ", ddof ", ddof );
        return matrix_details::variance_unchecked( m, ddof );
    }

    template< Matrix Mat >
    [[nodiscard]] auto standard_deviation( Mat const& m, std::uint_least64_t ddof = 0 ) noexcept
    {
        FENG_MATRIX_EXPECTS( m.size() != 0 && m.size() > ddof, "feng::standard_deviation: needs more elements than ddof, shape ", m.row(), "x", m.col(), ", ddof ", ddof );
        using std::sqrt;
        return sqrt( matrix_details::variance_unchecked( m, ddof ) );
    }

    ///
    /// - end of reduce functions
    ///


    ///
    /// - begin of misc functions
    ///

    template< typename T >
    [[nodiscard]] auto clip( T const lower, T const upper ) noexcept
    {
        return [lower, upper]<Matrix Mat>( Mat const& m ) noexcept
        {
            auto ans{ m };
            matrix_details::for_each( ans.begin(), ans.end(), [lower, upper]( auto & v ){ v = v < lower ? lower : v > upper ? upper : v; } );
            return ans;
        };
    }

    ///
    /// - end of misc functions
    ///

} //namespace feng

// S4-R2 (F06): views do not own their elements, so iterators taken from a temporary view stay valid while the
// owner lives.
namespace std::ranges
{
    template < feng::matrix_element Type, feng::Allocator Alloc >
    inline constexpr bool enable_borrowed_range< feng::matrix_view< Type, Alloc > > = true;

    template < feng::matrix_element Type, feng::Allocator Alloc >
    inline constexpr bool enable_borrowed_range< feng::mutable_matrix_view< Type, Alloc > > = true;
}

#undef better_assert

#endif

