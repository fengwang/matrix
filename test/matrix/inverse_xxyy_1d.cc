#include <f/matrix/matrix.hpp>
#include <f/algorithm/for_each.hpp>

#include <map>
#include <sstream>
#include <string>
#include <iomanip>

void calculate_Atb( std::string const& At_path, double* b_ptr, double* Atb_ptr, unsigned long N );

void normalize( f::matrix<double>& A )
{
    double const mx = *std::max_element( A.begin(), A.end() );
    double const mn = *std::min_element( A.begin(), A.end() );
    A -= mn;
    A /= mx - mn;
}

int main()
{
    using namespace f;
    matrix<double> orig;
    matrix<double> xx;
    matrix<double> yy;
    matrix<double> xxiv;
    matrix<double> yyiv;
    matrix<double> iv;
    matrix<double> iv_diff;

    if (1)
    {
        orig.load("./matrix/stone.txt");
        normalize( orig );
        orig.save_as( "stone_orig.txt" );

        xx.load("./matrix/stone_xx.txt");

        yy.load("./matrix/stone_yy.txt");

        iv.load("./matrix/laplace_1d_inverse.txt");

        assert( xx.row() == yy.row() );
        assert( xx.col() == yy.col() );
        assert( xx.row() == iv.row() );
        assert( xx.col() == iv.col() );

        auto const& xxiv_t = iv * xx.transpose();
        xxiv = xxiv_t.transpose();
        normalize( xxiv );
        xxiv.save_as("stone_xxiv.txt");
        xxiv.save_as_pgm("stone_xxiv.pgm");
        std::cerr << "XXIV solved\n" << std::endl;
        iv_diff = xxiv - orig;
        iv_diff.save_as_pgm("stone_xxiv_diff.pgm");
        std::for_each( iv_diff.begin(), iv_diff.end(), []( auto& x ){ x = std::abs(x); } );
        std::cout << "Total Error for XXIV is " << std::inner_product( iv_diff.begin(), iv_diff.end(), iv_diff.begin(), 0.0 ) / std::inner_product( orig.begin(), orig.end(), orig.begin(), 0.0 ) << std::endl;

        yyiv = iv * yy;
        normalize( yyiv );
        yyiv.save_as("stone_yyiv.txt");
        yyiv.save_as_pgm("stone_yyiv.pgm");
        std::cerr << "YYIV solved\n" << std::endl;
        iv_diff = yyiv - orig;
        iv_diff.save_as_pgm("stone_yyiv_diff.pgm");
        std::for_each( iv_diff.begin(), iv_diff.end(), []( auto& x ){ x = std::abs(x); } );
        std::cout << "Total Error for YYIV is " << std::inner_product( iv_diff.begin(), iv_diff.end(), iv_diff.begin(), 0.0 ) / std::inner_product( orig.begin(), orig.end(), orig.begin(), 0.0 ) << std::endl;
    }

    // calc A^T b
    iv_diff = yyiv - xxiv;
    unsigned long const N = iv.row();
    matrix<double> Atb{ N+N,1 };
    calculate_Atb( "./matrix/N_N.mtx", iv_diff.data(), Atb.data(), N ); // Atb = At * iv_diff
    std::cerr << "A^T b calculated\n" << std::endl;

    //load AtA ^ -1
    matrix<double> AtA_inverse;
    AtA_inverse.load("./matrix/AtA_inv_2N.txt");
    assert( AtA_inverse.row() == AtA_inverse.col() );
    assert( N+N == AtA_inverse.row() );
    std::cerr << "AtA^-1 loaded\n" << std::endl;

    // alpha beta = AtA^{-1} A^T b
    assert( AtA_inverse.col() == Atb.row() && "AtA_inverse and Atb dimension does not match with operator *!" );
    matrix<double> alpha_beta = AtA_inverse * Atb; // dim is [1, N+N]
    std::cerr << "alpha beta calculated\n" << std::endl;

    // back to model
    // XX[m][n] + \alpha[m] -- YY[m][n] + \beta[n]
    alpha_beta.reshape( 2, N );
    for ( unsigned long r = 0; r != N; ++r )
    {
        double const alpha = alpha_beta[0][r];
        double const beta = alpha_beta[1][r];
        std::for_each( xxiv.row_begin(r), xxiv.row_end(r), [alpha]( double& v ){ v += alpha; } );
        std::for_each( yyiv.col_begin(r), yyiv.col_end(r), [beta]( double& v ){ v += beta; } );
    }
    std::cerr << "XXIV YYIV patched\n" << std::endl;

    // Avarage out
    iv = ( xxiv + yyiv ) / 2.0;
    normalize(iv);
    iv.save_as("stone_iv.txt");
    iv.save_as_pgm("stone_iv.pgm");
    std::cerr << "XXIV YYIV averaged\n" << std::endl;
    iv_diff = iv - orig;
    std::for_each( iv_diff.begin(), iv_diff.end(), []( auto& x ){ x = std::abs(x); } );
    std::cout << "Total Error for Patched is " << std::inner_product( iv_diff.begin(), iv_diff.end(), iv_diff.begin(), 0.0 ) / std::inner_product( orig.begin(), orig.end(), orig.begin(), 0.0 ) << std::endl;
    iv_diff.save_as_pgm("stone_average_diff.pgm");

    iv_diff = xxiv - yyiv;
    std::for_each( iv_diff.begin(), iv_diff.end(), []( auto& x ){ x = std::abs(x); } );
    iv_diff.save_as("stone_iv_diff.txt");
    iv_diff.save_as_pgm("stone_fitted_diff.pgm");
    std::cerr << "XXIV YYIV diffed\n" << std::endl;

    std::cout << "Total Error is " << std::inner_product( iv_diff.begin(), iv_diff.end(), iv_diff.begin(), 0.0 ) / std::inner_product( orig.begin(), orig.end(), orig.begin(), 0.0 ) << std::endl;


    return 0;
}
