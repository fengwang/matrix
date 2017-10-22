#include <f/matrix/matrix.hpp>
#include <f/matrix/numeric/norm.hpp>
#include <complex>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <cmath>

typedef double value_type;
typedef std::complex<value_type> complex_type;
typedef f::matrix<value_type>   matrix_type;
typedef f::matrix<complex_type> complex_matrix_type;

complex_matrix_type const an( complex_matrix_type const& A, unsigned long n )
{
    if ( 1 == n )
    {
        return A;
    }
    return A * an( A, n - 1 );
}

unsigned long fn( unsigned long n )
{
    if ( 1 == n )
    {
        return 1;
    }
    return n * fn( n - 1 );
}

void compare_expm_12( complex_matrix_type const& A, complex_matrix_type const& S_ref )
{
    unsigned long n = A.row();
    assert( A.col() == n );

    value_type ratio = f::norm_1( A );
    unsigned long s =  ratio < value_type {1} ? 0 : static_cast<unsigned long>( std::ceil( std::log2( ratio ) ) );
    value_type factor = std::pow( value_type {2}, s );
    value_type s__2           = s ? static_cast<value_type>( 1 << s ) : value_type {1};

    complex_matrix_type a = A / s__2;

    complex_type c0 { -4.1356082392646311334114795511234522, - 0.77554204911016202877346518523227687 };
    complex_type c1 { -4.1356082392646311334114795511234522, + 0.77554204911016202877346518523227687} ;
    complex_type c2 { -3.6888971024462053570220817044343043, - 2.3027571492470953276351045107744921};
    complex_type c3 { -3.6888971024462053570220817044343043, + 2.3027571492470953276351045107744921};
    complex_type c4 { -2.7579892311919211464358709785450804, - 3.7525483823537071551232429081271234 };
    complex_type c5 { -2.7579892311919211464358709785450804, + 3.7525483823537071551232429081271234 };
    complex_type c6 { -1.2491251433584184050195851818976650, - 5.0495551072841080894517136908904045 };
    complex_type c7 { -1.2491251433584184050195851818976650, + 5.0495551072841080894517136908904045 };
    complex_type c8 {1.0534236396562679920254003621447688, - 6.0594914338643727910205790817884843 };
    complex_type c9 {1.0534236396562679920254003621447688, + 6.0594914338643727910205790817884843 };
    complex_type c10 {4.7781960766049080498636170538557330, - 6.4511763374203890543976549462152707 };
    complex_type c11 {4.7781960766049080498636170538557330, + 6.4511763374203890543976549462152707 };

    complex_matrix_type a0 = a;
    complex_matrix_type a1 = a;
    complex_matrix_type a2 = a;
    complex_matrix_type a3 = a;
    complex_matrix_type a4 = a;
    complex_matrix_type a5 = a;
    complex_matrix_type a6 = a;
    complex_matrix_type a7 = a;
    complex_matrix_type a8 = a;
    complex_matrix_type a9 = a;
    complex_matrix_type a10 = a;
    complex_matrix_type a11 = a;

    std::for_each( a0.diag_begin(), a0.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c0;
    } );
    std::for_each( a1.diag_begin(), a1.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c1;
    } );
    std::for_each( a2.diag_begin(), a2.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c2;
    } );
    std::for_each( a3.diag_begin(), a3.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c3;
    } );
    std::for_each( a4.diag_begin(), a4.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c4;
    } );
    std::for_each( a5.diag_begin(), a5.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c5;
    } );
    std::for_each( a6.diag_begin(), a6.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c6;
    } );
    std::for_each( a7.diag_begin(), a7.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c7;
    } );
    std::for_each( a8.diag_begin(), a8.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c8;
    } );
    std::for_each( a9.diag_begin(), a9.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c9;
    } );
    std::for_each( a10.diag_begin(), a10.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c10;
    } );
    std::for_each( a11.diag_begin(), a11.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c11;
    } );

    complex_type fac = {5.28885199410244786556, 0.0};
    a0 /= fac;
    a1 /= fac;
    a2 /= fac;
    a3 /= fac;
    a4 /= fac;
    a5 /= fac;
    a6 /= fac;
    a7 /= fac;
    a8 /= fac;
    a9 /= fac;
    a10 /= fac;
    a11 /= fac;

    //complex_matrix_type S = a0 * a1 * a2 * a3 * a4 * a5 * a6 * a7 * a8 * a9 * a10 * a11 / value_type{479001600};
    complex_matrix_type S = a0 * a1 * a2 * a3 * a4 * a5 * a6 * a7 * a8 * a9 * a10 * a11;

    for ( unsigned long i = 0; i != s; ++i )
    {
        S *= S;
    }

    complex_matrix_type dS = S - S_ref;

    value_type mr = 0.0;
    value_type mi = 0.0;

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            value_type mr_ = std::abs( std::real( dS[r][c] ) );
            value_type mi_ = std::abs( std::imag( dS[r][c] ) );
            if ( std::isnan( mr_ ) || std::isinf( mr ) )
            {
                std::cout << "\nr: inf/nan with " << r << ", " << c << "\n";
            }
            if ( std::isnan( mi_ ) || std::isinf( mi ) )
            {
                std::cout << "\ni: inf/nan with " << r << ", " << c << "\n";
            }
            if ( mr_ > mr )
            {
                mr = mr_;
            }
            if ( mi_ > mi )
            {
                mi = mi_;
            }
        }

    std::cout.precision( 15 );
    std::cout << "\nfor PP order 12, the max diff of real/imag is " << mr << "/" << mi << "\n";
}


