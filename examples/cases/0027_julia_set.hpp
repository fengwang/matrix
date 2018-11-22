#include <cmath>

#if 0
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
#endif

auto make_julia_set( std::complex<double> const& lower_left, std::complex<double> const& upper_right, std::complex<double>const& cval, unsigned long const dim = 1024, unsigned long const iterations = 1024, unsigned long const powers = 2 )
{
    std::complex<double> spacing_ratio{ (std::real(upper_right)-std::real(lower_left)) / static_cast<double>(dim),
                                        (std::imag(upper_right)-std::imag(lower_left)) / static_cast<double>(dim) };

    feng::matrix<std::complex<double>> cmat{ dim, dim };
    for ( auto r = 0UL; r != dim; ++r )
        for ( auto c = 0UL; c != dim; ++c )
            cmat[r][c] = std::complex<double>{ static_cast<double>(r), static_cast<double>(c) };

    auto const& converter = [&spacing_ratio, &lower_left, &upper_right, iterations, powers, &cval]( std::complex<double>& c )
    {
        std::complex<double> z = lower_left + std::complex<double>{ std::real(spacing_ratio)*std::real(c), std::imag(spacing_ratio)*std::imag(c) };
        c = std::complex<double>{ static_cast<double>(iterations), 0 };
        for ( auto idx = 0UL; idx != iterations; ++idx )
        {
            z = std::pow( z, powers ) + cval;
            if ( std::abs(z) > 2.0 )
            {
                c = std::complex<double>{ static_cast<double>(idx), 0 };
                break;
            }
        }
    };

    cmat.apply( converter );
    return feng::real(cmat);
}

void _0000_julia_set()
{
    auto&&  mat = make_julia_set( std::complex<double>{-1.5, -1.0}, std::complex<double>{1.5, 1.0}, std::complex<double>{-0.4, 0.6} );
    mat.apply( [](auto& x){ x = std::log(1.0+x); } );
    mat.save_as_bmp( "./images/0000_julia_set.bmp", "tealhot" );
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
            auto&&  mat = make_julia_set( std::complex<double>{-1.5, -1.0}, std::complex<double>{1.5, 1.0}, zc, 1024, 1024, 3 );
            //mat.apply( [](auto& x){ x = std::log(1.0+x); } );
            //std::string file_name = std::string{"./images/julia_set/0001_julia_set_"} + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            //std::string file_name = std::string{"./images/julia_set_2/0001_julia_set_"} + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            std::string file_name = std::string{"./images/julia_set_3/0001_julia_set_"} + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            mat.save_as_bmp( file_name, "bluehot" );
        }
}

void _0002_julia_set()
{
    //-0.9, 0.9
    unsigned long n = 32;
    for ( unsigned r = 0; r != n; ++r )
        for ( unsigned c = 0; c != n; ++c )
        {
            std::cout << "Generating Julia set at the index of (" << r << "/" << n << ", " << c << "/" << n << ")." << std::endl;
            std::complex<double> zc{ double(r)/n*1.8-0.9, double(c)/n*1.8-0.9 };
            auto&&  mat = make_julia_set( std::complex<double>{-1.5, -1.0}, std::complex<double>{1.5, 1.0}, zc, 1024, 1024, 4 );
            std::string file_name = std::string{"./images/julia_set_4/0001_julia_set_"} + std::to_string(r) + std::string{"-"} + std::to_string(c) + std::string{".bmp"};
            mat.save_as_bmp( file_name, "tealhot" );
        }
}


