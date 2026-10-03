// S1-R4 API family `access`: operator[], item, data, shape, size, row/col/diag/anti-diag/direct iterators, and the
// S9-R4 span, mdspan and submdspan adapters.
// Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
#include "../../matrix.hpp"

#include <complex>
#include <iterator>

namespace
{
    template < typename T, typename It >
    T walk( It first, It last )
    {
        T acc{};
        for ( ; first != last; ++first )
            acc += *first;
        return acc;
    }

// Every begin/end flavour of one iterator family, on the mutable and the const matrix, with an index argument.
#define API_ACCESS_FAMILY( prefix, index )                                                                             \
    acc += walk<T>( m.prefix##_begin( index ), m.prefix##_end( index ) );                                              \
    acc += walk<T>( cm.prefix##_begin( index ), cm.prefix##_end( index ) );                                            \
    acc += walk<T>( cm.prefix##_cbegin( index ), cm.prefix##_cend( index ) );                                          \
    acc += walk<T>( m.prefix##_rbegin( index ), m.prefix##_rend( index ) );                                            \
    acc += walk<T>( cm.prefix##_rbegin( index ), cm.prefix##_rend( index ) );                                          \
    acc += walk<T>( cm.prefix##_crbegin( index ), cm.prefix##_crend( index ) );

    template < typename T >
    T exercise_access()
    {
        feng::matrix<T> m{ 4, 5, T{ 1 } };
        feng::matrix<T> const& cm = m;
        T acc{};

        // element access
        m[1][2] = T{ 2 };
        acc += cm[1][2];
        m( 2, 3 ) = T{ 3 };
        acc += cm( 2, 3 );
        acc += feng::matrix<T>{ 1, 1, T{ 4 } }.item();
        *m.data() = T{ 5 };
        acc += *cm.data();

        // shape and size
        auto const [r, c] = cm.shape();
        acc += static_cast<T>( static_cast<int>( r + c + cm.row() + cm.col() + cm.size() ) );

        // direct iterators
        acc += walk<T>( m.begin(), m.end() );
        acc += walk<T>( cm.begin(), cm.end() );
        acc += walk<T>( cm.cbegin(), cm.cend() );
        acc += walk<T>( m.rbegin(), m.rend() );
        acc += walk<T>( cm.rbegin(), cm.rend() );
        acc += walk<T>( cm.crbegin(), cm.crend() );

        // row, column, diagonal and anti-diagonal iterators
        API_ACCESS_FAMILY( row, 1 )
        API_ACCESS_FAMILY( col, 2 )
        API_ACCESS_FAMILY( diag, 1 )
        API_ACCESS_FAMILY( diag, -1 )
        API_ACCESS_FAMILY( upper_diag, 1 )
        API_ACCESS_FAMILY( lower_diag, 1 )
        API_ACCESS_FAMILY( anti_diag, 1 )
        API_ACCESS_FAMILY( anti_diag, -1 )
        API_ACCESS_FAMILY( upper_anti_diag, 1 )
        API_ACCESS_FAMILY( lower_anti_diag, 1 )

        // the default-index overloads
        acc += walk<T>( m.row_begin(), m.row_end() );
        acc += walk<T>( m.diag_begin(), m.diag_end() );
        acc += walk<T>( m.anti_diag_begin(), m.anti_diag_end() );

        // S9-R4: span adapters, and the mdspan / submdspan adapters where the library has them
        acc += walk<T>( feng::as_span( m ).begin(), feng::as_span( m ).end() );
        acc += walk<T>( feng::as_span( cm ).begin(), feng::as_span( cm ).end() );
        acc += feng::row_span( m, 1 )[0] + feng::row_span( cm, 2 )[1];
#if defined( __cpp_lib_mdspan )
        acc += feng::to_mdspan( m )[1, 2] + feng::to_mdspan( cm )[2, 3];
#endif
#if defined( __cpp_lib_submdspan )
        acc += feng::submdspan( m, { 1, 3 }, { 0, 2 } )[0, 1] + feng::submdspan( cm, { 0, 1 }, { 0, 5 } )[0, 4];
#endif
        return acc;
    }

#undef API_ACCESS_FAMILY
} // namespace

double api_access_double() { return exercise_access<double>(); }
float api_access_float() { return exercise_access<float>(); }
std::complex<double> api_access_complex() { return exercise_access<std::complex<double>>(); }
int api_access_int() { return exercise_access<int>(); }
