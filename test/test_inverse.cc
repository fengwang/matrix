#include <iostream>
#include <algorithm>
#include <numeric>
#include <matrix.hpp>

int main()
{
    const unsigned long int n = 760;

    auto const m = feng::rand<double>(n, n);
    auto const I = feng::inv(m) * m;

    std::cout << "\ninner_product of I*I^{-1} [should be " << n << "] is " << std::inner_product( I.begin(), I.end(), I.begin(), double(0) ) << ".\n";
    std::cout << "\ninner_product of I*I^{-1} diagonal [should be " << n << "] is " << std::inner_product( I.diag_begin(), I.diag_end(), I.diag_begin(), double(0) ) << ".\n";

    return 0;
}

