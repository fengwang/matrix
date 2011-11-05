#include <least_square_fit.hpp>
#include <vg.hpp>

#include <vector>
#include <iostream>
#include <functional>
#include <cmath>
#include <algorithm>
#include <iterator>

using namespace feng;
using namespace std;
using namespace vg;

int main()
{
    const unsigned long n = 1000;
    const unsigned long m = 5;
    vector<double> x(n);
    vector<double> y(n);
    vector<function<double(double)> > f;
    vector<double> a(m);
    vector<double> b(m);

    // y = a0 + a1 * cos(x) + a2 * sin(x) + a3 * tan(x) + a4 * exp(x);
    variate_generator<double> vvgg(-10.0, 10.0);
    variate_generator<double,normal> vgg;

    copy( vvgg.begin(), vvgg.begin()+n, x.begin() );
    copy( vgg.begin(), vgg.begin()+m, a.begin() );


    f.push_back( [](double x) -> double{ return 1; } );
    f.push_back( [](double x) -> double{ return cos(x); } );
    f.push_back( [](double x) -> double{ return sin(x); } );
    f.push_back( [](double x) -> double{ return tan(x); } );
    f.push_back( [](double x) -> double{ return exp(x); } );

    copy( vgg.begin(), vgg.begin()+n, y.begin() );
    for ( auto i = 0; i < n; ++i )
        for ( auto j = 0; j < m; ++j )
            y[i] += a[j] * f[j](x[i]);

    linear_lease_square_fit<double>( x.begin(), x.end(), y.begin(), f.begin(), f.end(), b.begin() );

    for ( auto i = 0; i < m; ++i )
        cout << "\na[" << i << "] = " << a[i] << "<->" << b[i] << "<<>>" << a[i]-b[i]; 

    

    
    return 0;
}