void compare_expm_9( complex_matrix_type const& A, complex_matrix_type const& S_ref )
{
    unsigned long n = A.row();
    assert( A.col() == n );

    value_type ratio = f::norm_1( A );
    unsigned long s =  ratio < value_type {1} ? 0 : static_cast<unsigned long>( std::ceil( std::log2( ratio ) ) );
    value_type factor = std::pow( value_type {2}, s );
    value_type s__2           = s ? static_cast<value_type>( 1 << s ) : value_type {1};

    complex_matrix_type a = A / s__2;

    complex_type c0 { -3.3335514852690488032942739163345055, 0.0 };
    complex_type c1 { -3.0386480729366970892124687564926859,  - 1.5868011957588383288038677051222921};
    complex_type c2 { -3.0386480729366970892124687564926859,  + 1.5868011957588383288038677051222921 };
    complex_type c3 { -2.1108398003026547374987047865183922,  - 3.0899109287255009227777015426228801 };
    complex_type c4 { -2.1108398003026547374987047865183922,  + 3.0899109287255009227777015426228801 };
    complex_type c5 { -0.38106984566311299903129424501333242, - 4.3846445331453979503692027283066828 };
    complex_type c6 { -0.38106984566311299903129424501333242, + 4.3846445331453979503692027283066828 };
    complex_type c7 {2.6973334615369892273896047461916633,   - 5.1841620626494141778340870727109629 };
    complex_type c8 { 2.6973334615369892273896047461916633,   + 5.1841620626494141778340870727109629 };

    complex_matrix_type a0 = a;
    complex_matrix_type a1 = a;
    complex_matrix_type a2 = a;
    complex_matrix_type a3 = a;
    complex_matrix_type a4 = a;
    complex_matrix_type a5 = a;
    complex_matrix_type a6 = a;
    complex_matrix_type a7 = a;
    complex_matrix_type a8 = a;

    std::for_each( a0.diag_begin(), a0.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c0;
    } );
    std::for_each( a1.diag_begin(), a1.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c1;
    } );
    std::for_each( a2.diag_begin(), a2.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c2;
    } );
    std::for_each( a3.diag_begin(), a3.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c3;
    } );
    std::for_each( a4.diag_begin(), a4.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c4;
    } );
    std::for_each( a5.diag_begin(), a5.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c5;
    } );
    std::for_each( a6.diag_begin(), a6.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c6;
    } );
    std::for_each( a7.diag_begin(), a7.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c7;
    } );
    std::for_each( a8.diag_begin(), a8.diag_end(), [ = ]( complex_type & cv )
    {
        cv -= c8;
    } );

    complex_type fac {4.14716627439691288051, 0.0};

    a0 /= fac;
    a1 /= fac;
    a2 /= fac;
    a3 /= fac;
    a4 /= fac;
    a5 /= fac;
    a6 /= fac;
    a7 /= fac;
    a8 /= fac;

    //complex_matrix_type S = a0 * a1 * a2 * a3 * a4 * a5 * a6 * a7 * a8 / value_type{362880};
    complex_matrix_type S = a0 * a1 * a2 * a3 * a4 * a5 * a6 * a7 * a8;

    for ( unsigned long i = 0; i != s; ++i )
    {
        S *= S;
    }

    complex_matrix_type dS = S - S_ref;

    value_type mr = 0.0;
    value_type mi = 0.0;

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            value_type mr_ = std::abs( std::real( dS[r][c] ) );
            value_type mi_ = std::abs( std::imag( dS[r][c] ) );
            if ( std::isnan( mr_ ) || std::isinf( mr ) )
            {
                std::cout << "\nr: inf/nan with " << r << ", " << c << "\n";
            }
            if ( std::isnan( mi_ ) || std::isinf( mi ) )
            {
                std::cout << "\ni: inf/nan with " << r << ", " << c << "\n";
            }
            if ( mr_ > mr )
            {
                mr = mr_;
            }
            if ( mi_ > mi )
            {
                mi = mi_;
            }
        }

    std::cout.precision( 15 );
    std::cout << "\nfor PP order 9, the max diff of real/imag is " << mr << "/" << mi << "\n";
}

