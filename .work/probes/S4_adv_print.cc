#include "../../matrix.hpp"
#include <cstdio>
int main()
{
    auto const r = feng::rref( feng::matrix<double>{ 1, 2, { 2.0, 4.0 } } );
    if ( r.has_value() )
        std::printf( "rref({2,4}) = { %g, %g }\n", ( *r )[0][0], ( *r )[0][1] );
    else
        std::printf( "rref({2,4}) = nullopt\n" );
    return 0;
}
