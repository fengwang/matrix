#ifndef MEXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH
#define MEXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH

#include "./norm.hpp"

namespace f
{
    namespace expm_private
    {
        template<typename T>
        struct fix_complex_value_type
        {
            typedef T value_type;
        };

        template<typename T>
        struct fix_complex_value_type<std::complex<T>>
        {
            typedef typename fix_complex_value_type<T>::value_type value_type;
        };
    }//namespace expm_private

    // Nicholas J. Higham. The Scaling and Squaring Method for the Matrix Exponential Revisited. SIAM Review (2009) 51: pp. 747-764.
    template<typename T, typename A_>
    const matrix<T, A_>
    expm( const matrix<T, A_>& A )
    {
        typedef matrix<T, A_>                                                           matrix_type;
        typedef typename matrix_type::value_type                                        value_type_;
        typedef typename expm_private::fix_complex_value_type<value_type_>::value_type  value_type;
        typedef typename matrix_type::size_type                                         size_type;
        assert( A.row() == A.col() );
        static const value_type theta[] = { 0.000000000000000e+000, //0
                                            3.650024139523051e-008, //1
                                            5.317232856892575e-004, //2
                                            1.495585217958292e-002, //3
                                            8.536352760102745e-002, //4
                                            2.539398330063230e-001, //5
                                            5.414660951208968e-001, //6
                                            9.504178996162932e-001, //7
                                            1.473163964234804e+000, //8
                                            2.097847961257068e+000, //9
                                            2.811644121620263e+000, //10
                                            3.602330066265032e+000, //11
                                            4.458935413036850e+000, //12
                                            5.371920351148152e+000  //13
                                          };
        value_type const norm_A               = norm_1( A );
        //value_type const ratio                = theta[13] / norm_A;
        value_type const ratio                = norm_A / theta[13];
        size_type const s               = ratio < value_type( 1 ) ? 0 : static_cast<size_type>( std::ceil( std::log2( ratio ) ) );
        value_type const s__2           = s ? value_type( 1 << s ) : value_type( 1 );
        matrix_type const& _A                   = A / s__2;
        size_type const n                    = _A.row();
        static value_type const c []    = { 0.000000000000000,  // 0
                                            //64764752532480000,  // 1
                                            6.4764752532480000e+16,  // 1
                                            //32382376266240000,  // 2
                                            3.2382376266240000e+16,  // 2
                                            //7771770303897600,   // 3
                                            7.771770303897600e+15,   // 3
                                            //1187353796428800,   // 4
                                            1.187353796428800e+15,   // 4
                                            //129060195264000,    // 5
                                            1.29060195264000e+14,    // 5
                                            //10559470521600,     // 6
                                            1.0559470521600e+13,     // 6
                                            //670442572800,       // 7
                                            6.70442572800e+11,       // 7
                                            //33522128640,        // 8
                                            3.3522128640e+10,        // 8
                                            //1323241920,         // 9
                                            1.323241920e+9,         // 9
                                            //40840800,           // 10
                                            4.0840800e+7,           // 10
                                            //960960,             // 11
                                            9.60960e+5,             // 11
                                            //16380,              // 12
                                            1.6380e+4,              // 12
                                            //182,                // 13
                                            1.82e+2,                // 13
                                            1                   // 14
                                          };
        matrix_type const& _A2 = _A * _A;
        matrix_type const& _A4 = _A2 * _A2;
        matrix_type const& _A6 = _A2 * _A4;
        matrix_type const& U = _A * ( _A6 * ( c[14] * _A6 + c[12] * _A4 + c[10] * _A2 ) + c[8] * _A6 + c[6] * _A4 + c[4] * _A2 + c[2] * eye<value_type>( n, n ) );
        matrix_type const& V = _A6 * ( c[13] * _A6 + c[11] * _A4 + c[9] * _A2 ) + c[7] * _A6 + c[5] * _A4 + c[3] * _A2 + c[1] * eye<value_type>( n, n );
        matrix_type const& VU = V + U;
        matrix_type const& UV = V - U;
        matrix_type F = VU / UV;

        for ( size_type i = 0; i != s; ++i )
            F *= F;

        return F;
    }
}//namespace f

#endif//_EXPM_HPP_INCLUDED_SDFOIJ3498USFDKJDSVJSFD9IH498HSFDKJHVKJSFADUH4UHKSFADJHIEURHSFKDJHEIUHFUH

