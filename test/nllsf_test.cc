#include <least_square_fit.hpp>

#include <vg.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

// the model to fit 
//  y(x, a[]) = a_0 e^{a_1 \sin x} + a_2 \cos (a_3 x ) - a_4 ^ x + a_5
// the derivate functions:
// p_0(x,a[]) = e^{a_1 \sin x}
// p_1(x_a[]) = a_0 \sin x e^{a_1 \sin x}
// p_2(x_a[]) = \cos (a_3 x )
// p_3(x_a[]) = - a_2 a_3 \sin( a_3 x )
// p_4(x_a[]) = - a_4 ^ x ln(a_4)
// p_5(x_a[]) = 1

double fy( double x, const std::vector<double>& a )
{
    //std::cout << "\npy called with a = \n";
    //std::copy( a.begin(), a.end(), std::ostream_iterator<double>(std::cout, "\t"));
    return  a[0] * exp( a[1] * sin(x) ) +
            a[2] * cos( a[3] * x ) - 
            a[4] * x +
            //pow( a[4], x ) + 
            a[5];
}

double py0( double x, const std::vector<double>& a )
{
    return exp( a[1] * sin(x) );
}

double py1( double x, const std::vector<double>& a )
{
    return a[0] * exp( a[1] * sin(x) ) * sin(x);
}

double py2( double x, const std::vector<double>& a )
{
    return  cos( a[3] * x );
}

double py3( double x, const std::vector<double>& a )
{
    return - a[2] * a[3] * sin(a[3] * x);
}

double py4( double x, const std::vector<double>& a )
{
    //std::cout << "\npy4 called with a = \n";
    //std::copy( a.begin(), a.end(), std::ostream_iterator<double>(std::cout, "\t"));
    //return - pow( a[4], x ) * log(a[4]);
    return -x;
}

double py5( double x, const std::vector<double>& a )
{
    return 1.0;
}

int main()
{
    //randomly generate a[] ~ U(1.0, 2.0)
    vg::variate_generator<double, vg::uniform> a_g( 1.0, 2.0 );
    std::vector<double> a(6);
    std::copy( a_g.begin(), a_g.begin()+6, a.begin() );
    a[2] *= 2;
    //a[4] /= 2;
    std::cout << "\noriginal parameters: \n";
    std::copy( a.begin(), a.end(), std::ostream_iterator<double>(std::cout, "\t"));
    std::cout << std::endl;
    
    double x[1000];
    double y[1000];
    double Y[1000];//real data
    double w[1000];//weigh
    //set weigh to 1.0 -- all data set of same importance
    std::fill( w, w+1000, 1.0 );

    //Gaussian random number generator ~ N( 0.0, 2.0 )
    vg::variate_generator<double, vg::gaussian> n_g( 0.0, 2.0);
    //Unofirm random number generator ~ U( -10, 10 )
    vg::variate_generator<double, vg::uniform> x_g( -10.0, 10.0);
    for( std::size_t i = 0; i < 1000; ++i )
    {
        //generate white noise for a[] 
        std::vector<double> a_(6);
        std::transform( a.begin(), a.end(), n_g.begin(), a_.begin(), []( double a1, double a2 ){ return a1 + a1*a2; } );
        // genrate x ~ U(-10, 10)
        x[i] = x_g();
        // generate y ~ fy( x, a_ )
        y[i] = fy( x[i], a_ );
        Y[i] = fy( x[i], a );
    }
   
    //save fitted parameter a[]
    double a_fit[6];
    //composite partial derivate vector
    std::vector<double(*)(double, const std::vector<double>&)> py;
    py.push_back( py0 );
    py.push_back( py1 );
    py.push_back( py2 );
    py.push_back( py3 );
    py.push_back( py4 );
    py.push_back( py5 );

    //try to fit using levenberg marquardt algorithm
    feng::nonlinear_least_square_fit( x, x+1000, y, w, fy, py.begin(), py.end(), a_fit, feng::levenberg_marquardt<double>() );
    //feng::nonlinear_least_square_fit( x, x+1000, y, fy, py.begin(), py.end(), a_fit, feng::levenberg_marquardt<double>() );

    //out put fit result
    std::cout << "\nfitted parameters: \n";
    std::copy( a_fit, a_fit+6, std::ostream_iterator<double>(std::cout, "\t"));


    std::cout << std::endl;
    std::copy( x, x+1000, std::ostream_iterator<double>(std::cout, "\t"));
    std::cout << std::endl;
    std::copy( Y, Y+1000, std::ostream_iterator<double>(std::cout, "\t"));
    std::cout << std::endl;
    std::copy( y, y+1000, std::ostream_iterator<double>(std::cout, "\t"));
    std::cout << std::endl;
    std::vector<double> A(a_fit, a_fit+6);
    for ( std::size_t i = 0; i < 1000; ++i )
    {
        std::cout << fy(x[i], A) << "\t";
    }
    std::cout << std::endl;


    return 0;
}

