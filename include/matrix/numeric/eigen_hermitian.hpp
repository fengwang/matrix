
#include <matrix/matrix.hpp>
#include <matrix/numeric/householder.hpp>
#include <matrix/numeric/eigen_jacobi.hpp>

#include <complex>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <vector>
#include <valarray>

namespace feng
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
    template<typename Matrix1, typename Matrix2, typename T = double>
    void eigen_hermitian( const Matrix1& A, Matrix2& V, std::vector<T>& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian( A, V, Lambda.begin(), eps );
    }

    template<typename Matrix1, typename Matrix2, typename T = double>
    void eigen_hermitian( const Matrix1& A, Matrix2& V, std::valarray<T>& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian( A, V, Lambda.begin(), eps);
    }

    template<typename Matrix1, typename Matrix2, typename T, std::size_t D, typename A_, typename T_=double>
    void eigen_hermitian( const Matrix1& A, Matrix2& V, feng::matrix<T,D,A_>& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T(0);
        return eigen_hermitian( A, V, Lambda.diag_begin(), eps);
    }

    template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2, typename Otor, typename T = double>
    void eigen_hermitian( const matrix<std::complex<T1>,D1, A1>& A, matrix<std::complex<T2>, D2, A2>& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        assert( A.row() == A.col() );

        std::size_t const n = A.row();
        
        matrix<T1,D1, A1> A_( n, n );
        matrix<T1,D1, A1> B_( n, n );

        for_each( A.begin(), A.end(), A_.begin(), B_.begin(), [](const std::complex<T1>& c, T1& a, T1& b) { a=c.real(); b=c.imag() } );

        matrix<T1,D1,A1> AA =   A_ || (-B_) && 
                               (A_ ||   B_);
        




    }//eigen_hermitian

}//namespace feng


