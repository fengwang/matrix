#include <cmath>

auto make_mandelbrot( std::complex<double> const& lower_left, std::complex<double> const& upper_right, unsigned long const dim = 1024, unsigned long const iterations = 1024 )
{
    std::complex<double> spacing_ratio{ (std::real(upper_right)-std::real(lower_left)) / static_cast<double>(dim),
                                        (std::imag(upper_right)-std::imag(lower_left)) / static_cast<double>(dim) };
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
    return mat;
}

void _0000_mandelbrot()
{
    auto const& mat = make_mandelbrot( std::complex<double>{-2.25, -1.5}, std::complex<double>{0.75, 1.5} );
    mat.save_as_bmp( "./images/0000_mandelbrot.bmp", "gray" );
}

void _0001_mandelbrot()
{
    unsigned long const dims = 1024;
    unsigned long const iterations = 1024;
    auto&& mat = make_mandelbrot( std::complex<double>{-2.0, -1.25}, std::complex<double>{0.5, 1.25}, dims, iterations );
    mat.save_as_bmp( "./images/0001_mandelbrot.bmp", "bluehot" );
    mat /= static_cast<double>( iterations );
    std::cout << "Var(mandelbrot) = " << feng::variance( mat );
}

void _0002_mandelbrot()
{
    //double const x_start = -2.0;
    //double const y_start = -1.25;
    //double const length = 2.5;
    //double const x_start = -0.671875;
    //double const y_start = -0.5078125;
    double const x_start = -0.75;
    double const y_start = -0.2734375;
    double const length = 0.0390625;
    //unsigned long const dims = 512;
    unsigned long const dims = 4096;
    unsigned long const iterations = 880;
    unsigned long const samplings = 32;
    double const spacing = length / static_cast<double>(samplings);

    for ( auto r = 0UL; r != samplings; ++r )
        for ( auto c = 0UL; c != samplings; ++c )
        {
            double x = x_start + spacing * r;
            double y = y_start + spacing * c;
            auto&& mat = make_mandelbrot( std::complex<double>{x, y}, std::complex<double>{x+spacing, y+spacing}, dims, iterations );
            //std::for_each( mat.begin(), mat.end(), [](double& v){ v = std::sqrt((v-79)/800); } );
            //std::string const file_name = std::string{ "./images/mandelbrot_4/0002_mandel_brot_" } + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            std::string const file_name = std::string{ "./images/mandelbrot_5/0002_mandel_brot_" } + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            mat.save_as_bmp( file_name, "bluehot" );

        }
}


