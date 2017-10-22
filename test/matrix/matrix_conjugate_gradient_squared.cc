#include <iostream>
#include <f/matrix/numeric/conjugate_gradient_squared.hpp>
#include <f/matrix/matrix.hpp>
#include <f/variate_generator/variate_generator.hpp>

#include <f/test/test.hpp>


TEST( cgs1 )
{
    using namespace f;
    variate_generator<unsigned long> vln( 10, 200 );
    for ( unsigned long index = 0; index != 1000; ++index )
    {
        unsigned long n =vln();
        variate_generator<double> vg( -1.0, 1.0 );
        matrix<double> A(n, n);
        matrix<double> x(n, 1);
        std::generate( A.begin(), A.end(), vg );
        std::generate( x.begin(), x.end(), vg );
        matrix<double> const& b = A*x;

        std::generate( x.begin(), x.end(), vg );
        auto const test_result = conjugate_gradient_squared(A, x, b, 100, 1.0e-8);
        CHECK_EQUAL(0, test_result);
    }
}

int main()
{
    RUN_TESTS;

    return 0;
}




