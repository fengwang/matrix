#ifndef MEIGEN_HERMITIAN_HPP_INCLUDED_DSOIJ4398UASFKLDJSKFLDJKDJVCMNSDJKAKLJFLLJDOIJH4IIDF89498FJSDF
#define MEIGEN_HERMITIAN_HPP_INCLUDED_DSOIJ4398UASFKLDJSKFLDJKDJVCMNSDJKAKLJFLLJDOIJH4IIDF89498FJSDF

#include "../details/algorithm/for_each.hpp"
#include "./eigen_real_symmetric.hpp"
#include "./math.hpp"

namespace f
{
    /* Input:
     *          A       Symmetric dense matrix
     *          V       Eigen Vectors
     *          Lambda  Eigen Values
     * Function:
     *          calculate the eigen vectors V and eigen values lambda using Jacobi method
     * Returns:
     *          the iterations used
     */

    template< typename T1, typename A1, typename T2, typename A2, typename Otor, typename T = double>
    void eigen_hermitian_impl( const matrix<std::complex<T1>, A1>& A, matrix<std::complex<T2>, A2>& V, Otor o, const T eps = T( 1.0e-20 ) )
    {
        assert( A.row() == A.col() );
        std::size_t const n = A.row();
        auto const A_ = real( A );
        auto const B_ = imag( A );
        auto const AA =   ( A_ || ( -B_ ) ) && ( B_ || A_    );
        matrix<T1, A1> VV( n + n, n + n );
        matrix<T1, A1> LL( n + n, n + n );
        eigen_real_symmetric( AA, VV, LL, eps );
        std::vector<T1, A1> vec( n + n );
        std::copy( LL.diag_begin(), LL.diag_end(), vec.begin() );
        std::sort( vec.begin(), vec.end() );
        V.resize( n, n );

        for ( std::size_t i = 0; i != n;  ++i )
        {
            std::size_t const offset = std::distance( LL.diag_begin(), std::find( LL.diag_begin(), LL.diag_end(), vec[i + i] ) );
            assert( offset < n + n );
            for_each( V.col_begin( i ), V.col_end( i ), VV.col_begin( offset ), []( std::complex<T2>& c, T1 const r )
            {
                c.real( r );
            } );
            for_each( V.col_begin( i ), V.col_end( i ), VV.col_begin( offset ) + n, []( std::complex<T2>& c, T1 const i )
            {
                c.imag( i );
            } );
            *o++ = vec[i + i];
        }
    }//eigen_hermitian_impl

    template<typename Complex_Matrix1, typename Complex_Matrix2, typename T = double>
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::vector<T>& Lambda, const T eps = T( 1.0e-20 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }

    template<typename Complex_Matrix1, typename Complex_Matrix2, typename T = double>
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::valarray<T>& Lambda, const T eps = T( 1.0e-20 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }

    template<typename Complex_Matrix1, typename Complex_Matrix2, typename T, typename A_, typename T_ = double>
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, matrix<T, A_>& Lambda, const T_ eps = T_( 1.0e-20 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_hermitian_impl( A, V, Lambda.diag_begin(), eps );
    }

}//namespace f

#endif//_EIGEN_HERMITIAN_HPP_INCLUDED_DSOIJ4398UASFKLDJSKFLDJKDJVCMNSDJKAKLJFLLJDOIJH4IIDF89498FJSDF

