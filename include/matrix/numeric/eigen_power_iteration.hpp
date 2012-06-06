#ifndef _EIGEN_POWER_ITERATION_HPP_INCLUDED_DFSKLJWEOIJHASFLKFASODIJ34098USLAKFDJCLVKJSFODAIJI4UJELK
#define _EIGEN_POWER_ITERATION_HPP_INCLUDED_DFSKLJWEOIJHASFLKFASODIJ34098USLAKFDJCLVKJSFODAIJI4UJELK

#include <matrix/matrix.hpp>

#include <cstddef>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace feng
{

    template< typename T, std::size_t D, typename A_, typename O >
    T eigen_power_iteration( const matrix<T,D,A_>& A, O output, const T eps = T(1.0e-5) )
    {
        assert( A.row() == A.col() );

        matrix<T,D,A_> b( A.col(), 1 );
        std::copy( A.diag_cbegin(), A.diag_cend(), b.begin() ); // random initialize

        for (;;)
        {
            auto const old_b = b;
            b = A*b;
            auto const u    = std::inner_product(b.begin(), b.end(), b.begin(), T(0));
            auto const norm = std::sqrt(u);
            b /= norm;
            auto const U    = std::inner_product(b.begin(), b.end(), b.begin(), T(0));
            auto const V    = std::inner_product(old_b.begin(), old_b.end(), old_b.begin(), T(0));
            auto const UV   = std::inner_product(b.begin(), b.end(), old_b.begin(), T(0));

            if ( UV * UV > U*V*(T(1)-eps) )
            {
                std::copy( b.begin(), b.end(), output );
                return norm;
            }
        }

        assert( !"eigen_power_iteration:: should never reach here!" );
        return T(0); //just to kill warnings
    }

    template< typename T, std::size_t D, typename A_ >
    T eigen_power_iteration( const matrix<T,D,A_>& A, const T eps = T(1.0e-5) )
    {
        matrix<T,D,A_> b( A.col(), 1 );
        return eigen_power_iteration(A, b.begin(), eps);
    }

    template< typename T, std::size_t D, typename A_, typename O >
    T eigen_power_iteration( const matrix<std::complex<T>,D,A_>& A, O output, const T eps = T(1.0e-5) )
    {
        assert( A.row() == A.col() );

        matrix<std::complex<T>,D,A_> b( A.col(), 1 );
        matrix<std::complex<T>,D,A_> b_( A.col(), 1 );
        std::copy( A.diag_cbegin(), A.diag_cend(), b.begin() ); // random initialize

        matrix<std::complex<T>, D,A> A_(A);
        std::for_each( A_.begin(), A_.end(), [](std::complex<T>& c) { c = std::conj(c) } );

        for (;;)
        {
            auto const old_b = b;
            b = A_*b;
            std::transform( b.begin(), b.end(), b_.begin(), [](std::complex<T> v) { return std::conj(v); } );
            auto const u_   = std::inner_product(b.begin(), b.end(), b_.begin(), std::complex<T>(0,0));
            auto const u    = real(u_);
            auto const norm = std::sqrt(u);
            b /= norm;
            std::transform( b.begin(), b.end(), b_.begin(), [](std::complex<T> v) { return std::conj(v); } );
            auto const U_   = std::inner_product(b.begin(), b.end(), b_.begin(), std::complex<T>(0,0));
            auto const U    = real(U_);
            std::transform( old_b.begin(), old_b.end(), b_.begin(), [](std::complex<T> v) { return std::conj(v); } );
            auto const V_   = std::inner_product(old_b.begin(), old_b.end(), b.begin(), std::complex<T>(0,0));
            auto const V    = real(V_);
            auto const UV_  = std::inner_product(b.begin(), b.end(), b_.begin(), std::complex<T>(0,0));
            auto const UV   = real(UV_);

            if ( UV * UV > U*V*(T(1)-eps) )
            {
                std::copy( b.begin(), b.end(), output );
                return norm;
            }
        }

        assert( !"eigen_power_iteration:: should never reach here!" );
        return T(0); //just to kill warnings
    }

    template< typename T, std::size_t D, typename A_ >
    T eigen_power_iteration( const matrix<std::complex<T>,D,A_>& A, const T eps = T(1.0e-5) )
    {
        matrix<std::complex<T>,D,A_> b( A.col(), 1 );
        return eigen_power_iteration(A, b.begin(), eps);
    }

}//namespace feng

#endif//_EIGEN_POWER_ITERATION_HPP_INCLUDED_DFSKLJWEOIJHASFLKFASODIJ34098USLAKFDJCLVKJSFODAIJI4UJELK

