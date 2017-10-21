#ifndef KLNTJGBBWUPCXJSMJEWHLREDLPCMYXFEFLWIPGVFYSKYXFEUPQFWHXIFFBVBQTFRVWRXLMHNO
#define KLNTJGBBWUPCXJSMJEWHLREDLPCMYXFEFLWIPGVFYSKYXFEUPQFWHXIFFBVBQTFRVWRXLMHNO

namespace f
{

    namespace ifft_private
    {
        template< typename T >
        struct add_complex
        {
            typedef std::complex<T> result_type;
        };

        template< typename T >
        struct add_complex< std::complex<T>>
        {
            typedef std::complex<T> result_type;
        };
    }

    // TODO: adapt ifft algorithm
    template< typename T >
    auto ifft( matrix<T> const& x )
    {
        typedef typename ifft_private::add_complex<T>::result_type complex_type;
        matrix<complex_type> X( x.row(), x.col() );
        auto make_omege = []( auto k, auto n, auto N )
        {
            double const pi = 3.1415926535897932384626433;
            double const theta = pi * 2.0 * k * n / static_cast<double>( N );
            return complex_type{ std::cos( theta ), std::sin( theta ) };
        };
        unsigned long const R = X.row();
        unsigned long const C = X.col();

        for ( unsigned long r = 0; r != R; ++r )
            for ( unsigned long c = 0; c != C; ++c )
            {
                complex_type X_rc{ 0.0, 0.0 };

                for ( unsigned long r_ = 0; r_ != R; ++r_ )
                {
                    complex_type tmp{ 0.0, 0.0 };

                    for ( unsigned long c_ = 0; c_ != C; ++c_ )
                        tmp += x[r][c] * make_omege( c, c_, C );

                    X_rc += tmp * make_omege( r, r_, R );
                }

                X[r][c] = X_rc;
            }

        return X;
    }


}//namespace f

#endif//KLNTJGBBWUPCXJSMJEWHLREDLPCMYXFEFLWIPGVFYSKYXFEUPQFWHXIFFBVBQTFRVWRXLMHNO

