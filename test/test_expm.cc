#include <matrix.hpp>
#include <iostream>
#include <complex>

int main()
{
    using namespace feng;

    const unsigned long n = 1024;

    auto const m = rand<double>(n, n);
    auto const e = expm(m);

    //cross verified with matlab
    //m.save_as( "m.dat" );
    //e.save_as( "e.dat" );

    return 0;
}

