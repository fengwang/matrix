#include <c_interfaces/expm.h>

#include <stdio.h>

int main()
{
    unsigned n = 10;
    unsigned i = 0;
    unsigned j = 0;

    double a[n*n];
    double b[n*n];

    for ( i = 0; i != n; ++i )
        for ( j = 0; j != n; ++j )
        {
            a[i*n+j] = (i == j) ? 1.0 : 0.0;
            b[i*n+j] = a[i*n+j];
        }

    dexpm( a, a, n );

    for ( i = 0; i != n; ++i )
    {
        for ( j = 0; j != n; ++j )
            printf("%f\t", a[i*n+j]);
        printf("\n");
    }

    zexpm( a, b, a, b, n );

    for ( i = 0; i != n; ++i )
    {
        for ( j = 0; j != n; ++j )
            printf("%f+%fi\t", a[i*n+j], b[i*n+j]);
        printf("\n");
    }

    return 0;
}
