#ifndef _EXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH
#define _EXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH

#include <matrix/matrix.hpp>
#include <matrix/numeric/norm.hpp>
#include <matrix/misc/eye.hpp>

#include <cassert>
#include <cmath>

#include <iostream>

namespace feng
{
    namespace expm_private
    {
    template<typename T>
    struct fix_complex_value_type
    {
        typedef T value_type; 
    };

    template<typename T>
    struct fix_complex_value_type<std::complex<T> >
    {
        typedef typename fix_complex_value_type<T>::value_type value_type;
    };
    }//namespace expm_private

    // Nicholas J. Higham. The Scaling and Squaring Method for the Matrix Exponential Revisited. SIAM Review (2009) 51: pp. 747-764.
    template<typename T, std::size_t D, typename A_>
    const matrix<T, D, A_>
    expm( const matrix<T, D, A_>& A )
    {
        typedef matrix<T, D, A_>                                                        matrix_type;
        typedef typename matrix_type::value_type                                        value_type_;
        typedef typename expm_private::fix_complex_value_type<value_type_>::value_type  value_type;
        typedef typename matrix_type::size_type                                         size_type;
        assert( A.row() == A.col() );
        static const value_type theta[] = { 0.000000000000000e+000,
                                            3.650024139523051e-008,
                                            5.317232856892575e-004,
                                            1.495585217958292e-002, 
                                            8.536352760102745e-002,
                                            2.539398330063230e-001,
                                            5.414660951208968e-001,
                                            9.504178996162932e-001,
                                            1.473163964234804e+000,
                                            2.097847961257068e+000,
                                            2.811644121620263e+000,
                                            3.602330066265032e+000,
                                            4.458935413036850e+000,
                                            5.371920351148152e+000
                                          };
        auto const norm_A               = norm_1( A );
        auto const ratio                = theta[13] / norm_A;
        size_type const s               = ratio < value_type(1) ? 0 : static_cast<size_type>( std::ceil( std::log2( ratio ) ) );
        const value_type s__2           = s ? value_type(1 << s) : value_type(1);
        auto const _A                   = A / s__2;
        auto const n                    = _A.row();
        static value_type const c []    = { 0.000000000000000,  // 0
                                            64764752532480000,  // 1
                                            32382376266240000,  // 2
                                            7771770303897600,   // 3
                                            1187353796428800,   // 4
                                            129060195264000,    // 5
                                            10559470521600,     // 6
                                            670442572800,       // 7
                                            33522128640,        // 8
                                            1323241920,         // 9
                                            40840800,           // 10
                                            960960,             // 11
                                            16380,              // 12
                                            182,                // 13
                                            1                   // 14
                                        };                 
        auto const _A2 = _A * _A;
        auto const _A4 = _A2 * _A2;
        auto const _A6 = _A2 * _A4;
        auto const U   = _A * ( _A6 * ( c[14] * _A6 + c[12] * _A4 + c[10] * _A2 ) + c[8] * _A6 + c[6] * _A4 + c[4] * _A2 + c[2] * eye<value_type>( n, n ) );
        auto const V   = _A6 * ( c[13] * _A6 + c[11] * _A4 + c[9] * _A2 ) + c[7] * _A6 + c[5] * _A4 + c[3] * _A2 + c[1] * eye<value_type>( n, n );
        auto       F   = ( -U + V ).inverse() * ( U + V );
        for ( size_type i = 0; i != s; ++i )
            F *= F;
        return F;
    }
}//namespace feng

#endif//_EXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH

