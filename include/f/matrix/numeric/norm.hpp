#ifndef MNORM_HPP_INCLUDED_DFSOIJ3498USLKFJDKLSJFDI84UIAFSDIOU4KLJHSAFDOIUJH4938YUSFAKJDHVC879UY4EKD
#define MNORM_HPP_INCLUDED_DFSOIJ3498USLKFJDKLSJFDI84UIAFSDIOU4KLJHSAFDOIUJH4938YUSFAKJDHVC879UY4EKD

#include "./eigen_power_iteration.hpp"

namespace f
{
    // return ||A|| _{inf}
    template< typename Matrix >
    typename Matrix::value_type
    norm( const Matrix& A )
    {
        typedef typename Matrix::value_type value_type;
        std::vector<value_type> m( A.row() );

        for ( std::size_t i = 0; i != A.row(); ++i )
            m[i] = std::accumulate( A.row_cbegin( i ), A.row_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }

    template< typename Matrix >
    typename Matrix::value_type
    norm( const Matrix& A, const std::size_t n )
    {
        typedef typename Matrix::value_type value_type;

        if ( 1 == n )
        {
            std::vector<value_type> m( A.col() );

            for ( std::size_t i = 0; i != A.col(); ++i )
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

        assert( !"norm:: other norm algorithm has not been implemented!" );
        return  value_type( 0 );
    }

    template< typename Matrix >
    typename Matrix::value_type
    norm_1( const Matrix& A )
    {
        typedef typename Matrix::value_type value_type;
        std::vector<value_type> m( A.col() );

        for ( std::size_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }

    template< typename T, typename A_ >
    T
    norm_1( const matrix<std::complex<T>, A_>& A )
    {
        std::vector<T> m( A.col() );

        for ( std::size_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), T( 0 ), []( const T u, const std::complex<T>& v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }

    template< typename Matrix >
    typename Matrix::value_type
    norm_2( const Matrix& A )
    {
        return std::sqrt( eigen_power_iteration( A ) );
    }

}//namespace f

#endif//_NORM_HPP_INCLUDED_DFSOIJ3498USLKFJDKLSJFDI84UIAFSDIOU4KLJHSAFDOIUJH4938YUSFAKJDHVC879UY4EKD

