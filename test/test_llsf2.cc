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

int main()
{
    const unsigned long n = 1000;
    const unsigned long m = 2;
    vector<double> x(n);
    vector<double> y(n);
    vector<double> w(n);
    vector<function<double(double)> > f;
    vector<double> a(m);
    vector<double> b(m);

    // y = a0 + a1 * cos(x) + a2 * sin(x) + a3 * tan(x) + a4 * exp(x);
    vg::vg<double> vvgg(-10.0, 10.0);
    vg::vg<double,vg::normal> vgg;
    vg::vg<double> vg1(1.0, 10.0);

    copy( vvgg.begin(), vvgg.begin()+n, x.begin() );
    copy( vgg.begin(), vgg.begin()+m, a.begin() );
    copy( vg1.begin(), vg1.begin()+n, w.begin() );

    f.push_back( [](double x) -> double{ return 1.0; } );
    f.push_back( [](double x) -> double{ return x; } );

    //copy( vgg.begin(), vgg.begin()+n, y.begin() );
    for ( unsigned long i = 0; i < n; ++i )
        for ( unsigned long j = 0; j < m; ++j )
            y[i] += a[j] * f[j](x[i]);

    //linear_lease_square_fit<double>( x.begin(), x.end(), y.begin(), w.begin(), f.begin(), f.end(), b.begin() );
    linear_lease_square_fit( x.begin(), x.end(), y.begin(), w.begin(), f.begin(), f.end(), b.begin() );

    for ( unsigned long i = 0; i < m; ++i )
        cout << "\na[" << i << "] = " << a[i] << "<->" << b[i] << "<<>>" << a[i]-b[i]; 

    

    
    return 0;
}
