#include <cmath>

auto make_julia_set( std::complex<double> const& lower_left, std::complex<double> const& upper_right, std::complex<double>const& cval, unsigned long const dim = 1024, unsigned long const iterations = 1024, unsigned long const powers = 2 )
{
    std::complex<double> spacing_ratio{ (std::real(upper_right)-std::real(lower_left)) / static_cast<double>(dim),
                                        (std::imag(upper_right)-std::imag(lower_left)) / static_cast<double>(dim) };
    feng::matrix<double> mat{ dim, dim };
    std::fill( mat.begin(), mat.end(), static_cast<double>(iterations) );
    for ( auto r = 0UL; r != dim; ++r )
        for ( auto c = 0UL; c != dim; ++c )
        {
            std::complex<double> z = lower_left + std::complex<double>{ std::real(spacing_ratio)*static_cast<double>(r), std::imag(spacing_ratio)*static_cast<double>(c) };
            for ( auto idx = 0UL; idx != iterations; ++idx )
            {
                //z = z * z + cval;
                z = std::pow( z, powers ) + cval;
                if ( std::abs(z) > 2.0 )
                {
                    mat[r][c] = idx;
                    break;
                }
            }
        }
    return mat;
}

void _0000_julia_set()
{
    auto&&  mat = make_julia_set( std::complex<double>{-1.5, -1.0}, std::complex<double>{1.5, 1.0}, std::complex<double>{-0.4, 0.6} );
    mat.apply( [](auto& x){ x = std::log(1.0+x); } );
    mat.save_as_bmp( "./images/0000_julia_set.bmp", "bluehot" );
}

void _0001_julia_set()
{
    //-0.9, 0.9
    unsigned long n = 32;
    for ( unsigned r = 0; r != n; ++r )
        for ( unsigned c = 0; c != n; ++c )
        {
            std::cout << "Generating Julia set at the index of (" << r << "/" << n << ", " << c << "/" << n << ")." << std::endl;
            std::complex<double> zc{ double(r)/n*1.8-0.9, double(c)/n*1.8-0.9 };
            auto&&  mat = make_julia_set( std::complex<double>{-1.5, -1.0}, std::complex<double>{1.5, 1.0}, zc );
            mat.apply( [](auto& x){ x = std::log(1.0+x); } );
            std::string file_name = std::string{"./images/julia_set/0001_julia_set_"} + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            mat.save_as_bmp( file_name, "bluehot" );
        }
}

