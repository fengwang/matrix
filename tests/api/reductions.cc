// S1-R4 API family `reductions`: sum, mean, variance, standard_deviation, min, max, minmax, norm, norm_1, norm_2, tr,
// is_* predicates, isequal. Compiled alone by `tools/check.sh api`; calling each API instantiates its template.
#include "../../matrix.hpp"

#include <complex>

namespace
{
    template < typename T >
    T exercise_sums()
    {
        // sum, norm_1, tr: every element type
        feng::matrix<T> const m{ 3, 3, T{ 2 } };
        T acc{};
        acc += feng::sum( m );
        acc += static_cast<T>( feng::norm_1( m ) );
        acc += feng::tr( m );
        acc += m.tr();
        return acc;
    }

    template < typename T >
    T exercise_statistics()
    {
        feng::matrix<T> const m{ 3, 3, T{ 2 } };
        T acc{};
        acc += feng::variance( m );
        acc += feng::standard_deviation( m );
        acc += feng::norm_2( m );
        return acc;
    }

    template < typename T >
    T exercise_ordered()
    {
        // min, max, minmax, mean, ordering operators: ordered element types
        feng::matrix<T> const m{ 3, 3, T{ 2 } };
        auto const less = []( T const& x, T const& y ) { return x < y; };
        T acc{};
        acc += feng::min( m );
        acc += feng::max( m );
        acc += feng::mean( m ); // complex mean compiles since S6 (returns std::complex<X>)
        acc += m.min();
        acc += m.max();
        acc += m.min( less );
        acc += m.max( less );
        auto const [lo, hi] = m.minmax();
        auto const [lo2, hi2] = m.minmax( less );
        acc += lo + hi + lo2 + hi2;
        feng::matrix<T> const n{ m };
        acc += static_cast<T>( ( m < n ) + ( m > n ) + ( m <= n ) + ( m >= n ) );
        return acc;
    }

    template < typename T >
    int exercise_predicates()
    {
        feng::matrix<T> const m = feng::eye<T>( 3, 3 );
        feng::matrix<T> const n{ m };
        int acc{};
        acc += feng::is_column( m ) + feng::is_column_matrix( m ) + feng::iscolumn( m );
        acc += feng::is_row( m ) + feng::is_row_matrix( m ) + feng::isrow( m );
        acc += feng::is_empty( m ) + feng::is_empty_matrix( m ) + feng::isempty( m );
        acc += feng::is_equal( m, n ) + feng::is_equal( m, n, m );
        acc += feng::isequal( m, n ) + feng::isequal( m, n, m );
        acc += feng::is_symmetric( m );
        acc += feng::is_symmetric( m, []( T const& x, T const& y ) { return x == y; } );
        acc += feng::is_orthogonal( m );
        acc += feng::is_orthogonal( m, []( T const& v ) { return v == T{}; } );
        acc += ( m == n );
        return acc;
    }

    template < typename T >
    int exercise_real_predicates()
    {
        feng::matrix<T> const m = feng::eye<T>( 3, 3 );
        int acc{};
        acc += feng::is_positive_definite( m );
        acc += feng::is_inf( m )[0][0] + feng::isinf( m )[0][0];
        acc += feng::is_nan( m )[0][0] + feng::isnan( m )[0][0];
        return acc;
    }

    double exercise_complex()
    {
        feng::matrix<std::complex<double>> const m{ 3, 3, std::complex<double>{ 1.0, 1.0 } };
        double acc{};
        acc += feng::norm_1( m );
        acc += feng::norm( m )[0][0];
        acc += std::real( feng::sum( m ) + feng::tr( m ) );
        return acc;
    }
} // namespace

double api_reductions_double()
{
    return exercise_sums<double>() + exercise_statistics<double>() + exercise_ordered<double>() +
           exercise_predicates<double>() + exercise_real_predicates<double>() + exercise_complex();
}
float api_reductions_float()
{
    return exercise_sums<float>() + exercise_statistics<float>() + exercise_ordered<float>() +
           static_cast<float>( exercise_predicates<float>() + exercise_real_predicates<float>() );
}
std::complex<double> api_reductions_complex()
{
    return exercise_sums<std::complex<double>>() + static_cast<double>( exercise_predicates<std::complex<double>>() );
}
int api_reductions_int() { return exercise_sums<int>() + exercise_ordered<int>() + exercise_predicates<int>(); }
