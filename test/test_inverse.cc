#include <vg.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>

int main()
{
    using namespace feng;

    vg::variate_generator<double> v;
    vg::variate_generator<unsigned int> N(1, 1000);
    matrix<double> m;
    for ( unsigned int i = 0; i < 100; ++i )
    {
        unsigned int const n = N();        
        std::cout << n << ":\t" << std::endl;
        m.resize(n, n);
        matrix<double> m(n,n);
        std::copy( v.begin(), v.begin()+n*n, m.begin() );
        auto const I = m * (m.inverse());
        std::cout << int ( std::accumulate( I.begin(), I.end(), double(0) ) - n ) << std::endl;
    }

    return 0;
}



