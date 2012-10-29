#include <vg.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>

int main()
{
    using namespace feng;

    vg::vg<double> v;
    vg::vg<unsigned int> N(1, 1000);
    matrix<double> m;
    for ( unsigned int i = 0; i < 1; ++i )
    {
        unsigned int const n = N();        
        std::cout << n << "\t:\tthe matrix size" << std::endl;
        m.resize(n, n);
        matrix<double> m(n,n);
        std::copy( v.begin(), v.begin()+n*n, m.begin() );
        auto const I = m * (m.inverse());
        std::cout << std::accumulate( I.begin(), I.end(), double(0) - n ) << "\t:\tthe rem"<< std::endl;
    }

    return 0;
}