void compare_expm_p( complex_matrix_type const& A, unsigned long order, complex_matrix_type const& S_ref )
{
    unsigned long n = A.row();
    assert( A.col() == n );

    value_type ratio = f::norm_1( A );
    unsigned long s =  ratio < value_type {1} ? 0 : static_cast<unsigned long>( std::ceil( std::log2( ratio ) ) );
    value_type factor = std::pow( value_type {2}, s );
    value_type s__2           = s ? static_cast<value_type>( 1 << s ) : value_type {1};

    complex_matrix_type I { n, n, complex_type{0.0, 0.0} };
    std::fill( I.diag_begin(), I.diag_end(), complex_type {1.0, 0.0} );

    complex_matrix_type a = A / s__2;

    complex_matrix_type S = I;

    for ( unsigned long i = 1; i != order; ++i )
    {
        S += an( a, i ) / ( value_type )fn( i );
        //coef /= (value_type)i;
        //S += coef * a;
        //a *= A;
    }

    for ( unsigned long i = 0; i != s; ++i )
    {
        S *= S;
    }

    complex_matrix_type dS = S - S_ref;

    value_type mr = 0.0;
    value_type mi = 0.0;

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            value_type mr_ = std::abs( std::real( dS[r][c] ) );
            value_type mi_ = std::abs( std::imag( dS[r][c] ) );
            if ( mr_ > mr )
            {
                mr = mr_;
            }
            if ( mi_ > mi )
            {
                mi = mi_;
            }
        }

    std::cout.precision( 15 );
    std::cout << "\nfor order " << order << ", the max diff of real/imag is " << mr << "/" << mi << "\n";
}

void e_compare_expm_9( complex_matrix_type const& A, complex_matrix_type const& S_ref )
{
    unsigned long n = A.row();
    assert( A.col() == n );

    value_type ratio = f::norm_1( A );
    unsigned long s =  ratio < value_type {1} ? 0 : static_cast<unsigned long>( std::ceil( std::log2( ratio ) ) );
    value_type factor = std::pow( value_type {2}, s );
    value_type s__2           = s ? static_cast<value_type>( 1 << s ) : value_type {1};

    complex_matrix_type a1 = A / s__2;
    complex_matrix_type a2 = a1 * a1;
    complex_matrix_type a3 = a1 * a2;

    complex_matrix_type I {n, n, complex_type{0, 0}};
    std::fill( I.diag_begin(), I.diag_end(), complex_type {1, 0} );

    auto const p1 = a3 + complex_type {9.4108476311424429817192114293198773, 0.0} * a2 + complex_type {32.010299739519700993526703045242105, 0.0} * a1 + complex_type {39.173630726649007085977022185666594, 0.0} * I;
    auto const p2 = a3 + complex_type {4.6027494462684224740287038180501168, 4.3846445331453979503692027283066828} * a2 +
                    complex_type {15.611949003840816783610314223770969, 18.510564381485517241363978456937948} * a1 + complex_type {5.3361950563949313409201792145852824, 61.399028939445575913258464907489851} * I;
    auto const p3 = a3 + complex_type { - 5.0135970774108654557479152473699942, - 4.3846445331453979503692027283066828} * a2 +
                    complex_type {32.095399202760843419253361350193330, 23.653696832396624699846938534814053} * a1 + complex_type {13.013971209325298438705971137884319, -149.74062725478794246266244284870375} * I ;

    complex_matrix_type S = p1 * p2 * p3 / value_type {362880.0};

    for ( unsigned long i = 0; i != s; ++i )
    {
        S *= S;
    }

    complex_matrix_type dS = S - S_ref;

    value_type mr = 0.0;
    value_type mi = 0.0;

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            value_type mr_ = std::abs( std::real( dS[r][c] ) );
            value_type mi_ = std::abs( std::imag( dS[r][c] ) );
            if ( std::isnan( mr_ ) || std::isinf( mr ) )
            {
                std::cout << "\nr: inf/nan with " << r << ", " << c << "\n";
            }
            if ( std::isnan( mi_ ) || std::isinf( mi ) )
            {
                std::cout << "\ni: inf/nan with " << r << ", " << c << "\n";
            }
            if ( mr_ > mr )
            {
                mr = mr_;
            }
            if ( mi_ > mi )
            {
                mi = mi_;
            }
        }

    std::cout.precision( 15 );
    std::cout << "\nfor PPse order 9, the max diff of real/imag is " << mr << "/" << mi << "\n";
}

