#include <f/least_square_fit/least_square_fit.hpp>

#include <f/variate_generator/variate_generator.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

/*
   the model to fit
   y(x, a, b, c ) = ( a0 + a1 x + a2 x^2 + a3 x^3 ) * 
                    ( b0 +      + b2 x^2 + b3 x^3 ) * 
                    ( c0 + c1 x +        + c3 x^3 )
   */


struct convert
{
    std::vector< double > a;
    std::vector< double > b;
    std::vector< double > c;
    std::vector< double > x;

    convert( const double X, const std::vector<double>& A ) 
    {
        a.resize( 4 );
        b.resize( 4 );
        c.resize( 4 );
        x.resize( 4 );

        a[0] = A[0]; a[1] = A[1]; a[2] = A[2]; a[3] = A[3];
        b[0] = A[4]; b[1] = 0.0;  b[2] = A[5]; b[3] = A[6];
        c[0] = A[7]; c[1] = A[8]; c[2] = 0.0;  c[3] = A[9];
        x[0] = 1.0;  x[1] = X;    x[2] = X*X;  x[3] = x[2] * X;
    }

    double pa() const 
    {
        return std::inner_product( a.begin(), a.end(), x.begin(), 0.0 );
    }

    double pb() const 
    {
        return std::inner_product( b.begin(), b.end(), x.begin(), 0.0 );
    }

    double pc() const 
    {
        return std::inner_product( c.begin(), c.end(), x.begin(),  0.0 );
    }

};

double fy( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pb() * c.pc();
}

double fa0( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pb() * c.pc();
}

double fa1( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return x * c.pb() * c.pc();
}

double fa2( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return (c.x)[2] * c.pb() * c.pc();
}

double fa3( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return (c.x)[3] * c.pb() * c.pc();
}

double fb0( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pc();
}

double fb1( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pc() * (c.x)[2];
}

double fb2( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pc() * (c.x)[3];
}

double fc0( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pb();
}

double fc1( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pb() * (c.x)[1];
}

double fc2( double x, const std::vector<double>& A )
{
    convert const c( x, A );

    return c.pa() * c.pb() * (c.x)[3];
}


int main()
{
    double x[1000];
    double y[1000];
    double w[1000];//weigh
    //set weigh to 1.0 -- all data set of same importance
    std::fill( w, w+1000, 1.0 );

    //Unofirm random number generator ~ U( -3, 3 )
    f::variate_generator<double, f::uniform> x_g( -10.0, 10.0);
    for ( unsigned int i = 0; i != 1000; ++i )
    {
        x[i] = x_g();    
        y[i] = std::exp( x[i] );
    }
   
    //save fitted parameter a[]
    double a_fit[10];
    //composite partial derivate vector
    std::vector<double(*)(double, const std::vector<double>&)> py;
    py.push_back( fa0 );
    py.push_back( fa1 );
    py.push_back( fa2 );
    py.push_back( fa3 );
    py.push_back( fb0 );
    py.push_back( fb1 );
    py.push_back( fb2 );
    py.push_back( fc0 );
    py.push_back( fc1 );
    py.push_back( fc2 );
    //try to fit using levenberg marquardt algorithm
    f::nonlinear_least_square_fit( x, x+1000, y, w, fy, py.begin(), py.end(), a_fit, f::levenberg_marquardt<double>() );
    //f::nonlinear_least_square_fit( x, x+1000, y, fy, py.begin(), py.end(), a_fit, f::levenberg_marquardt<double>() );

    //out put fit result
    std::cout << "\nfitted parameters: \n";
    std::cout.precision(15);
    std::copy( a_fit, a_fit+10, std::ostream_iterator<double>(std::cout, "\t"));

    return 0;
}
