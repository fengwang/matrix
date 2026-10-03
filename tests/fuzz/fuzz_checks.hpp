// S5-R5 (PR-7): the checks shared by the libFuzzer harnesses (which trap on a failure) and the
// `[S5][S5-R5]` replay test (which REQUIREs them). Each check parses a byte buffer with the matrix_details
// parsers into pre-filled destinations of several element types and returns nullptr when every parse was
// transactional and consistent, otherwise a static message. No file I/O.
#ifndef FENG_TESTS_FUZZ_CHECKS_HPP_INCLUDED
#define FENG_TESTS_FUZZ_CHECKS_HPP_INCLUDED

#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace fuzz_checks
{
    enum class format { npy, text, binary };

    template < typename T >
    struct is_complex : std::false_type {};
    template < typename F >
    struct is_complex< std::complex< F > > : std::true_type {};

    template < typename T >
    T fill_value( std::size_t i ) noexcept
    {
        if constexpr ( is_complex< T >::value )
        {
            using F = typename T::value_type;
            return T{ static_cast< F >( i ) + F( 0.5 ), -static_cast< F >( i ) };
        }
        else
            return static_cast< T >( i * 7 + 3 );
    }

    template < typename T >
    feng::matrix< T > prefilled( std::size_t r, std::size_t c )
    {
        feng::matrix< T > m( r, c );
        for ( std::size_t i = 0; i != m.size(); ++i ) m.data()[i] = fill_value< T >( i );
        return m;
    }

    template < typename T >
    bool consistent( feng::matrix< T > const& m ) noexcept
    {
        if ( m.size() != m.row() * m.col() ) return false;
        if ( m.size() != 0 && m.data() == nullptr ) return false;
        return true;
    }

    // Compares element values, not bytes: long double (and std::complex<long double>) carry padding bytes whose
    // contents are unspecified after an element assignment, so a byte comparison would be unreliable. The
    // pre-filled values are finite, so operator== is exact.
    template < typename T >
    bool unchanged( feng::matrix< T > const& m, std::size_t r, std::size_t c, std::vector< T > const& before ) noexcept
    {
        if ( m.row() != r || m.col() != c || m.size() != r * c || m.size() != before.size() ) return false;
        if ( before.empty() ) return true;
        if ( m.data() == nullptr ) return false;
        for ( std::size_t i = 0; i != before.size(); ++i )
            if ( !( m.data()[i] == before[i] ) ) return false;
        return true;
    }

    template < format F, typename T >
    char const* check_shape( std::uint8_t const* data, std::size_t size, std::size_t r, std::size_t c )
    {
        feng::matrix< T > m = prefilled< T >( r, c );
        std::vector< T > const before( m.data(), m.data() + m.size() );
        bool ok = false;
        if constexpr ( F == format::npy )
            ok = feng::matrix_details::parse_npy< T >( data, size, m );
        else if constexpr ( F == format::text )
            ok = feng::matrix_details::parse_text< T >( reinterpret_cast< char const* >( data ), size, m );
        else
            ok = feng::matrix_details::parse_binary< T >( data, size, m );
        if ( ok )
            return consistent( m ) ? nullptr : "a successful parse left an inconsistent matrix (size != rows*cols or null data)";
        return unchanged( m, r, c, before ) ? nullptr : "a failed parse changed the destination";
    }

    template < format F, typename T >
    char const* check_type( std::uint8_t const* data, std::size_t size )
    {
        if ( char const* e = check_shape< F, T >( data, size, 2, 3 ) ) return e;
        return check_shape< F, T >( data, size, 0, 0 );
    }

    template < format F >
    char const* check_all( std::uint8_t const* data, std::size_t size )
    {
        char const* results[] = {
            check_type< F, double >( data, size ),
            check_type< F, float >( data, size ),
            check_type< F, std::uint8_t >( data, size ),
            check_type< F, std::int8_t >( data, size ),
            check_type< F, std::int16_t >( data, size ),
            check_type< F, std::uint16_t >( data, size ),
            check_type< F, std::int32_t >( data, size ),
            check_type< F, std::uint32_t >( data, size ),
            check_type< F, std::int64_t >( data, size ),
            check_type< F, std::uint64_t >( data, size ),
            check_type< F, std::complex< float > >( data, size ),
            check_type< F, std::complex< double > >( data, size ),
        };
        for ( char const* e : results )
            if ( e ) return e;
        // The native binary format also supports long double and std::complex<long double> (is_binary_element_v);
        // the npy and text parsers do not take them.
        if constexpr ( F == format::binary )
        {
            if ( char const* e = check_type< F, long double >( data, size ) ) return e;
            if ( char const* e = check_type< F, std::complex< long double > >( data, size ) ) return e;
        }
        return nullptr;
    }

    inline char const* check_npy( std::uint8_t const* data, std::size_t size ) { return check_all< format::npy >( data, size ); }
    inline char const* check_text( std::uint8_t const* data, std::size_t size ) { return check_all< format::text >( data, size ); }
    inline char const* check_binary( std::uint8_t const* data, std::size_t size ) { return check_all< format::binary >( data, size ); }

    // BMP has one result type: three equally shaped, non-empty, consistent channels on success.
    inline char const* check_bmp( std::uint8_t const* data, std::size_t size )
    {
        auto const ans = feng::matrix_details::parse_bmp( data, size );
        if ( !ans ) return nullptr;
        auto const& [r, g, b] = *ans;
        if ( !consistent( r ) || !consistent( g ) || !consistent( b ) ) return "a successful BMP parse left an inconsistent channel";
        if ( r.row() != g.row() || r.row() != b.row() || r.col() != g.col() || r.col() != b.col() )
            return "a successful BMP parse returned channels of different shapes";
        if ( r.size() == 0 ) return "a successful BMP parse returned empty channels";
        return nullptr;
    }
}

#endif
