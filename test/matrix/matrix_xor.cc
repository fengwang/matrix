#include <f/matrix/matrix.hpp>

#include <iostream>

int main()
{
   const unsigned long n = 1000;

    auto xy = f::rand<float>( 2, n*4 );
    auto xxory = f::rand<float>( 1, n*4 );

    xy *= 0.02f;
    xy -= 0.01f;

    xxory *= 0.02f;
    xxory -= 0.01f;



    for ( unsigned long i = 0; i != n; ++i )
    {
        {
            unsigned long c = 4*i + 0;
            xy[0][c] += 0.0f;
            xy[1][c] += 0.0f;
            xxory[0][c] += 0.0f;
        }
        {
            unsigned long c = 4*i + 1;
            xy[0][c] += 1.0f;
            xy[1][c] += 0.0f;
            xxory[0][c] += 1.0f;
        }
        {
            unsigned long c = 4*i + 2;
            xy[0][c] += 0.0f;
            xy[1][c] += 1.0f;
            xxory[0][c] += 1.0f;
        }
        {
            unsigned long c = 4*i + 3;
            xy[0][c] += 1.0f;
            xy[1][c] += 1.0f;
            xxory[0][c] += 0.0f;
        }
    }

    xy.save_as( "xor_in.txt" );
    xxory.save_as( "xor_out.txt" );


    return 0;
}

