#ifndef _EIGEN_REAL_SYMMETRIC_HPP_INCLUDED_SODIJ498USLKFDJASOIFJVC9UDGJI58UGDJIRIOOIGDUGGUGOIUDFLKJS
#define _EIGEN_REAL_SYMMETRIC_HPP_INCLUDED_SODIJ498USLKFDJASOIFJVC9UDGJI58UGDJIRIOOIGDUGGUGOIUDFLKJS

#include <matrix/matrix.hpp>
#include <matrix/numeric/householder.hpp>
#include <matrix/numeric/eigen_jacobi.hpp>

#include <cassert>
#include <cstddef>
#include <cmath>
#include <vector>
#include <valarray>

namespace feng
{
    /* Input:
     *          A       Real Symmetric dense matrix
     *          V       Eigen Vectors
     *          Lambda  Eigen Values
     * Function:
     *          calculate the eigen vectors V and eigen values lambda using Householder Transformation and Jacobi method
     */
    template<typename Matrix1, typename Matrix2, typename T = double>
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::vector<T>& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps );
    }

    template<typename Matrix1, typename Matrix2, typename T = double>
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::valarray<T>& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps);
    }

    template<typename Matrix1, typename Matrix2, typename T, std::size_t D, typename A_, typename T_=double>
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, feng::matrix<T,D,A_>& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T(0);
        return eigen_real_symmetric( A, V, Lambda.diag_begin(), eps);
    }

    template< typename Matrix1, typename Matrix2, typename Otor, typename T = double>
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        assert( A.row() == A.col() );
        //assert( is_symmetric( A ) );

        Matrix1 D(A);
        Matrix1 Q(A);
        householder( A, Q, D);
        eigen_jacobi( D, V, o, eps );
        
        V = Q*V;
    }//eigen_real_symmetric

}//namespace feng

#endif//_EIGEN_REAL_SYMMETRIC_HPP_INCLUDED_SODIJ498USLKFDJASOIFJVC9UDGJI58UGDJIRIOOIGDUGGUGOIUDFLKJS