void e_compare_expm_12( complex_matrix_type const& A, complex_matrix_type const& S_ref )
{
    unsigned long n = A.row();
    assert( A.col() == n );

    value_type ratio = f::norm_1( A );
    unsigned long s =  ratio < value_type {1} ? 0 : static_cast<unsigned long>( std::ceil( std::log2( ratio ) ) );
    value_type factor = std::pow( value_type {2}, s );
    value_type s__2           = s ? static_cast<value_type>( 1 << s ) : value_type {1};

    complex_matrix_type a1 = A / s__2;
    complex_matrix_type a2 = a1 * a1;
    complex_matrix_type a3 = a1 * a2;

    complex_matrix_type I {n, n, complex_type{0, 0}};
    std::fill( I.diag_begin(), I.diag_end(), complex_type {1, 0} );

#if 0
#endif
    //\frac{ \left( {x}^{3}+ 11.960113580975467623845040806681209\,{x}^{2}+ 2.3027571492470953276351045107744921\,i{x}^{2}+ 48.216387479963393384471191061785405\,x+
    // 19.046602878903642636325578024029306\,ix+ 65.310893917618476265868251033357330+ 40.769672808922641940108972832504098\,i \right)
    auto const p1 = a3 + complex_type {11.960113580975467623845040806681209, 2.3027571492470953276351045107744921} * a2 +
                    complex_type {48.216387479963393384471191061785405, 19.046602878903642636325578024029306} * a1 +
                    complex_type {65.310893917618476265868251033357330, 40.769672808922641940108972832504098} * I;

    //    \left( {x}^{3}+ 9.2048755648300476498938236615244651\,{x}^{2}- 2.3027571492470953276351045107744921\,i{x}^{2}+ 42.036000928319459605557351021467572\,x- 12.701958839347392927389871045441117\,ix+
    // 80.005257638245259295967979727473762- 49.942482505585608164471982697687926\,i \right)
    auto const p2 = a3 + complex_type { 9.2048755648300476498938236615244651, - 2.3027571492470953276351045107744921} * a2 +
                    complex_type {42.036000928319459605557351021467572, - 12.701958839347392927389871045441117} * a1 +
                    complex_type {80.005257638245259295967979727473762, - 49.942482505585608164471982697687926} * I;

    //   \left( {x}^{3}+ 1.4448266470605688180137700016505612\,{x}^{2}+
    //6.0594914338643727910205790817884843\,i{x}^{2}+ 24.426604495463644394877697266203611\,x+ 15.138126212009885920288795371008840\,ix- 28.503874364304155245311580398456749+
    //163.95966071048634009892790465937710\,i \right)
    auto const p3 = a3 + complex_type { 1.4448266470605688180137700016505612,  6.0594914338643727910205790817884843} * a2 +
                    complex_type { 24.426604495463644394877697266203611, 15.138126212009885920288795371008840 } * a1 +
                    complex_type { - 28.503874364304155245311580398456749, 163.95966071048634009892790465937710 } * I;

    //    \left( {x}^{3}- 10.609815792866084091752634469856235\,{x}^{2}- 6.0594914338643727910205790817884843\,i{x}^{2}+
    // 74.515763286992166021758598193373921\,x+ 57.906876391023589465359778838904725\,ix- 67.891925160606028223684424127967459- 390.52715683643666259132465969063316\,i \right)}{479001600}
    auto const p4 = a3 + complex_type { - 10.609815792866084091752634469856235, - 6.0594914338643727910205790817884843} * a2 +
                    complex_type { 74.515763286992166021758598193373921, 57.906876391023589465359778838904725 } * a1 +
                    complex_type { - 67.891925160606028223684424127967459, - 390.52715683643666259132465969063316 } * I;


    complex_matrix_type S = p1 * p2 * p3 * p4 / complex_type {479001600, 0};

    for ( unsigned long i = 0; i != s; ++i )
    {
        S *= S;
    }

    complex_matrix_type dS = S - S_ref;

    value_type mr = 0.0;
    value_type mi = 0.0;

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            value_type mr_ = std::abs( std::real( dS[r][c] ) );
            value_type mi_ = std::abs( std::imag( dS[r][c] ) );
            if ( std::isnan( mr_ ) || std::isinf( mr ) )
            {
                std::cout << "\nr: inf/nan with " << r << ", " << c << "\n";
            }
            if ( std::isnan( mi_ ) || std::isinf( mi ) )
            {
                std::cout << "\ni: inf/nan with " << r << ", " << c << "\n";
            }
            if ( mr_ > mr )
            {
                mr = mr_;
            }
            if ( mi_ > mi )
            {
                mi = mi_;
            }
        }

    std::cout.precision( 15 );
    std::cout << "\nfor PPse order 12, the max diff of real/imag is " << mr << "/" << mi << "\n";
}

