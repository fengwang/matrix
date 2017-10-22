#include <f/least_square_fit/least_square_fit.hpp>
#include <f/variate_generator/variate_generator.hpp>

#include <vector>
#include <iostream>
#include <functional>
#include <cmath>
#include <algorithm>
#include <iterator>

using namespace f;
using namespace std;

void test_1()
{
    const unsigned long n = 1000;
    const unsigned long m = 5;
    vector<double> x(n);
    vector<double> y(n);
    vector<function<double(double)> > f;
    vector<double> a(m);
    vector<double> b(m);
    matrix<double> X( n, m );
    matrix<double> Y( n, 1 );
    matrix<double> A( m, 1 );

    // y = a0 + a1 * cos(x) + a2 * sin(x) + a3 * tan(x) + a4 * exp(x);
    variate_generator<double> vvgg(-10.0, 10.0);
    variate_generator<double,normal> vgg;

    copy( vvgg.begin(), vvgg.begin()+n, x.begin() );
    copy( vgg.begin(), vgg.begin()+m, a.begin() );
    std::copy( a.begin(), a.end(), A.begin() );

    f.push_back( [](double x) -> double{ return 1; } );
    f.push_back( [](double x) -> double{ return cos(x); } );
    f.push_back( [](double x) -> double{ return sin(x); } );
    f.push_back( [](double x) -> double{ return tan(x); } );
    f.push_back( [](double x) -> double{ return exp(x); } );

    copy( vgg.begin(), vgg.begin()+n, y.begin() );
    for ( std::size_t i = 0; i < n; ++i )
        for ( std::size_t j = 0; j < m; ++j )
        {
            X[i][j] = f[j](x[i]);
            y[i] += a[j] * f[j](x[i]);
        }
    std::copy( y.begin(), y.end(), Y.begin() );

    //linear_lease_square_fit<double>( x.begin(), x.end(), y.begin(), f.begin(), f.end(), b.begin() );
    linear_least_square_fit( x.begin(), x.end(), y.begin(), f.begin(), f.end(), b.begin() );

    X.save_as( "X.dat" );
    Y.save_as( "Y.dat" );
    A.save_as( "A.dat" );

    for ( std::size_t i = 0; i < m; ++i )
        cout << "\na[" << i << "] = " << a[i] << "<->" << b[i] << "<<>>" << a[i]-b[i];

}

void test_2()
{
    unsigned long const limit = 10000*50000;
    matrix<double> ans( 10, 1 );
    std::fill( ans.begin(), ans.end(), 0.0 );

    for ( unsigned long i = 0; i != limit; ++i )
    {
        matrix<double> x( 10, 10 );
        matrix<double> y( 10, 1 );

        variate_generator<double> vg( -2.0, 2.0 );

        for ( unsigned long r = 0; r != 10; ++r )
        {
            double x_ = vg();

            y[r][0] = x_ / std::sin(x_) - 1.0;
            x[r][0] = x_;

            for ( unsigned long  c = 1; c != 10; ++c )
            {
                x[r][c] = x[r][c-1] * x_;
            }
        }

        ans += x.inverse() * y;
    }

    ans /= limit;

    std::cout << ans;

}


int main()
{
    //test_1();
    test_2();

    return 0;
}
