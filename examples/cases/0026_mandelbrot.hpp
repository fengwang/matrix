#include <cmath>

void _0000_mandelbrot()
{
    unsigned long const dim = 1024;
    unsigned long iterations = 256;
    std::complex<double> lower_left{ -2.25, -1.5 };
    std::complex<double> upper_right{ 0.75, 1.5 };
    std::complex<double> spacing_ratio{ (std::real(upper_right)-std::real(lower_left)) / static_cast<double>(dim),
                                        (std::imag(upper_right)-std::imag(lower_left)) / static_cast<double>(dim) };
    std::cout << "The spacing ratio is " << spacing_ratio << "\n";
    feng::matrix<double> mat{ dim, dim };
    std::fill( mat.begin(), mat.end(), static_cast<double>(iterations) );
    for ( auto r = 0UL; r != dim; ++r )
        for ( auto c = 0UL; c != dim; ++c )
        {
            auto const& cval = lower_left + std::complex<double>{ std::real(spacing_ratio)*static_cast<double>(r), std::imag(spacing_ratio)*static_cast<double>(c) };
            std::complex<double> z{0.0, 0.0};
            for ( auto idx = 0UL; idx != iterations; ++idx )
            {
                z = z * z + cval;
                if ( std::abs(z) > 2.0 )
                {
                    mat[r][c] = idx;
                    break;
                }
            }
        }
    mat.save_as_bmp( "./images/0000_mandelbrot.bmp", "gray" );
}