int main()
{

    matrix_type ai( "Ai.csv" );
    matrix_type ar( "Ar.csv" );
    matrix_type si( "Si.csv" );
    matrix_type sr( "Sr.csv" );

    std::cout << "\nthe max element in Ai is " << *std::max_element( ai.begin(), ai.end() ) << "\n";
    std::cout << "\nthe max element in Ar is " << *std::max_element( ar.begin(), ar.end() ) << "\n";
    std::cout << "\nthe max element in Si is " << *std::max_element( si.begin(), si.end() ) << "\n";
    std::cout << "\nthe max element in Sr is " << *std::max_element( sr.begin(), sr.end() ) << "\n";

    auto const& abs_less = []( value_type lhs, value_type rhs )
    {
        return std::abs( lhs ) < std::abs( rhs );
    };

    std::cout << "\nthe min element in Ai is " << *std::min_element( ai.begin(), ai.end(), abs_less ) << "\n";
    std::cout << "\nthe min element in Ar is " << *std::min_element( ar.begin(), ar.end(), abs_less ) << "\n";
    std::cout << "\nthe min element in Si is " << *std::min_element( si.begin(), si.end(), abs_less ) << "\n";
    std::cout << "\nthe min element in Sr is " << *std::min_element( sr.begin(), sr.end(), abs_less ) << "\n";

    unsigned long n = ai.row();
    assert( ai.col() == n );

    assert( ar.row() == n );
    assert( ar.col() == n );

    assert( si.row() == n );
    assert( si.col() == n );

    assert( sr.row() == n );
    assert( sr.col() == n );

    complex_matrix_type A( n, n );
    complex_matrix_type S( n, n );

    for ( unsigned long r = 0; r != n; ++r )
        for ( unsigned long c = 0; c != n; ++c )
        {
            A[r][c] = std::complex<value_type> {ar[r][c], ai[r][c]};
            S[r][c] = std::complex<value_type> {sr[r][c], si[r][c]};
        }

    //5, 9, 11, 12, 15, 18
    //
#if 1
    compare_expm_p( A, 5, S );
    compare_expm_p( A, 9, S );
    compare_expm_p( A, 11, S );
    compare_expm_p( A, 12, S );
    compare_expm_p( A, 15, S );
    compare_expm_p( A, 18, S );

#endif
    compare_expm_9( A, S );
    compare_expm_12( A, S );
    e_compare_expm_9( A, S );
    e_compare_expm_12( A, S );
    return 0;
}

