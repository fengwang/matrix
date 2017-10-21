#ifndef DEXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH
#define DEXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH

#include "./norm.hpp"

namespace f
{

    template< typename T, typename A >
    void
    dft( matrix< std::complex<T>, A > const& src, matrix< std::complex<T>, A>& dst )
    {
        dst.resize( src.row(), src.col() );
        T const pi = 3.141592653589793;

        for ( std::size_t r = 0; r != dst.row(); ++r )
            for ( std::size_t c = 0; c != dst.col(); ++c )
            {
                std::complex<T> sum = std::complex<T> {0, 0};

                for ( std::size_t k1 = 0; k1 != dst.row(); ++k1 )
                    for ( std::size_t k2 = 0; k2 != dst.col(); ++k2 )
                        sum += std::exp( std::complex<T> {0, 2.0 * pi * ( k2* c / dst.col() + k1* r / dst.row() )} ) * src[k1][k2];
                dst[r][c] = sum;
            }
    }

    template< typename T, typename A >
    matrix< std::complex<T>, A > const
    dft( matrix< std::complex<T>, A > const& src )
    {
        matrix< std::complex<T>, A > dst;
        dft( src, dst );
        return dst;
    }

    //link with -lfftw3
    extern "C"
    {
        typedef struct fftw_plan_s* fftw_plan;
        typedef double fftw_complex[2];
        fftw_plan fftw_plan_dft_2d( int n0, int n1, fftw_complex* in, fftw_complex* out, int sign, unsigned flags );
        void fftw_execute( const fftw_plan p );
    }

    template< typename A >
    void
    fftw( matrix<std::complex<double>, A> const& src, matrix<std::complex<double>, A>& dst )
    {
        int const n0 = static_cast<int>( src.row() );
        int const n1 = static_cast<int>( src.col() );
        dst.resize( src.row(), src.col() );
        fftw_execute( fftw_plan_dft_2d( n0, n1, reinterpret_cast<fftw_complex*>( src.data() ), reinterpret_cast<fftw_complex*>( dst.data() ), -1, 0 ) );
    }

    template< typename A >
    matrix< std::complex<double>, A > const
    fftw( matrix< std::complex<double>, A > const& src )
    {
        matrix< std::complex<double>, A > dst;
        fftw( src, dst );
        return dst;
    }

}//namespace f

#endif//DEXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH

